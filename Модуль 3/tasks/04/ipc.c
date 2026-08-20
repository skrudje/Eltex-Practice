#include "ipc.h"
#include "common.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

static int make_key_file(void)
{
    int fd = open(KEY_FILE, O_CREAT | O_RDWR, 0600);
    if (fd == -1) {
        perror("open key file");
        return -1;
    }
    close(fd);
    return 0;
}

static int get_keys(key_t *shm_key, key_t *sem_key)
{
    if (make_key_file() == -1) return -1;
    *shm_key = ftok(KEY_FILE, SHM_PROJ_ID);
    *sem_key = ftok(KEY_FILE, SEM_PROJ_ID);
    if (*shm_key == (key_t)-1 || *sem_key == (key_t)-1) {
        perror("ftok");
        return -1;
    }
    return 0;
}

int ipc_create(IpcContext *context)
{
    key_t shm_key, sem_key;
    union semun argument;
    context->shm_id = -1;
    context->sem_id = -1;
    context->memory = NULL;

    if (get_keys(&shm_key, &sem_key) == -1) return -1;

    context->shm_id = shmget(shm_key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0600);
    if (context->shm_id == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "Ошибка: производитель уже запущен или остались старые IPC-объекты.\n"
                            "Выполните: ./shared_list --cleanup\n");
        } else {
            perror("shmget");
        }
        return -1;
    }

    context->sem_id = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (context->sem_id == -1) {
        perror("semget");
        shmctl(context->shm_id, IPC_RMID, NULL);
        context->shm_id = -1;
        return -1;
    }

    argument.val = 1;
    if (semctl(context->sem_id, 0, SETVAL, argument) == -1) {
        perror("semctl SETVAL");
        shmctl(context->shm_id, IPC_RMID, NULL);
        semctl(context->sem_id, 0, IPC_RMID);
        return -1;
    }

    context->memory = shmat(context->shm_id, NULL, 0);
    if (context->memory == (void *)-1) {
        perror("shmat");
        context->memory = NULL;
        shmctl(context->shm_id, IPC_RMID, NULL);
        semctl(context->sem_id, 0, IPC_RMID);
        return -1;
    }
    return 0;
}

int ipc_open(IpcContext *context)
{
    key_t shm_key, sem_key;
    context->shm_id = -1;
    context->sem_id = -1;
    context->memory = NULL;

    if (get_keys(&shm_key, &sem_key) == -1) return -1;

    context->shm_id = shmget(shm_key, SHM_SIZE, 0600);
    if (context->shm_id == -1) {
        fprintf(stderr, "Ошибка: производитель не запущен.\n");
        return -1;
    }

    context->sem_id = semget(sem_key, 1, 0600);
    if (context->sem_id == -1) {
        fprintf(stderr, "Ошибка: семафор производителя не найден.\n");
        return -1;
    }

    context->memory = shmat(context->shm_id, NULL, 0);
    if (context->memory == (void *)-1) {
        perror("shmat");
        context->memory = NULL;
        return -1;
    }
    return 0;
}

void ipc_detach(IpcContext *context)
{
    if (context->memory != NULL) {
        shmdt(context->memory);
        context->memory = NULL;
    }
}

int ipc_remove(IpcContext *context)
{
    int result = 0;
    if (context->shm_id != -1 && shmctl(context->shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID");
        result = -1;
    }
    if (context->sem_id != -1 && semctl(context->sem_id, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
        result = -1;
    }
    unlink(KEY_FILE);
    return result;
}

int ipc_cleanup_existing(void)
{
    key_t shm_key, sem_key;
    int shm_id, sem_id, removed = 0;
    if (get_keys(&shm_key, &sem_key) == -1) return -1;

    shm_id = shmget(shm_key, SHM_SIZE, 0600);
    if (shm_id != -1 && shmctl(shm_id, IPC_RMID, NULL) == 0) ++removed;

    sem_id = semget(sem_key, 1, 0600);
    if (sem_id != -1 && semctl(sem_id, 0, IPC_RMID) == 0) ++removed;

    unlink(KEY_FILE);
    printf("Удалено IPC-объектов: %d\n", removed);
    return 0;
}

static int semaphore_change(int sem_id, short operation)
{
    struct sembuf action = {0, operation, SEM_UNDO};
    while (semop(sem_id, &action, 1) == -1) {
        if (errno == EINTR) continue;
        if (errno != EIDRM && errno != EINVAL) perror("semop");
        return -1;
    }
    return 0;
}

int semaphore_lock(int sem_id) { return semaphore_change(sem_id, -1); }
int semaphore_unlock(int sem_id) { return semaphore_change(sem_id, 1); }
