#include "chat.h"
#include "queues.h"

#include <stdio.h>
#include <stdlib.h>

static void print_usage(const char *program_name)
{
    fprintf(stderr, "Использование: %s <имя_очереди>\n", program_name);
    fprintf(stderr, "Пример: %s /my_chat\n", program_name);
}

int main(int argc, char *argv[])
{
    ChatQueues queues;
    int result;

    if (argc != 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (queues_open(&queues, argv[1]) == -1) {
        perror("Не удалось открыть очереди");
        return EXIT_FAILURE;
    }

    printf("Роль: %s.\n", queues.creator ? "создатель очередей" : "второй участник");
    printf("Очередь 1: %s\n", queues.first_name);
    printf("Очередь 2: %s\n", queues.second_name);
    fflush(stdout);

    result = chat_run(&queues);

    queues_close(&queues);
    queues_unlink(&queues);

    if (queues.creator) {
        printf("Очереди удалены.\n");
    }

    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
