#include "consumer.h"
#include "ipc.h"
#include "producer.h"
#include <stdio.h>
#include <string.h>

static void print_usage(const char *program)
{
    printf("Использование:\n");
    printf("  %s -p          запустить производителя\n", program);
    printf("  %s -c          запустить потребителя\n", program);
    printf("  %s --cleanup   удалить оставшиеся IPC-объекты\n", program);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "-p") == 0) return run_producer();
    if (strcmp(argv[1], "-c") == 0) return run_consumer();
    if (strcmp(argv[1], "--cleanup") == 0) return ipc_cleanup_existing() == 0 ? 0 : 1;
    print_usage(argv[0]);
    return 1;
}
