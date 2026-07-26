#include <stdio.h>
#include "calculator.h"

#define OPERATIONS_DIRECTORY "./operations"

static int passed = 0;
static int failed = 0;

static int almost_equal(double a, double b) {
    double difference = a - b;

    if (difference < 0) {
        difference = -difference;
    }

    return difference < 0.000001;
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

static void check_char(const char *name, char actual, char expected) {
    if (actual == expected) {
        printf("[OK]   %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s: получено %c, ожидалось %c\n",
               name, actual, expected);
        failed++;
    }
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

int main(void) {
    Operation *operations = NULL;
    int count = 0;

    check_int("Загрузка каталога библиотек",
              operations_load(OPERATIONS_DIRECTORY, &operations, &count),
              1);
    check_int("Количество загруженных функций", count, 4);

    if (count == 4) {
        check_char("Первая операция — сложение", operations[0].symbol, '+');
        check_char("Вторая операция — вычитание", operations[1].symbol, '-');
        check_char("Третья операция — умножение", operations[2].symbol, '*');
        check_char("Четвёртая операция — деление", operations[3].symbol, '/');

        check_number("Сложение из libadd.so",
                     operations[0].function(4, 6), 10);
        check_number("Вычитание из libsubtract.so",
                     operations[1].function(9, 2), 7);
        check_number("Умножение из libmultiply.so",
                     operations[2].function(3, 5), 15);
        check_number("Деление из libdivide.so",
                     operations[3].function(8, 4), 2);
    }

    operations_unload(&operations, &count);
    check_int("Количество после выгрузки", count, 0);
    check_int("Указатель после выгрузки равен NULL", operations == NULL, 1);

    check_int("Несуществующий каталог не загружается",
              operations_load("./no_such_directory", &operations, &count),
              0);

    printf("\nПройдено: %d\n", passed);
    printf("Ошибок: %d\n", failed);

    return failed == 0 ? 0 : 1;
}
