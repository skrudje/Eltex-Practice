#!/bin/sh
set -u

PASSED=0
FAILED=0
DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
cd "$DIR" || exit 1

pass() {
    echo "[OK] $1"
    PASSED=$((PASSED + 1))
}

fail() {
    echo "[FAIL] $1"
    FAILED=$((FAILED + 1))
}

./shared_list --cleanup >/dev/null 2>&1 || true

TMP_DIR="$(mktemp -d)"
PRODUCER_LOG="$TMP_DIR/producer.log"
CONSUMER1_LOG="$TMP_DIR/consumer1.log"
CONSUMER2_LOG="$TMP_DIR/consumer2.log"
CONSUMER3_LOG="$TMP_DIR/consumer3.log"
NO_PRODUCER_LOG="$TMP_DIR/no_producer.log"

cleanup() {
    ./shared_list --cleanup >/dev/null 2>&1 || true
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT INT TERM

if ./shared_list -c >"$NO_PRODUCER_LOG" 2>&1; then
    fail "Потребитель не должен запускаться без производителя"
else
    grep -q "производитель не запущен" "$NO_PRODUCER_LOG" \
        && pass "Проверена ошибка запуска потребителя без производителя" \
        || fail "Нет ожидаемого сообщения об отсутствии производителя"
fi

PRODUCER_DELAY_MS=5 ./shared_list -p >"$PRODUCER_LOG" 2>&1 &
PRODUCER_PID=$!
sleep 0.1

CONSUMER_DELAY_MS=5 ./shared_list -c >"$CONSUMER1_LOG" 2>&1 & C1=$!
CONSUMER_DELAY_MS=5 ./shared_list -c >"$CONSUMER2_LOG" 2>&1 & C2=$!
CONSUMER_DELAY_MS=5 ./shared_list -c >"$CONSUMER3_LOG" 2>&1 & C3=$!

wait "$C1"; C1_STATUS=$?
wait "$C2"; C2_STATUS=$?
wait "$C3"; C3_STATUS=$?
wait "$PRODUCER_PID"; PRODUCER_STATUS=$?

[ "$PRODUCER_STATUS" -eq 0 ] \
    && pass "Производитель завершился без ошибки" \
    || fail "Производитель завершился с ошибкой"

if [ "$C1_STATUS" -eq 0 ] && [ "$C2_STATUS" -eq 0 ] && [ "$C3_STATUS" -eq 0 ]; then
    pass "Три потребителя завершились без ошибки"
else
    fail "Ошибка одного из потребителей"
fi

grep -q "Все наборы обработаны" "$PRODUCER_LOG" \
    && pass "Все наборы обработаны" \
    || fail "Нет подтверждения обработки всех наборов"

GENERATED="$(sed -n 's/.*Создано наборов: \([0-9][0-9]*\).*/\1/p' "$PRODUCER_LOG" | tail -1)"
PROCESSED="$(cat "$CONSUMER1_LOG" "$CONSUMER2_LOG" "$CONSUMER3_LOG" | grep -c "обработан блок" || true)"

if [ -n "$GENERATED" ] && [ "$GENERATED" -eq "$PROCESSED" ]; then
    pass "Каждый созданный блок обработан ровно один раз"
else
    fail "Создано: ${GENERATED:-?}, обработано: $PROCESSED"
fi

DUPLICATES="$(cat "$CONSUMER1_LOG" "$CONSUMER2_LOG" "$CONSUMER3_LOG" \
    | sed -n 's/.*offset=\([0-9][0-9]*\).*/\1/p' \
    | sort | uniq -d | wc -l)"

[ "$DUPLICATES" -eq 0 ] \
    && pass "Нет повторной обработки одного блока" \
    || fail "Один блок обработан несколькими потребителями"

grep -q "POSIX разделяемая память и семафор удалены" "$PRODUCER_LOG" \
    && pass "Производитель удалил POSIX IPC-объекты" \
    || fail "Нет подтверждения удаления POSIX IPC"

if [ ! -e /dev/shm/eltex_module3_task5_shm ] && \
   [ ! -e /dev/shm/sem.eltex_module3_task5_sem ]; then
    pass "POSIX IPC-объекты отсутствуют в /dev/shm после завершения"
else
    fail "После завершения в /dev/shm остались IPC-объекты"
fi

echo
echo "Пройдено: $PASSED"
echo "Ошибок: $FAILED"

[ "$FAILED" -eq 0 ]
