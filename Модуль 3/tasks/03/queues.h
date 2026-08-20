#ifndef QUEUES_H
#define QUEUES_H

#include <mqueue.h>
#include <stdbool.h>
#include <stddef.h>

#define CHAT_QUEUE_NAME_SIZE 256

typedef struct {
    mqd_t send_queue;
    mqd_t receive_queue;
    bool creator;
    char first_name[CHAT_QUEUE_NAME_SIZE];
    char second_name[CHAT_QUEUE_NAME_SIZE];
} ChatQueues;

int queue_build_names(const char *base_name,
                      char *first_name,
                      size_t first_size,
                      char *second_name,
                      size_t second_size);

int queues_open(ChatQueues *queues, const char *base_name);
void queues_close(ChatQueues *queues);
void queues_unlink(const ChatQueues *queues);

#endif
