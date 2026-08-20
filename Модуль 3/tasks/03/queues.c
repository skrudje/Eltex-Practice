#define _POSIX_C_SOURCE 200809L

#include "queues.h"
#include "protocol.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define OPEN_RETRY_COUNT 30
#define OPEN_RETRY_DELAY_NS 100000000L

static int open_existing_with_retry(const char *name, mqd_t *queue)
{
    struct timespec delay = {0, OPEN_RETRY_DELAY_NS};

    for (int attempt = 0; attempt < OPEN_RETRY_COUNT; ++attempt) {
        *queue = mq_open(name, O_RDWR);
        if (*queue != (mqd_t)-1) {
            return 0;
        }

        if (errno != ENOENT) {
            return -1;
        }
        nanosleep(&delay, NULL);
    }

    errno = ENOENT;
    return -1;
}

int queue_build_names(const char *base_name,
                      char *first_name,
                      size_t first_size,
                      char *second_name,
                      size_t second_size)
{
    char normalized[CHAT_QUEUE_NAME_SIZE];
    int written;

    if (base_name == NULL || base_name[0] == '\0' ||
        first_name == NULL || second_name == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (base_name[0] == '/') {
        written = snprintf(normalized, sizeof(normalized), "%s", base_name);
    } else {
        written = snprintf(normalized, sizeof(normalized), "/%s", base_name);
    }

    if (written < 0 || (size_t)written >= sizeof(normalized)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    if (strchr(normalized + 1, '/') != NULL) {
        errno = EINVAL;
        return -1;
    }

    written = snprintf(first_name, first_size, "%s_1", normalized);
    if (written < 0 || (size_t)written >= first_size) {
        errno = ENAMETOOLONG;
        return -1;
    }

    written = snprintf(second_name, second_size, "%s_2", normalized);
    if (written < 0 || (size_t)written >= second_size) {
        errno = ENAMETOOLONG;
        return -1;
    }
    return 0;
}

int queues_open(ChatQueues *queues, const char *base_name)
{
    struct mq_attr attributes = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = CHAT_MESSAGE_SIZE,
        .mq_curmsgs = 0
    };
    mqd_t first_queue = (mqd_t)-1;
    mqd_t second_queue = (mqd_t)-1;

    if (queues == NULL) {
        errno = EINVAL;
        return -1;
    }

    queues->send_queue = (mqd_t)-1;
    queues->receive_queue = (mqd_t)-1;
    queues->creator = false;

    if (queue_build_names(base_name,
                          queues->first_name,
                          sizeof(queues->first_name),
                          queues->second_name,
                          sizeof(queues->second_name)) == -1) {
        return -1;
    }

    first_queue = mq_open(queues->first_name,
                          O_CREAT | O_EXCL | O_RDWR,
                          0600,
                          &attributes);

    if (first_queue != (mqd_t)-1) {
        queues->creator = true;

        second_queue = mq_open(queues->second_name,
                               O_CREAT | O_EXCL | O_RDWR,
                               0600,
                               &attributes);
        if (second_queue == (mqd_t)-1) {
            int saved_errno = errno;
            mq_close(first_queue);
            mq_unlink(queues->first_name);
            errno = saved_errno;
            return -1;
        }

        queues->receive_queue = first_queue;
        queues->send_queue = second_queue;
        return 0;
    }

    if (errno != EEXIST) {
        return -1;
    }

    if (open_existing_with_retry(queues->first_name, &first_queue) == -1) {
        return -1;
    }

    if (open_existing_with_retry(queues->second_name, &second_queue) == -1) {
        int saved_errno = errno;
        mq_close(first_queue);
        errno = saved_errno;
        return -1;
    }

    queues->send_queue = first_queue;
    queues->receive_queue = second_queue;
    return 0;
}

void queues_close(ChatQueues *queues)
{
    if (queues == NULL) {
        return;
    }

    if (queues->send_queue != (mqd_t)-1) {
        mq_close(queues->send_queue);
        queues->send_queue = (mqd_t)-1;
    }

    if (queues->receive_queue != (mqd_t)-1) {
        mq_close(queues->receive_queue);
        queues->receive_queue = (mqd_t)-1;
    }
}

void queues_unlink(const ChatQueues *queues)
{
    if (queues == NULL || !queues->creator) {
        return;
    }

    if (mq_unlink(queues->first_name) == -1 && errno != ENOENT) {
        perror("mq_unlink первой очереди");
    }

    if (mq_unlink(queues->second_name) == -1 && errno != ENOENT) {
        perror("mq_unlink второй очереди");
    }
}
