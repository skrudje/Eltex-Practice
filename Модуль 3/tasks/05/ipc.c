#define _POSIX_C_SOURCE 200809L
#include "ipc.h"
#include "common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static void init_context(IpcContext *context)
{
    context->shm_fd = -1;
    context->semaphore = SEM_FAILED;
    context->memory = NULL;
}

int ipc_create(IpcContext *context)
{
    init_context(context);

    context->shm_fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (context->shm_fd == -1) {
        if (errno == EEXIST) {
            fprintf(stderr,
                    "Ошибка: производитель уже запущен или остался старый POSIX IPC-объект.\n"
                    "Выполните: ./shared_list --cleanup\n");
        } else {
            perror("shm_open");
        }
        return -1;
    }

    if (ftruncate(context->shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        close(context->shm_fd);
        context->shm_fd = -1;
        shm_unlink(SHM_NAME);
        return -1;
    }

    context->memory = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE,
                           MAP_SHARED, context->shm_fd, 0);
    if (context->memory == MAP_FAILED) {
        perror("mmap");
        context->memory = NULL;
        close(context->shm_fd);
        context->shm_fd = -1;
        shm_unlink(SHM_NAME);
        return -1;
    }

    context->semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0600, 1);
    if (context->semaphore == SEM_FAILED) {
        if (errno == EEXIST) {
            fprintf(stderr, "Ошибка: POSIX-семафор с таким именем уже существует.\n");
        } else {
            perror("sem_open");
        }
        munmap(context->memory, SHM_SIZE);
        context->memory = NULL;
        close(context->shm_fd);
        context->shm_fd = -1;
        shm_unlink(SHM_NAME);
        return -1;
    }

    return 0;
}

int ipc_open(IpcContext *context)
{
    init_context(context);

    context->shm_fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (context->shm_fd == -1) {
        fprintf(stderr, "Ошибка: производитель не запущен.\n");
        return -1;
    }

    context->memory = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE,
                           MAP_SHARED, context->shm_fd, 0);
    if (context->memory == MAP_FAILED) {
        perror("mmap");
        context->memory = NULL;
        close(context->shm_fd);
        context->shm_fd = -1;
        return -1;
    }

    context->semaphore = sem_open(SEM_NAME, 0);
    if (context->semaphore == SEM_FAILED) {
        fprintf(stderr, "Ошибка: POSIX-семафор производителя не найден.\n");
        munmap(context->memory, SHM_SIZE);
        context->memory = NULL;
        close(context->shm_fd);
        context->shm_fd = -1;
        return -1;
    }

    return 0;
}

void ipc_detach(IpcContext *context)
{
    if (context->memory != NULL) {
        munmap(context->memory, SHM_SIZE);
        context->memory = NULL;
    }

    if (context->semaphore != SEM_FAILED) {
        sem_close(context->semaphore);
        context->semaphore = SEM_FAILED;
    }

    if (context->shm_fd != -1) {
        close(context->shm_fd);
        context->shm_fd = -1;
    }
}

int ipc_remove(void)
{
    int result = 0;

    if (shm_unlink(SHM_NAME) == -1 && errno != ENOENT) {
        perror("shm_unlink");
        result = -1;
    }

    if (sem_unlink(SEM_NAME) == -1 && errno != ENOENT) {
        perror("sem_unlink");
        result = -1;
    }

    return result;
}

int ipc_cleanup_existing(void)
{
    int removed = 0;

    if (shm_unlink(SHM_NAME) == 0) {
        ++removed;
    } else if (errno != ENOENT) {
        perror("shm_unlink");
        return -1;
    }

    if (sem_unlink(SEM_NAME) == 0) {
        ++removed;
    } else if (errno != ENOENT) {
        perror("sem_unlink");
        return -1;
    }

    printf("Удалено POSIX IPC-объектов: %d\n", removed);
    return 0;
}

int semaphore_lock(sem_t *semaphore)
{
    while (sem_wait(semaphore) == -1) {
        if (errno == EINTR) continue;
        perror("sem_wait");
        return -1;
    }
    return 0;
}

int semaphore_unlock(sem_t *semaphore)
{
    if (sem_post(semaphore) == -1) {
        perror("sem_post");
        return -1;
    }
    return 0;
}
