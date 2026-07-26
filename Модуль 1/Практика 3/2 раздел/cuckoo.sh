#!/bin/bash

RUN_DIR="/tmp/run"
FIFO="$RUN_DIR/cuckoo.pid"
LOG="$(dirname "$0")/cuckoo.log"

mkdir -p "$RUN_DIR"

if [[ ! -p "$FIFO" ]]; then
    rm -f "$FIFO"
    mkfifo "$FIFO"
fi

shutdown() {
    echo "$(date '+%F %T') Shutdown!" >> "$LOG"
    rm -f "$FIFO"
    exit 0
}

trap shutdown SIGTERM

echo "$(date '+%F %T') Startup!" >> "$LOG"

while true; do
    if IFS= read -r request < "$FIFO"; then
        if [[ $request =~ ^(.+)\[([0-9]+)\]:[[:space:]]how[[:space:]]much[[:space:]]time[[:space:]]do[[:space:]]I[[:space:]]have\?$ ]]; then
            NAME="${BASH_REMATCH[1]}"
            PID="${BASH_REMATCH[2]}"
            N=$((RANDOM % 9 + 2))
            REPLY_FIFO="$RUN_DIR/reply_$PID"

            echo "$(date '+%F %T') $NAME[$PID] $N" >> "$LOG"

            if [[ -p "$REPLY_FIFO" ]]; then
                echo "$N" > "$REPLY_FIFO"
            fi
        fi
    fi
done
