#include "common.h"
#include <stdio.h>

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
    int minimum, maximum;

    find_min_max(values1, 5, &minimum, &maximum);
    check(minimum == -3 && maximum == 20, "Поиск min/max в массиве");

    find_min_max(values2, 1, &minimum, &maximum);
    check(minimum == 42 && maximum == 42, "Min/max для одного элемента");

    check(sizeof(SharedHeader) < SHM_SIZE, "Заголовок помещается в разделяемую память");
    check(sizeof(BlockHeader) + MIN_ELEMENTS * sizeof(int) < SHM_SIZE,
          "Минимальный блок помещается в разделяемую память");

    printf("\nПройдено: %d\nОшибок: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
