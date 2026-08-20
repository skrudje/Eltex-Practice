#!/usr/bin/env bash
set -u

PASSED=0
FAILED=0
BROKER_PID=""
SUBSCRIBER_PID=""

pass() {
    echo "[OK] $1"
    PASSED=$((PASSED + 1))
}

fail() {
    echo "[FAIL] $1"
    FAILED=$((FAILED + 1))
}

cleanup() {
    if [[ -n "$SUBSCRIBER_PID" ]] && kill -0 "$SUBSCRIBER_PID" 2>/dev/null; then
        kill -INT "$SUBSCRIBER_PID" 2>/dev/null || true
        wait "$SUBSCRIBER_PID" 2>/dev/null || true
    fi

    if [[ -n "$BROKER_PID" ]] && kill -0 "$BROKER_PID" 2>/dev/null; then
        kill -INT "$BROKER_PID" 2>/dev/null || true
        wait "$BROKER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

rm -f broker.log subscriber.log publisher.log duplicate.log error.log

if ./pubsub >error.log 2>&1; then
    fail "Запуск без режима должен завершаться ошибкой"
else
    pass "Проверка запуска без режима"
fi

if printf 'test\n' | ./pubsub -p news >error.log 2>&1; then
    fail "Издатель не должен работать без брокера"
else
    pass "Издатель обнаруживает отсутствие брокера"
fi

./pubsub -b >broker.log 2>&1 &
BROKER_PID=$!

for _ in {1..30}; do
    if grep -q "Брокер запущен" broker.log 2>/dev/null; then
        break
    fi
    sleep 0.1
done

if grep -q "Брокер запущен" broker.log; then
    pass "Запуск брокера"
else
    fail "Брокер не запустился"
    exit 1
fi

if ./pubsub -b >duplicate.log 2>&1; then
    fail "Второй брокер не должен запускаться"
else
    if grep -q "уже запущен" duplicate.log; then
        pass "Защита от запуска второго брокера"
    else
        fail "Нет сообщения о втором брокере"
    fi
fi

./pubsub -s news tech >subscriber.log 2>&1 &
SUBSCRIBER_PID=$!
sleep 0.4

printf 'Сообщение для подписчика\n' | ./pubsub -p news >publisher.log 2>&1
sleep 0.5

if grep -q "\[news\] Сообщение для подписчика" subscriber.log; then
    pass "Доставка сообщения по подходящей теме"
else
    fail "Подписчик не получил сообщение"
fi

printf 'Не должно прийти\n' | ./pubsub -p sport >/dev/null 2>&1
sleep 0.3

if grep -q "Не должно прийти" subscriber.log; then
    fail "Получено сообщение по неподписанной теме"
else
    pass "Фильтрация сообщений по теме"
fi

kill -INT "$BROKER_PID"
wait "$BROKER_PID"
BROKER_PID=""

for _ in {1..30}; do
    if ! kill -0 "$SUBSCRIBER_PID" 2>/dev/null; then
        break
    fi
    sleep 0.1
done

if kill -0 "$SUBSCRIBER_PID" 2>/dev/null; then
    fail "Подписчик не завершился после SIGINT от брокера"
else
    wait "$SUBSCRIBER_PID" 2>/dev/null || true
    SUBSCRIBER_PID=""
    pass "Брокер завершает подписчика сигналом SIGINT"
fi

if printf 'test\n' | ./pubsub -p news >error.log 2>&1; then
    fail "Очередь должна быть удалена после завершения брокера"
else
    pass "Удаление очереди сообщений"
fi

echo
echo "Пройдено: $PASSED"
echo "Ошибок: $FAILED"

if [[ "$FAILED" -ne 0 ]]; then
    exit 1
fi
