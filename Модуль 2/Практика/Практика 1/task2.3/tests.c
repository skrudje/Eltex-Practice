#include <stdio.h>
#include <stdlib.h>
#include "calculator.h"

static int passed = 0;
static int failed = 0;

static int almost_equal(double a, double b) {
    double difference = a - b;

    if (difference < 0) {
        difference = -difference;
    }

    return difference < 0.000001;
}

static void check_number(const char *name, double actual, double expected) {
    if (almost_equal(actual, expected)) {
        printf("[OK]   %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s: получено %.6f, ожидалось %.6f\n",
               name, actual, expected);
        failed++;
    }
}

static void check_int(const char *name, int actual, int expected) {
    if (actual == expected) {
        printf("[OK]   %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s: получено %d, ожидалось %d\n",
               name, actual, expected);
        failed++;
    }
}

int main(void) {
    Operation *operations = NULL;
    int count = 0;

    check_int("Добавление операции сложения", operation_add(&operations, &count, "Сложение", '+', calc_summ), 1);
    check_int("Количество операций после первого добавления", count, 1);
    check_int("Добавление операции умножения", operation_add(&operations, &count, "Умножение", '*', calc_multiply), 1);
    check_int("Количество операций после второго добавления", count, 2);
    check_number("Вызов сложения через указатель на функцию", operations[0].function(4, 6), 10);
    check_number("Вызов умножения через указатель на функцию", operations[1].function(3, 5), 15);
    check_number("Обычное вычитание", calc_subtract(9, 2), 7);
    check_number("Обычное деление", calc_divide(8, 4), 2);

    free(operations);

    printf("\nПройдено: %d\n", passed);
    printf("Ошибок: %d\n", failed);

    return failed == 0 ? 0 : 1;
}
