#include "permissions.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

static int passed = 0;
static int failed = 0;

static void check(int condition, const char *name)
{
    if (condition) {
        printf("[OK] %s\n", name);
        ++passed;
    } else {
        printf("[FAIL] %s\n", name);
        ++failed;
    }
}

int main(void)
{
    mode_t mode;
    char text[10];
    char octal[4];

    check(permissions_parse("755", &mode) == 0 && mode == 0755,
          "Разбор цифровых прав 755");
    check(permissions_parse("rwxr-xr-x", &mode) == 0 && mode == 0755,
          "Разбор буквенных прав rwxr-xr-x");
    check(permissions_parse("89x", &mode) == -1,
          "Отклонение неверных цифровых прав");

    permissions_to_binary(0755, text);
    check(strcmp(text, "111101101") == 0,
          "Битовое представление 755");

    permissions_to_symbolic(0640, text);
    check(strcmp(text, "rw-r-----") == 0,
          "Буквенное представление 640");

    permissions_to_octal(0640, octal);
    check(strcmp(octal, "640") == 0,
          "Цифровое представление 640");

    mode = 0644;
    check(permissions_modify(&mode, "u+x") == 0 && mode == 0744,
          "Модификатор u+x");

    mode = 0777;
    check(permissions_modify(&mode, "g-w,o=rx") == 0 && mode == 0755,
          "Несколько chmod-подобных модификаторов");

    mode = 0644;
    check(permissions_modify(&mode, "a=") == 0 && mode == 0000,
          "Модификатор a очищает права");

    printf("\nПройдено: %d\nОшибок: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
