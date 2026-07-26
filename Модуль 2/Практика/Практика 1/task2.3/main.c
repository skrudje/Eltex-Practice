#include <stdio.h>
#include <stdlib.h>
#include "calculator.h"

static void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

static int read_int(const char *message) {
    char line[50];
    int value;

    while (1) {
        printf("%s", message);

        if (fgets(line, sizeof(line), stdin) != NULL &&
            sscanf(line, "%d", &value) == 1) {
            return value;
        }

        clear_screen();
        printf("Ошибка: введите целое число.\n");
    }
}

static double read_double(const char *message) {
    char line[50];
    double value;

    while (1) {
        printf("%s", message);

        if (fgets(line, sizeof(line), stdin) != NULL &&
            sscanf(line, "%lf", &value) == 1) {
            return value;
        }

        clear_screen();
        printf("Ошибка: введите число.\n");
    }
}

static int ask_operation(const char *name) {
    int answer;

    do {
        printf("Добавить операцию \"%s\"?\n", name);
        printf("1. Да\n");
        printf("0. Нет\n");
        answer = read_int("Ваш выбор: ");

        if (answer != 0 && answer != 1) {
            printf("Введите 1 или 0.\n\n");
        }
    } while (answer != 0 && answer != 1);
    return answer;
}

static int add_selected_operation(Operation **operations,
                                  int *count,
                                  const char *name,
                                  char symbol,
                                  OperationFunction function) {
    if (!operation_add(operations, count, name, symbol, function)) {
        printf("Ошибка выделения памяти.\n");
        return 0;
    }
    return 1;
}

int main(void) {
    Operation *operations = NULL;
    int operation_count = 0;
    int choice;
    int i;
    double a;
    double b;
    double result;

    printf("Сначала выберите доступные операции.\n\n");

    if (ask_operation("Сложение") &&
        !add_selected_operation(&operations, &operation_count,
                                "Сложение", '+', calc_summ)) {
        free(operations);
        return 1;
    }

    if (ask_operation("Вычитание") &&
        !add_selected_operation(&operations, &operation_count,
                                "Вычитание", '-', calc_subtract)) {
        free(operations);
        return 1;
    }

    if (ask_operation("Умножение") &&
        !add_selected_operation(&operations, &operation_count,
                                "Умножение", '*', calc_multiply)) {
        free(operations);
        return 1;
    }

    if (ask_operation("Деление") &&
        !add_selected_operation(&operations, &operation_count,
                                "Деление", '/', calc_divide)) {
        free(operations);
        return 1;
    }

    if (ask_operation("Рандомная функция") &&
        !add_selected_operation(&operations, &operation_count,
                                 "Рандом", '/', calc_divide)) {
        free(operations);
        return 1;
     }

    if (operation_count == 0) {
        printf("Не выбрано ни одной операции.\n");
        free(operations);
        return 0;
    }

    do {
        printf("\n---- Калькулятор ----\n\n");

        for (i = 0; i < operation_count; i++) {
            printf("%d. %s (%c)\n",
                   i + 1,
                   operations[i].name,
                   operations[i].symbol);
        }

        printf("0. Выход\n\n");
        choice = read_int("Выберите действие: ");

        if (choice == 0) {
            break;
        }

        if (choice < 1 || choice > operation_count) {
            printf("Такого пункта нет.\n\n");
            continue;
        }

        a = read_double("Введите первое число: ");
        b = read_double("Введите второе число: ");

        if (operations[choice - 1].symbol == '/' && b == 0) {
            printf("Ошибка: делить на ноль нельзя.\n\n");
            continue;
        }

        result = operations[choice - 1].function(a, b);

        printf("Результат: %.2f %c %.2f = %.2f\n\n",
               a,
               operations[choice - 1].symbol,
               b,
               result);
    } while (choice != 0);

    free(operations);
    printf("Программа завершена.\n");
    return 0;
}
