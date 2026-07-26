#!/bin/bash

BASE_DIR="$(cd "$(dirname "$0")" && pwd)"
CONF="$BASE_DIR/observer.conf"
LOG="$BASE_DIR/observer.log"

cd "$BASE_DIR" || exit 1

is_running() {
    TARGET="$1"

    for CMDLINE in /proc/[0-9]*/cmdline; do
        [[ -r "$CMDLINE" ]] || continue

        CMD=$(tr '\0' ' ' < "$CMDLINE" 2>/dev/null)

        if [[ "$CMD" == *"$TARGET"* ]]; then
            return 0
        fi
    done

    return 1
}

while IFS= read -r TASK || [[ -n "$TASK" ]]; do
    [[ -z "$TASK" ]] && continue
    [[ "$TASK" == \#* ]] && continue

    if [[ ! -x "$TASK" ]]; then
        echo "$(date '+%F %T') Файл отсутствует: $TASK" >> "$LOG"
        continue
    fi

    if ! is_running "$TASK"; then
        nohup "$TASK" >/dev/null 2>&1 &
        echo "$(date '+%F %T') Перезапущен $TASK, PID=$!" >> "$LOG"
    fi
done < "$CONF"
