#!/usr/bin/env bash

set -u

PROGRAM="$(pwd)/file_copy"
TEST_DIR="$(mktemp -d)"
PASSED=0
FAILED=0

finish() {
    rm -rf "$TEST_DIR"
}
trap finish EXIT

check() {
    local name="$1"
    shift

    if "$@"; then
        echo "[OK] $name"
        PASSED=$((PASSED + 1))
    else
        echo "[FAIL] $name"
        FAILED=$((FAILED + 1))
    fi
}

cd "$TEST_DIR" || exit 1
printf 'Первая тестовая строка\n' > one.txt
printf 'Вторая строка\nЕще одна строка\n' > two.txt

"$PROGRAM" one.txt >/dev/null
check "Копирование одного файла" cmp -s one.txt one.txt.copy

"$PROGRAM" one.txt two.txt >/dev/null
check "Копирование нескольких файлов" cmp -s two.txt two.txt.copy

"$PROGRAM" missing.txt 2>error.log >/dev/null
check "Ошибка отсутствующего файла выводится в stderr" grep -q "missing.txt" error.log

"$PROGRAM" >/dev/null 2>&1
check "Запуск без файлов завершается ошибкой" test "$?" -ne 0

"$PROGRAM" -p /tmp/module3_pipe one.txt >/dev/null 2>named.log
check "Режим -p пока явно помечен как TODO" grep -q "TODO" named.log

echo
echo "Пройдено: $PASSED"
echo "Ошибок: $FAILED"

if [ "$FAILED" -ne 0 ]; then
    exit 1
fi
