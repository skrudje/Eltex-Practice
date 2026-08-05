#define _POSIX_C_SOURCE 200809L

#include "channels.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void print_usage(const char *program_name) {
    fprintf(stderr,
            "Использование:\n"
            "  %s файл1 [файл2 ...]\n"
            "  %s -p имя_канала файл1 [файл2 ...]\n",
            program_name,
            program_name);
}

int main(int argc, char *argv[]) {
    const char *channel_name = NULL;
    int option;

    while ((option = getopt(argc, argv, "p:")) != -1) {
        switch (option) {
            case 'p':
                channel_name = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Не указаны файлы для копирования.\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    char **file_names = &argv[optind];
    int file_count = argc - optind;

    if (channel_name != NULL) {
        return run_named_pipe_mode(channel_name,
                                   file_names,
                                   file_count);
    }
    return run_unnamed_pipe_mode(file_names, file_count);
}
