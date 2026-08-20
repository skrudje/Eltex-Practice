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

TMP_DIR="$(mktemp -d)"
ALICE_FIFO="$TMP_DIR/alice.in"
BOB_FIFO="$TMP_DIR/bob.in"
CAROL_FIFO="$TMP_DIR/carol.in"
ALICE_LOG="$TMP_DIR/alice.log"
BOB_LOG="$TMP_DIR/bob.log"
CAROL_LOG="$TMP_DIR/carol.log"
PORT=$((55000 + ($$ % 5000)))

mkfifo "$ALICE_FIFO" "$BOB_FIFO" "$CAROL_FIFO"

cleanup() {
    [ -n "${ALICE_PID:-}" ] && kill "$ALICE_PID" 2>/dev/null || true
    [ -n "${BOB_PID:-}" ] && kill "$BOB_PID" 2>/dev/null || true
    [ -n "${CAROL_PID:-}" ] && kill "$CAROL_PID" 2>/dev/null || true
    exec 3>&- 4>&- 5>&- 2>/dev/null || true
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT INT TERM

# Открываем FIFO на чтение и запись, чтобы запуск клиента не блокировался.
exec 3<>"$ALICE_FIFO"
exec 4<>"$BOB_FIFO"
exec 5<>"$CAROL_FIFO"

./udp_chat -n Alice -p "$PORT" <"$ALICE_FIFO" >"$ALICE_LOG" 2>&1 & ALICE_PID=$!
sleep 0.4
./udp_chat -n Bob -p "$PORT" <"$BOB_FIFO" >"$BOB_LOG" 2>&1 & BOB_PID=$!
sleep 0.4
./udp_chat -n Carol -p "$PORT" <"$CAROL_FIFO" >"$CAROL_LOG" 2>&1 & CAROL_PID=$!
sleep 0.6

printf '%s\n' 'message-from-alice' >&3
sleep 0.5
printf '%s\n' 'message-from-bob' >&4
sleep 0.5

printf '%s\n' '/quit' >&4
sleep 0.5
printf '%s\n' '/quit' >&5
sleep 0.3
printf '%s\n' '/quit' >&3

wait "$BOB_PID"; BOB_STATUS=$?
wait "$CAROL_PID"; CAROL_STATUS=$?
wait "$ALICE_PID"; ALICE_STATUS=$?

if [ "$ALICE_STATUS" -eq 0 ] && [ "$BOB_STATUS" -eq 0 ] && [ "$CAROL_STATUS" -eq 0 ]; then
    pass "Три равноправных клиента завершились без ошибки"
else
    fail "Один из клиентов завершился с ошибкой"
fi

if grep -q "Bob подключился" "$ALICE_LOG" && grep -q "Carol подключился" "$ALICE_LOG"; then
    pass "Участники получают уведомления о подключении новых клиентов"
else
    fail "Не получены все уведомления о подключении"
fi

if grep -q "\[Alice\] message-from-alice" "$BOB_LOG" && \
   grep -q "\[Alice\] message-from-alice" "$CAROL_LOG"; then
    pass "Сообщение Alice получили остальные участники"
else
    fail "Broadcast-сообщение Alice получено не всеми"
fi

if grep -q "\[Bob\] message-from-bob" "$ALICE_LOG" && \
   grep -q "\[Bob\] message-from-bob" "$CAROL_LOG"; then
    pass "Сообщение Bob получили остальные участники"
else
    fail "Broadcast-сообщение Bob получено не всеми"
fi

if ! grep -q "\[Alice\] message-from-alice" "$ALICE_LOG" && \
   ! grep -q "\[Bob\] message-from-bob" "$BOB_LOG"; then
    pass "Клиент не выводит собственное broadcast-сообщение повторно"
else
    fail "Клиент получил и вывел собственное сообщение"
fi

if grep -q "Bob отключился" "$ALICE_LOG" && \
   grep -q "Bob отключился" "$CAROL_LOG"; then
    pass "Остальные участники получают уведомление об отключении"
else
    fail "Не получено уведомление об отключении Bob"
fi

echo
echo "Пройдено: $PASSED"
echo "Ошибок: $FAILED"

[ "$FAILED" -eq 0 ]
