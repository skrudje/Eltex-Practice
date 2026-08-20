#include "consumer.h"
#include "common.h"
#include "ipc.h"
#include <stdio.h>
#include <unistd.h>

static BlockHeader *block_at(void *memory, size_t offset)
{
    return (BlockHeader *)((char *)memory + offset);
}

static int register_consumer(IpcContext *context)
{
    SharedHeader *header = context->memory;

    if (semaphore_lock(context->semaphore) == -1) return -1;

    if (header->magic != SHARED_MAGIC) {
        semaphore_unlock(context->semaphore);
        fprintf(stderr, "Ошибка: разделяемая память еще не инициализирована.\n");
        return -1;
    }

    ++header->consumers_count;

    if (semaphore_unlock(context->semaphore) == -1) return -1;
    return 0;
}

static int unregister_consumer(IpcContext *context)
{
    SharedHeader *header = context->memory;

    if (semaphore_lock(context->semaphore) == -1) return -1;

    if (header->consumers_count > 0) {
        --header->consumers_count;
    }

    if (semaphore_unlock(context->semaphore) == -1) return -1;
    return 0;
}

static int process_one_block(IpcContext *context, size_t *processed_offset,
                             int *minimum, int *maximum, int *element_count,
                             int *should_finish)
{
    SharedHeader *header = context->memory;
    size_t offset;

    *processed_offset = 0;
    *should_finish = 0;

    if (semaphore_lock(context->semaphore) == -1) return -1;

    offset = header->first_offset;

    while (offset != 0) {
        BlockHeader *block = block_at(context->memory, offset);

        if (block->count > 0) {
            int *values = (int *)(block + 1);

            *element_count = block->count;
            find_min_max(values, block->count, minimum, maximum);

            block->count = 0;
            *processed_offset = offset;

            if (semaphore_unlock(context->semaphore) == -1) return -1;
            return 1;
        }

        offset = block->next_offset;
    }

    if (header->generation_done) {
        *should_finish = 1;
    }

    if (semaphore_unlock(context->semaphore) == -1) return -1;
    return 0;
}

int run_consumer(void)
{
    IpcContext context;
    long delay_ms = get_delay_from_env("CONSUMER_DELAY_MS", 150);

    if (ipc_open(&context) == -1) return 1;

    if (register_consumer(&context) == -1) {
        ipc_detach(&context);
        return 1;
    }

    printf("Потребитель PID=%ld подключен.\n", (long)getpid());

    for (;;) {
        size_t offset;
        int minimum = 0;
        int maximum = 0;
        int count = 0;
        int should_finish = 0;
        int result;

        result = process_one_block(&context, &offset, &minimum, &maximum,
                                   &count, &should_finish);

        if (result == -1) {
            ipc_detach(&context);
            return 1;
        }

        if (result == 1) {
            printf("PID=%ld: обработан блок offset=%zu, элементов=%d, min=%d, max=%d\n",
                   (long)getpid(), offset, count, minimum, maximum);
            sleep_milliseconds(delay_ms);
            continue;
        }

        if (should_finish) break;

        sleep_milliseconds(delay_ms);
    }

    unregister_consumer(&context);
    ipc_detach(&context);

    printf("Потребитель PID=%ld завершил работу.\n", (long)getpid());
    return 0;
}
