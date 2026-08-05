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

# 1. Неименованный канал.
"$PROGRAM" one.txt >/dev/null
check "Неименованный канал: один файл" cmp -s one.txt one.txt.copy
rm -f one.txt.copy

# 2. Именованные каналы.
CHANNEL="$TEST_DIR/copy_channel"
"$PROGRAM" -p "$CHANNEL" one.txt two.txt >/dev/null
check "Именованные каналы: первый файл" cmp -s one.txt one.txt.copy
check "Именованные каналы: второй файл" cmp -s two.txt two.txt.copy
check "FIFO .data удален после работы" test ! -e "$CHANNEL.data"
check "FIFO .ready удален после работы" test ! -e "$CHANNEL.ready"

# 3. Бинарные данные размером больше одного блока.
dd if=/dev/urandom of=binary.bin bs=1024 count=12 status=none
"$PROGRAM" -p "$TEST_DIR/binary_channel" binary.bin >/dev/null
check "Бинарный файл передан блоками" cmp -s binary.bin binary.bin.copy

# 4. Несуществующий файл должен дать диагностику в stderr.
"$PROGRAM" -p "$TEST_DIR/error_channel" missing.txt 2>error.log >/dev/null
check "Ошибка отсутствующего файла выведена в stderr" grep -q "missing.txt" error.log

# 5. Некорректный запуск.
"$PROGRAM" >/dev/null 2>&1
check "Запуск без файлов завершается ошибкой" test "$?" -ne 0

"$PROGRAM" -p >/dev/null 2>&1
check "Ключ -p без имени завершается ошибкой" test "$?" -ne 0

echo
echo "Пройдено: $PASSED"
echo "Ошибок: $FAILED"

if [ "$FAILED" -ne 0 ]; then
    exit 1
fi
