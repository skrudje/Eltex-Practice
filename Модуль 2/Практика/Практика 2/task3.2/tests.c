#include "ipv4.h"
#include <stdint.h>
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
    uint32_t address, gateway, mask, destination;
    char text[16];

    check(ipv4_parse("192.168.1.10", &address) == 0 &&
          address == 0xC0A8010A,
          "Преобразование IPv4 в 32-битное число");

    ipv4_format(0xC0A8010A, text);
    check(strcmp(text, "192.168.1.10") == 0,
          "Преобразование числа обратно в IPv4");

    check(ipv4_parse("300.1.1.1", &address) == -1,
          "Отклонение неверного IPv4");

    check(ipv4_parse("255.255.255.0", &mask) == 0 &&
          ipv4_mask_is_valid(mask),
          "Корректная маска /24");

    check(ipv4_parse("255.0.255.0", &mask) == 0 &&
          !ipv4_mask_is_valid(mask),
          "Отклонение разорванной маски");

    ipv4_parse("192.168.1.1", &gateway);
    ipv4_parse("255.255.255.0", &mask);
    ipv4_parse("192.168.1.200", &destination);

    check(ipv4_same_subnet(gateway, destination, mask),
          "Адрес принадлежит своей подсети");

    ipv4_parse("192.168.2.1", &destination);

    check(!ipv4_same_subnet(gateway, destination, mask),
          "Адрес принадлежит другой сети");

    printf("\nПройдено: %d\nОшибок: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
