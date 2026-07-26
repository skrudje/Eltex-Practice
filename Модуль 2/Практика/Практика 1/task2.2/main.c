#include <stdio.h>
#include "calculator.h"

static void clear_screen(void) {
    fputs("\033[H\033[2J", stdout);
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
    int choice;
    double a;
    double b;
    double result;

    clear_screen();

    do {
        printf("----- Калькулятор -----\n\n");
        printf("1. Сложение\n");
        printf("2. Вычитание\n");
        printf("3. Умножение\n");
        printf("4. Деление\n");
        printf("0. Выход\n\n");

        choice = read_int("Выберите действие: ");

        if (choice == 0) {
            break;
        }

        if (choice < 1 || choice > 4) {
            printf("Такого пункта нет.\n");
            wait_enter();
            clear_screen();
            continue;
        }

        a = read_double("Введите первое число: ");
        b = read_double("Введите второе число: ");

        switch (choice) {
            case 1:
                result = calc_summ(a, b);
                printf("Результат: %.2f\n", result);
                break;

            case 2:
                result = calc_subtract(a, b);
                printf("Результат: %.2f\n", result);
                break;

            case 3:
                result = calc_multiply(a, b);
                printf("Результат: %.2f\n", result);
                break;

            case 4:
                if (b == 0) {
                    printf("Ошибка: делить на ноль нельзя.\n");
                } else {
                    result = calc_divide(a, b);
                    printf("Результат: %.2f\n", result);
                }
                break;
        }

        wait_enter();
        clear_screen();
    } while (choice != 0);

    printf("Программа завершена.\n");
    return 0;
}