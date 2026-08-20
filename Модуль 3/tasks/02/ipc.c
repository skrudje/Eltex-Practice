#include "ipc.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <unistd.h>

static int prepare_key_file(int create)
{
    int descriptor;

    if (create) {
        descriptor = open(QUEUE_KEY_FILE, O_CREAT | O_RDWR, 0600);
    } else {
        descriptor = open(QUEUE_KEY_FILE, O_RDONLY);
    }

    if (descriptor == -1) {
        return -1;
    }

    close(descriptor);
    return 0;
}

static key_t get_queue_key(int create_key_file)
{
    if (prepare_key_file(create_key_file) == -1) {
        return (key_t)-1;
    }

    return ftok(QUEUE_KEY_FILE, QUEUE_PROJECT_ID);
}

int create_broker_queue(void)
{
    key_t key = get_queue_key(1);

    if (key == (key_t)-1) {
        perror("ftok");
        return -1;
    }

    return msgget(key, IPC_CREAT | IPC_EXCL | 0660);
}

int connect_to_broker_queue(void)
{
    key_t key = get_queue_key(0);

    if (key == (key_t)-1) {
        fprintf(stderr, "Брокер не запущен: очередь сообщений не найдена.\n");
        return -1;
    }

    return msgget(key, 0660);
}

int send_queue_text(int queue_id, long type, const char *text)
{
    QueueMessage message;
    size_t length;

    if (text == NULL || type <= 0) {
        return -1;
    }

    length = strlen(text) + 1;
    if (length > sizeof(message.text)) {
        fprintf(stderr, "Сообщение слишком длинное.\n");
        return -1;
    }

    message.type = type;
    memcpy(message.text, text, length);

    while (msgsnd(queue_id, &message, length, 0) == -1) {
        if (errno == EINTR) {
            continue;
        }

        if (errno == EIDRM || errno == EINVAL) {
            fprintf(stderr, "Очередь сообщений стала недоступна.\n");
        } else {
            perror("msgsnd");
        }
        return -1;
    }

    return 0;
}

int remove_queue(int queue_id)
{
    int result = 0;

    if (msgctl(queue_id, IPC_RMID, NULL) == -1) {
        if (errno != EIDRM && errno != EINVAL) {
            perror("msgctl IPC_RMID");
            result = -1;
        }
    }

    unlink(QUEUE_KEY_FILE);
    return result;
}
