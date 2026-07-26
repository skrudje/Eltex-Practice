#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#define MESSAGE_TEXT_SIZE 100

typedef struct {
    int id;
    int priority;
    char text[MESSAGE_TEXT_SIZE];
} Message;

typedef struct QueueNode {
    Message message;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *head;
    int count;
} PriorityQueue;

void queue_init(PriorityQueue *queue);
int queue_push(PriorityQueue *queue, int id, int priority, const char *text);
int queue_pop_first(PriorityQueue *queue, Message *result);
int queue_pop_priority(PriorityQueue *queue, int priority, Message *result);
int queue_pop_min_priority(PriorityQueue *queue, int min_priority, Message *result);
void queue_print(const PriorityQueue *queue);
void queue_clear(PriorityQueue *queue);
int queue_is_empty(const PriorityQueue *queue);
int queue_count(const PriorityQueue *queue);

#endif
