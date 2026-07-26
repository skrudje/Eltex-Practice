# Задание 2.2

Обычный калькулятор с четырьмя функциями: сложение, вычитание, умножение и деление.
После выполнения операции программа снова показывает меню.

## Сборка и запуск

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic main.c calculator.c -o calculator
./calculator
```

Или через Makefile:

```bash
make
make run
```

## Автотесты

```bash
make test
```
