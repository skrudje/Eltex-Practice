#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

#define SHM_SIZE 4096
#define SHM_NAME "/eltex_module3_task5_shm"
#define SEM_NAME "/eltex_module3_task5_sem"
#define MIN_ELEMENTS 10
#define MAX_ELEMENTS 30

typedef struct {
    uint32_t magic;
    size_t first_offset;
    size_t used_bytes;
    int generation_done;
    int consumers_count;
} SharedHeader;

typedef struct {
    size_t next_offset;
    int count;
} BlockHeader;

#define SHARED_MAGIC 0x5441534BU

void sleep_milliseconds(long milliseconds);
long get_delay_from_env(const char *name, long default_value);
void find_min_max(const int *values, int count, int *minimum, int *maximum);

#endif
