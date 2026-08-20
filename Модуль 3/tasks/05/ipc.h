#ifndef IPC_H
#define IPC_H

#include <semaphore.h>

typedef struct {
    int shm_fd;
    sem_t *semaphore;
    void *memory;
} IpcContext;

int ipc_create(IpcContext *context);
int ipc_open(IpcContext *context);
void ipc_detach(IpcContext *context);
int ipc_remove(void);
int ipc_cleanup_existing(void);
int semaphore_lock(sem_t *semaphore);
int semaphore_unlock(sem_t *semaphore);

#endif
