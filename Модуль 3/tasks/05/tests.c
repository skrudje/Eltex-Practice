#include "common.h"
#include <stdio.h>
#include <string.h>

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
    int values1[] = {10, 5, 20, -3, 7};
    int values2[] = {42};
    int minimum;
    int maximum;

    find_min_max(values1, 5, &minimum, &maximum);
    check(minimum == -3 && maximum == 20, "Поиск min/max в массиве");

    find_min_max(values2, 1, &minimum, &maximum);
    check(minimum == 42 && maximum == 42, "Min/max для одного элемента");

    check(sizeof(SharedHeader) < SHM_SIZE,
          "Заголовок помещается в разделяемую память");

    check(sizeof(BlockHeader) + MIN_ELEMENTS * sizeof(int) < SHM_SIZE,
          "Минимальный блок помещается в разделяемую память");

    check(SHM_NAME[0] == '/' && SEM_NAME[0] == '/',
          "Имена POSIX IPC-объектов начинаются с /");

    check(strcmp(SHM_NAME, SEM_NAME) != 0,
          "Разделяемая память и семафор имеют разные имена");

    printf("\nПройдено: %d\nОшибок: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
