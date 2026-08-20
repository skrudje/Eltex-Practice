#include "producer.h"
#include "common.h"
#include "ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static BlockHeader *block_at(void *memory, size_t offset)
{
    return (BlockHeader *)((char *)memory + offset);
}

static int all_blocks_processed(void *memory)
{
    SharedHeader *header = memory;
    size_t offset = header->first_offset;
    while (offset != 0) {
        BlockHeader *block = block_at(memory, offset);
        if (block->count != 0) return 0;
        offset = block->next_offset;
    }
    return 1;
}

static int append_random_block(IpcContext *context, size_t *last_offset, int block_number)
{
    SharedHeader *header = context->memory;
    size_t free_bytes;
    int max_count, count;
    size_t block_size, offset;
    BlockHeader *block;
    int *values;

    if (semaphore_lock(context->sem_id) == -1) return -1;

    free_bytes = SHM_SIZE - header->used_bytes;
    if (free_bytes < sizeof(BlockHeader) + MIN_ELEMENTS * sizeof(int)) {
        semaphore_unlock(context->sem_id);
        return 0;
    }

    max_count = (int)((free_bytes - sizeof(BlockHeader)) / sizeof(int));
    if (max_count > MAX_ELEMENTS) max_count = MAX_ELEMENTS;

    count = MIN_ELEMENTS + rand() % (max_count - MIN_ELEMENTS + 1);
    block_size = sizeof(BlockHeader) + (size_t)count * sizeof(int);
    offset = header->used_bytes;

    block = block_at(context->memory, offset);
    block->next_offset = 0;
    block->count = count;
    values = (int *)(block + 1);

    for (int i = 0; i < count; ++i) values[i] = rand() % 1000;

    if (*last_offset == 0) {
        header->first_offset = offset;
    } else {
        block_at(context->memory, *last_offset)->next_offset = offset;
    }

    *last_offset = offset;
    header->used_bytes += block_size;

    printf("Производитель: создан набор %d, элементов: %d, смещение: %zu\n",
           block_number, count, offset);

    if (semaphore_unlock(context->sem_id) == -1) return -1;
    return 1;
}

int run_producer(void)
{
    IpcContext context;
    SharedHeader *header;
    size_t last_offset = 0;
    int block_count = 0;
    long delay_ms = get_delay_from_env("PRODUCER_DELAY_MS", 100);

    if (ipc_create(&context) == -1) return 1;

    memset(context.memory, 0, SHM_SIZE);
    header = context.memory;
    header->magic = SHARED_MAGIC;
    header->used_bytes = sizeof(SharedHeader);

    srand((unsigned int)(time(NULL) ^ (unsigned int)getpid()));

    printf("Производитель запущен.\n");
    printf("Shared memory ID: %d\n", context.shm_id);
    printf("Semaphore ID:     %d\n", context.sem_id);
    printf("Размер памяти:    %d байт\n\n", SHM_SIZE);

    for (;;) {
        int result = append_random_block(&context, &last_offset, block_count + 1);
        if (result == -1) {
            ipc_detach(&context);
            ipc_remove(&context);
            return 1;
        }
        if (result == 0) break;
        ++block_count;
        sleep_milliseconds(delay_ms);
    }

    if (semaphore_lock(context.sem_id) == -1) {
        ipc_detach(&context);
        ipc_remove(&context);
        return 1;
    }

    header->generation_done = 1;
    printf("\nПамять заполнена. Создано наборов: %d\n", block_count);
    printf("Использовано памяти: %zu из %d байт\n", header->used_bytes, SHM_SIZE);
    semaphore_unlock(context.sem_id);

    printf("Ожидание обработки всех наборов...\n");

    for (;;) {
        int processed, consumers;
        if (semaphore_lock(context.sem_id) == -1) break;
        processed = all_blocks_processed(context.memory);
        consumers = header->consumers_count;
        semaphore_unlock(context.sem_id);

        if (processed && consumers == 0) {
            printf("Все наборы обработаны, потребители завершились.\n");
            break;
        }
        sleep_milliseconds(delay_ms);
    }

    ipc_detach(&context);
    ipc_remove(&context);
    printf("Разделяемая память и семафор удалены.\n");
    return 0;
}
