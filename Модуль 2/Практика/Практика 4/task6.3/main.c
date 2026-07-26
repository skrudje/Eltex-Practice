#include <stdio.h>
#include "calculator.h"

#define OPERATIONS_DIRECTORY "./operations"

static void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

static int read_int(const char *message) {
    char line[50];
    int value;

    while (1) {
        printf("%s", message);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) != NULL &&
            sscanf(line, "%d", &value) == 1) {
            return value;
        }

        printf("Ошибка: введите целое число.\n");
    }
}

static double read_double(const char *message) {
    char line[50];
    double value;

    while (1) {
        printf("%s", message);
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) != NULL &&
            sscanf(line, "%lf", &value) == 1) {
            return value;
        }

        printf("Ошибка: введите число.\n");
    }
}

static void wait_enter(void) {
    char line[10];

    printf("\nНажмите Enter, чтобы вернуться в меню...");
    fflush(stdout);
    fgets(line, sizeof(line), stdin);
}

int main(void) {
    Operation *operations;
    int count;
    int choice;
    int i;
    double a;
    double b;
    double result;

    if (!operations_load(OPERATIONS_DIRECTORY, &operations, &count)) {
        printf("В каталоге %s не найдены подходящие библиотеки.\n",
               OPERATIONS_DIRECTORY);
        return 1;
    }

    do {
        clear_screen();
        printf("--- Калькулятор ---\n");
        printf("Загружено операций: %d\n\n", count);

        for (i = 0; i < count; i++) {
            printf("%d. %s (%c) — %s\n",
                   i + 1,
                   operations[i].name,
                   operations[i].symbol,
                   operations[i].library_name);
        }

        printf("0. Выход\n\n");
        choice = read_int("Выберите действие: ");

        if (choice == 0) {
            break;
        }

        if (choice < 1 || choice > count) {
            printf("Такого пункта меню нет.\n");
            wait_enter();
            continue;
        }

        a = read_double("Введите первое число: ");
        b = read_double("Введите второе число: ");

        if (operations[choice - 1].symbol == '/' && b == 0) {
            printf("Ошибка: делить на ноль нельзя.\n");
            wait_enter();
            continue;
        }

        result = operations[choice - 1].function(a, b);

        printf("\nРезультат: %.2f %c %.2f = %.2f\n",
               a,
               operations[choice - 1].symbol,
               b,
               result);

        wait_enter();
    } while (choice != 0);

    operations_unload(&operations, &count);
    printf("Работа программы завершена.\n");

    return 0;
}
