#include "chat.h"
#include "common.h"

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_usage(const char *program)
{
    printf("Использование: %s [-n имя] [-a broadcast_ip] [-p порт]\n", program);
    printf("\n");
    printf("  -n имя           имя участника чата\n");
    printf("  -a broadcast_ip  широковещательный IPv4-адрес\n");
    printf("  -p порт          UDP-порт (1024..65535)\n");
    printf("  -h               показать справку\n");
    printf("\n");
    printf("По умолчанию: broadcast=%s, port=%d\n",
           DEFAULT_BROADCAST, DEFAULT_PORT);
}

int main(int argc, char **argv)
{
    ChatConfig config;
    int option;

    memset(&config, 0, sizeof(config));
    make_default_name(config.name, sizeof(config.name));
    snprintf(config.broadcast_ip, sizeof(config.broadcast_ip), "%s",
             DEFAULT_BROADCAST);
    config.port = DEFAULT_PORT;

    while ((option = getopt(argc, argv, "n:a:p:h")) != -1) {
        switch (option) {
        case 'n':
            if (strlen(optarg) >= sizeof(config.name)) {
                fprintf(stderr, "Имя слишком длинное (максимум %zu символов)\n",
                        sizeof(config.name) - 1);
                return 1;
            }
            strcpy(config.name, optarg);
            break;

        case 'a':
            if (strlen(optarg) >= sizeof(config.broadcast_ip)) {
                fprintf(stderr, "Некорректный IPv4-адрес\n");
                return 1;
            }
            strcpy(config.broadcast_ip, optarg);
            break;

        case 'p':
            if (parse_port(optarg, &config.port) == -1) {
                fprintf(stderr, "Некорректный порт: %s\n", optarg);
                return 1;
            }
            break;

        case 'h':
            print_usage(argv[0]);
            return 0;

        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind != argc) {
        print_usage(argv[0]);
        return 1;
    }

    return run_chat(&config);
}
