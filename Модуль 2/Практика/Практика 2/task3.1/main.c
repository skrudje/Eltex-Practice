#include "permissions.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define INPUT_SIZE 512

static void remove_newline(char *text)
{
    text[strcspn(text, "\n")] = '\0';
}

static int read_line(const char *prompt, char *buffer, size_t size)
{
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) return -1;
    remove_newline(buffer);
    return 0;
}

static void print_permissions(mode_t mode)
{
    char symbolic[10], binary[10], octal[4];

    permissions_to_symbolic(mode, symbolic);
    permissions_to_binary(mode, binary);
    permissions_to_octal(mode, octal);

    printf("Буквенное представление: %s\n", symbolic);
    printf("Цифровое представление: %s\n", octal);
    printf("Битовое представление:   %s\n", binary);
}

int main(void)
{
    mode_t current_mode = 0;
    int has_current_mode = 0;
    char input[INPUT_SIZE];

    for (;;) {
        printf("\n\n\n--- Права доступа к файлу ---\n\n");
        printf("1. Ввести права доступа\n");
        printf("2. Получить права доступа файла\n");
        printf("3. Изменить текущую маску прав\n");
        printf("0. Выход\n");

        if (read_line("Выберите действие: ", input, sizeof(input)) == -1) break;

        if (strcmp(input, "0") == 0) break;

        if (strcmp(input, "1") == 0) {
            if (read_line("Введите права (например 755 или rwxr-xr-x): ",
                          input, sizeof(input)) == -1) break;

            if (permissions_parse(input, &current_mode) == -1) {
                fprintf(stderr, "Ошибка: неверное обозначение прав доступа.\n");
                continue;
            }

            has_current_mode = 1;
            print_permissions(current_mode);
        } else if (strcmp(input, "2") == 0) {
            if (read_line("Введите имя файла: ", input, sizeof(input)) == -1) break;

            if (permissions_from_file(input, &current_mode) == -1) {
                perror("stat");
                continue;
            }

            has_current_mode = 1;
            print_permissions(current_mode);
            printf("Для сравнения выполните: ls -l -- \"%s\"\n", input);
        } else if (strcmp(input, "3") == 0) {
            if (!has_current_mode) {
                printf("Сначала задайте права через пункт 1 или 2.\n");
                continue;
            }

            print_permissions(current_mode);

            if (read_line("Команда изменения (например u+x,g-w,o=r): ",
                          input, sizeof(input)) == -1) break;

            if (permissions_modify(&current_mode, input) == -1) {
                fprintf(stderr,
                        "Ошибка: используйте формат [ugoa][+-=][rwx], "
                        "например u+x,g-w,o=r.\n");
                continue;
            }
            printf("Результат изменения маски:\n");
            print_permissions(current_mode);
            printf("Права реального файла не изменялись.\n");
        } else {
            printf("Неизвестная команда.\n");
        }
    }
    return 0;
}
