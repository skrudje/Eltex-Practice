#!/bin/bash

NAME=$(basename "$0")

if [[ "$NAME" == "template_task.sh" ]]; then
    echo "я бригадир, сам не работаю"
    exit 0
fi

LOG="$(pwd)/report_${NAME}.log"
CUCKOO_FIFO="/tmp/run/cuckoo.pid"
REPLY_FIFO="/tmp/run/reply_$$"

cleanup() {
    rm -f "$REPLY_FIFO"
}

trap cleanup EXIT

if [[ ! -p "$CUCKOO_FIFO" ]]; then
    echo "Канал cuckoo.pid не найден"
    exit 1
fi

rm -f "$REPLY_FIFO"
mkfifo "$REPLY_FIFO"

echo "$(date '+%F %T') [$$] Скрипт запущен" >> "$LOG"

echo "$NAME[$$]: how much time do I have?" > "$CUCKOO_FIFO"

read -r N < "$REPLY_FIFO"

sleep "$N"

echo "$(date '+%F %T') [$$] Скрипт завершился, работал $N секунд." >> "$LOG"
