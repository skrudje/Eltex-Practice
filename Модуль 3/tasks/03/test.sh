#!/usr/bin/env bash
set -u

passed=0
failed=0
workdir="$(mktemp -d)"
queue_name="/m3_chat_test_$$"

cleanup() {
    pkill -P $$ p2p_chat 2>/dev/null || true
    rm -rf "$workdir"
    rm -f "/dev/mqueue/${queue_name#/}_1" "/dev/mqueue/${queue_name#/}_2" 2>/dev/null || true
}
trap cleanup EXIT

ok() {
    echo "[OK] $1"
    passed=$((passed + 1))
}

fail() {
    echo "[FAIL] $1"
    failed=$((failed + 1))
}

if ./p2p_chat >"$workdir/no_args.out" 2>"$workdir/no_args.err"; then
    fail "Запуск без имени очереди"
else
    grep -q "Использование" "$workdir/no_args.err" && \
        ok "Запуск без имени очереди" || fail "Запуск без имени очереди"
fi

if ./p2p_chat /tmp/invalid >"$workdir/invalid.out" 2>"$workdir/invalid.err"; then
    fail "Отклонение имени с дополнительным слешем"
else
    ok "Отклонение имени с дополнительным слешем"
fi

(
    printf "Сообщение от первого\n"
    sleep 2
    printf "/exit\n"
) | ./p2p_chat "$queue_name" >"$workdir/first.out" 2>"$workdir/first.err" &
first_pid=$!

sleep 0.4

(
    sleep 0.5
    printf "Ответ от второго\n"
    sleep 0.7
    printf "/exit\n"
) | ./p2p_chat "$queue_name" >"$workdir/second.out" 2>"$workdir/second.err" &
second_pid=$!

wait "$first_pid"
first_status=$?
wait "$second_pid"
second_status=$?

if [[ $first_status -eq 0 && $second_status -eq 0 ]]; then
    ok "Оба участника завершились без ошибки"
else
    fail "Оба участника завершились без ошибки"
fi

grep -q "Ответ от второго" "$workdir/first.out" && \
    ok "Первый участник получил сообщение" || fail "Первый участник получил сообщение"

grep -q "Сообщение от первого" "$workdir/second.out" && \
    ok "Второй участник получил сообщение" || fail "Второй участник получил сообщение"

grep -q "Роль: создатель очередей" "$workdir/first.out" && \
    ok "Первый процесс создал очереди" || fail "Первый процесс создал очереди"

grep -q "Роль: второй участник" "$workdir/second.out" && \
    ok "Второй процесс подключился к очередям" || fail "Второй процесс подключился к очередям"

if [[ ! -e "/dev/mqueue/${queue_name#/}_1" && ! -e "/dev/mqueue/${queue_name#/}_2" ]]; then
    ok "Создатель удалил очереди"
else
    fail "Создатель удалил очереди"
fi

echo
echo "Пройдено: $passed"
echo "Ошибок: $failed"

[[ $failed -eq 0 ]]
