#include <stdio.h>
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

static void check(const char *name, double actual, double expected) {
    if (almost_equal(actual, expected)) {
        printf("[OK]   %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s: получено %.6f, ожидалось %.6f\n",
               name, actual, expected);
        failed++;
    }
}

int main(void) {
    check("Сложение положительных чисел", calc_summ(2, 3), 5);
    check("Сложение с отрицательным числом", calc_summ(-4, 1), -3);
    check("Вычитание", calc_subtract(10, 7), 3);
    check("Умножение", calc_multiply(2.5, 4), 10);
    check("Деление", calc_divide(9, 3), 3);
    check("Деление с дробным результатом", calc_divide(5, 2), 2.5);

    printf("\nПройдено: %d\n", passed);
    printf("Ошибок: %d\n", failed);

    return failed == 0 ? 0 : 1;
}
