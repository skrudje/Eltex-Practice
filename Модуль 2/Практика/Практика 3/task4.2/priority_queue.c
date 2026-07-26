#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "priority_queue.h"

void queue_init(PriorityQueue *queue) {
    queue->head = NULL;
    queue->count = 0;
}

int queue_push(PriorityQueue *queue, int id, int priority, const char *text) {
    QueueNode *node;
    QueueNode *current;

    if (priority < 0 || priority > 255 || text == NULL) {
        return 0;
    }

    node = malloc(sizeof(QueueNode));
    if (node == NULL) {
        return 0;
    }

    node->message.id = id;
    node->message.priority = priority;
    strncpy(node->message.text, text, MESSAGE_TEXT_SIZE - 1);
    node->message.text[MESSAGE_TEXT_SIZE - 1] = '\0';
    node->next = NULL;

    if (queue->head == NULL || priority > queue->head->message.priority) {
        node->next = queue->head;
        queue->head = node;
    } else {
        current = queue->head;

        while (current->next != NULL &&
               current->next->message.priority >= priority) {
            current = current->next;
        }

        node->next = current->next;
        current->next = node;
    }
    queue->count++;
    return 1;
}

int queue_pop_first(PriorityQueue *queue, Message *result) {
    QueueNode *node;

    if (queue->head == NULL || result == NULL) {
        return 0;
    }

    node = queue->head;
    *result = node->message;
    queue->head = node->next;
    free(node);
    queue->count--;

    return 1;
}

int queue_pop_priority(PriorityQueue *queue, int priority, Message *result) {
    QueueNode *current;
    QueueNode *previous = NULL;

    if (priority < 0 || priority > 255 || result == NULL) {
        return 0;
    }

    current = queue->head;

    while (current != NULL && current->message.priority != priority) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        return 0;
    }

    if (previous == NULL) {
        queue->head = current->next;
    } else {
        previous->next = current->next;
    }

    *result = current->message;
    free(current);
    queue->count--;

    return 1;
}

int queue_pop_min_priority(PriorityQueue *queue, int min_priority, Message *result) {
    QueueNode *current;
    QueueNode *previous = NULL;

    if (min_priority < 0 || min_priority > 255 || result == NULL) {
        return 0;
    }

    if (queue->head == NULL || queue->head->message.priority <= min_priority) {
        return 0;
    }

    current = queue->head;

    while (current->next != NULL && current->next->message.priority > min_priority) {
        previous = current;
        current = current->next;
    }

    if (previous == NULL) {
        queue->head = current->next;
    } else {
        previous->next = current->next;
    }

    *result = current->message;
    free(current);
    queue->count--;

    return 1;
}

void queue_print(const PriorityQueue *queue) {
    const QueueNode *current = queue->head;

    if (current == NULL) {
        printf("Очередь пуста.\n");
        return;
    }

    printf("\nСообщения в очереди:\n");

    while (current != NULL) {
        printf("ID: %d | Приоритет: %d | %s\n",
               current->message.id,
               current->message.priority,
               current->message.text);
        current = current->next;
    }
}

void queue_clear(PriorityQueue *queue) {
    QueueNode *current = queue->head;

    while (current != NULL) {
        QueueNode *next = current->next;
        free(current);
        current = next;
    }

    queue->head = NULL;
    queue->count = 0;
}
int queue_is_empty(const PriorityQueue *queue) {
    return queue->head == NULL;
}
int queue_count(const PriorityQueue *queue) {
    return queue->count;
}
