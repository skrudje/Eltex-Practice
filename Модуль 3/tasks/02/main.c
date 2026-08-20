#include "broker.h"
#include "publisher.h"
#include "subscriber.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *program_name)
{
    fprintf(stderr,
            "Использование:\n"
            "  %s -b\n"
            "  %s -p <topic>\n"
            "  %s -s <topic1> [topic2 ...]\n",
            program_name, program_name, program_name);
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-b") == 0) {
        if (argc != 2) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        return run_broker();
    }

    if (strcmp(argv[1], "-p") == 0) {
        if (argc != 3) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        return run_publisher(argv[2]);
    }

    if (strcmp(argv[1], "-s") == 0) {
        if (argc < 3) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        return run_subscriber(&argv[2], argc - 2);
    }

    print_usage(argv[0]);
    return EXIT_FAILURE;
}
