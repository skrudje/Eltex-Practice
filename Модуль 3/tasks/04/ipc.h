#ifndef IPC_H
#define IPC_H

typedef struct {
    int shm_id;
    int sem_id;
    void *memory;
} IpcContext;

int ipc_create(IpcContext *context);
int ipc_open(IpcContext *context);
void ipc_detach(IpcContext *context);
int ipc_remove(IpcContext *context);
int ipc_cleanup_existing(void);
int semaphore_lock(int sem_id);
int semaphore_unlock(int sem_id);

#endif
