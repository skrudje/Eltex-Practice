#include "ipv4.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int parse_packet_count(const char *text, long *count)
{
    char *end;
    long value;

    errno = 0;
    value = strtol(text, &end, 10);

    if (errno != 0 || *text == '\0' || *end != '\0' || value <= 0)
        return -1;

    *count = value;
    return 0;
}

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Использование: %s <IP_шлюза> <маска_подсети> <N>\n"
            "Пример: %s 192.168.1.1 255.255.255.0 10\n",
            program, program);
}

int main(int argc, char *argv[])
{
    uint32_t gateway;
    uint32_t mask;
    long packet_count;

    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    if (ipv4_parse(argv[1], &gateway) == -1) {
        fprintf(stderr, "Ошибка: неверный IPv4-адрес шлюза.\n");
        return 1;
    }

    if (ipv4_parse(argv[2], &mask) == -1 || !ipv4_mask_is_valid(mask)) {
        fprintf(stderr, "Ошибка: неверная маска подсети.\n");
        return 1;
    }

    if (parse_packet_count(argv[3], &packet_count) == -1) {
        fprintf(stderr, "Ошибка: N должно быть положительным целым числом.\n");
        return 1;
    }

    srand((unsigned int)time(NULL));

    uint32_t local_network = gateway & mask;
    char gateway_text[16], mask_text[16], network_text[16];

    ipv4_format(gateway, gateway_text);
    ipv4_format(mask, mask_text);
    ipv4_format(local_network, network_text);

    printf("Шлюз:         %s\n", gateway_text);
    printf("Маска:        %s\n", mask_text);
    printf("Своя подсеть: %s\n\n", network_text);

    long local_count = 0;
    long other_count = 0;

    for (long i = 0; i < packet_count; ++i) {
        uint32_t destination = ipv4_random();
        char destination_text[16];

        ipv4_format(destination, destination_text);

        if (ipv4_same_subnet(gateway, destination, mask)) {
            ++local_count;
            printf("Пакет %ld: %-15s -> своя подсеть\n",
                   i + 1, destination_text);
        } else {
            ++other_count;
            printf("Пакет %ld: %-15s -> другая сеть, отправить через шлюз\n",
                   i + 1, destination_text);
        }
    }

    double local_percent = (double)local_count * 100.0 / (double)packet_count;
    double other_percent = (double)other_count * 100.0 / (double)packet_count;

    printf("\n\n\n--- Статистика ---\n\n");
    printf("Своя подсеть: %ld из %ld (%.2f%%)\n",
           local_count, packet_count, local_percent);
    printf("Другие сети:  %ld из %ld (%.2f%%)\n",
           other_count, packet_count, other_percent);

    return 0;
}
