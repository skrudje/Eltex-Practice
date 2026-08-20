#include "broker.h"

#include "ipc.h"
#include "protocol.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <time.h>
#include <unistd.h>

typedef struct Subscription {
    pid_t pid;
    char topic[MAX_TOPIC_LENGTH];
    struct Subscription *next;
} Subscription;

typedef struct Publisher {
    pid_t pid;
    struct Publisher *next;
} Publisher;

static volatile sig_atomic_t stop_requested = 0;

static void handle_stop_signal(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

static int install_signal_handlers(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_stop_signal;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1 ||
        sigaction(SIGTERM, &action, NULL) == -1) {
        perror("sigaction");
        return -1;
    }

    return 0;
}

static int subscription_exists(const Subscription *head,
                               pid_t pid, const char *topic)
{
    while (head != NULL) {
        if (head->pid == pid && strcmp(head->topic, topic) == 0) {
            return 1;
        }
        head = head->next;
    }

    return 0;
}

static int add_subscription(Subscription **head,
                            pid_t pid, const char *topic)
{
    Subscription *node;

    if (subscription_exists(*head, pid, topic)) {
        return 0;
    }

    node = malloc(sizeof(*node));
    if (node == NULL) {
        perror("malloc");
        return -1;
    }

    node->pid = pid;
    strcpy(node->topic, topic);
    node->next = *head;
    *head = node;
    return 0;
}

static void remove_subscription(Subscription **head,
                                pid_t pid, const char *topic)
{
    Subscription *current = *head;
    Subscription *previous = NULL;

    while (current != NULL) {
        if (current->pid == pid && strcmp(current->topic, topic) == 0) {
            if (previous == NULL) {
                *head = current->next;
            } else {
                previous->next = current->next;
            }
            free(current);
            return;
        }

        previous = current;
        current = current->next;
    }
}

static int publisher_exists(const Publisher *head, pid_t pid)
{
    while (head != NULL) {
        if (head->pid == pid) {
            return 1;
        }
        head = head->next;
    }

    return 0;
}

static int add_publisher(Publisher **head, pid_t pid)
{
    Publisher *node;

    if (publisher_exists(*head, pid)) {
        return 0;
    }

    node = malloc(sizeof(*node));
    if (node == NULL) {
        perror("malloc");
        return -1;
    }

    node->pid = pid;
    node->next = *head;
    *head = node;
    return 0;
}

static void forward_message(int queue_id,
                            const Subscription *subscriptions,
                            const ParsedCommand *command)
{
    char output[MAX_MESSAGE_LENGTH];
    int written;

    written = snprintf(output, sizeof(output), "%s\n%s",
                       command->topic, command->payload);
    if (written < 0 || (size_t)written >= sizeof(output)) {
        fprintf(stderr, "Брокер: пересылаемое сообщение слишком длинное.\n");
        return;
    }

    while (subscriptions != NULL) {
        if (strcmp(subscriptions->topic, command->topic) == 0) {
            if (send_queue_text(queue_id, (long)subscriptions->pid,
                                output) == -1) {
                fprintf(stderr,
                        "Брокер: не удалось отправить сообщение PID %ld.\n",
                        (long)subscriptions->pid);
            }
        }
        subscriptions = subscriptions->next;
    }
}

static void process_command(int queue_id,
                            Subscription **subscriptions,
                            Publisher **publishers,
                            const char *text)
{
    ParsedCommand command;

    if (parse_command(text, &command) == -1) {
        fprintf(stderr, "Брокер: получено некорректное сообщение.\n");
        return;
    }

    switch (command.type) {
    case COMMAND_SUBSCRIBE:
        if (add_subscription(subscriptions,
                             command.pid, command.topic) == 0) {
            printf("Брокер: PID %ld подписан на '%s'.\n",
                   (long)command.pid, command.topic);
            fflush(stdout);
        }
        break;

    case COMMAND_UNSUBSCRIBE:
        remove_subscription(subscriptions,
                            command.pid, command.topic);
        printf("Брокер: PID %ld отписан от '%s'.\n",
               (long)command.pid, command.topic);
        fflush(stdout);
        break;

    case COMMAND_SEND:
        if (add_publisher(publishers, command.pid) == 0) {
            printf("Брокер: сообщение от PID %ld, тема '%s'.\n",
                   (long)command.pid, command.topic);
            fflush(stdout);
        }
        forward_message(queue_id, *subscriptions, &command);
        break;

    default:
        break;
    }
}

static void notify_process(pid_t pid)
{
    if (kill(pid, SIGINT) == -1 && errno != ESRCH) {
        perror("kill SIGINT");
    }
}

static int pid_was_notified(const pid_t *pids, size_t count, pid_t pid)
{
    size_t index;

    for (index = 0; index < count; ++index) {
        if (pids[index] == pid) {
            return 0;
        }
    }

    return 1;
}

static void notify_all(const Subscription *subscriptions,
                       const Publisher *publishers)
{
    pid_t *notified = NULL;
    size_t count = 0;
    size_t capacity = 0;

    while (subscriptions != NULL) {
        if (pid_was_notified(notified, count, subscriptions->pid)) {
            if (count == capacity) {
                size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
                pid_t *new_items = realloc(notified,
                                           new_capacity * sizeof(*new_items));
                if (new_items == NULL) {
                    perror("realloc");
                    break;
                }
                notified = new_items;
                capacity = new_capacity;
            }

            notified[count++] = subscriptions->pid;
            notify_process(subscriptions->pid);
        }
        subscriptions = subscriptions->next;
    }

    while (publishers != NULL) {
        if (pid_was_notified(notified, count, publishers->pid)) {
            if (count == capacity) {
                size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
                pid_t *new_items = realloc(notified,
                                           new_capacity * sizeof(*new_items));
                if (new_items == NULL) {
                    perror("realloc");
                    break;
                }
                notified = new_items;
                capacity = new_capacity;
            }

            notified[count++] = publishers->pid;
            notify_process(publishers->pid);
        }
        publishers = publishers->next;
    }

    free(notified);
}

static void drain_queue(int queue_id)
{
    QueueMessage message;
    struct timespec pause_time = {0, 100000000L};
    int empty_checks = 0;
    int checks;

    for (checks = 0; checks < 30 && empty_checks < 3; ++checks) {
        ssize_t received = msgrcv(queue_id, &message,
                                  sizeof(message.text), 0,
                                  IPC_NOWAIT | MSG_NOERROR);

        if (received >= 0) {
            empty_checks = 0;
            continue;
        }

        if (errno == ENOMSG) {
            ++empty_checks;
            nanosleep(&pause_time, NULL);
            continue;
        }

        if (errno == EIDRM || errno == EINVAL) {
            return;
        }

        if (errno != EINTR) {
            perror("msgrcv при очистке");
        }
    }
}

static void free_subscriptions(Subscription *head)
{
    while (head != NULL) {
        Subscription *next = head->next;
        free(head);
        head = next;
    }
}

static void free_publishers(Publisher *head)
{
    while (head != NULL) {
        Publisher *next = head->next;
        free(head);
        head = next;
    }
}

int run_broker(void)
{
    int queue_id;
    Subscription *subscriptions = NULL;
    Publisher *publishers = NULL;
    struct timespec pause_time = {0, 100000000L};

    queue_id = create_broker_queue();
    if (queue_id == -1) {
        if (errno == EEXIST) {
            fprintf(stderr, "Брокер уже запущен: очередь сообщений существует.\n");
        } else {
            perror("msgget");
        }
        return EXIT_FAILURE;
    }

    if (install_signal_handlers() == -1) {
        remove_queue(queue_id);
        return EXIT_FAILURE;
    }

    printf("Брокер запущен. PID: %ld, ID очереди: %d.\n",
           (long)getpid(), queue_id);
    printf("Для завершения нажмите Ctrl+C.\n");
    fflush(stdout);

    while (!stop_requested) {
        QueueMessage message;
        ssize_t received;

        received = msgrcv(queue_id, &message, sizeof(message.text),
                          BROKER_MESSAGE_TYPE,
                          IPC_NOWAIT | MSG_NOERROR);

        if (received >= 0) {
            if ((size_t)received >= sizeof(message.text)) {
                message.text[sizeof(message.text) - 1] = '\0';
            } else {
                message.text[received] = '\0';
            }
            process_command(queue_id, &subscriptions,
                            &publishers, message.text);
            continue;
        }

        if (errno == ENOMSG || errno == EINTR) {
            nanosleep(&pause_time, NULL);
            continue;
        }

        if (errno == EIDRM || errno == EINVAL) {
            fprintf(stderr, "Брокер: очередь сообщений удалена.\n");
            break;
        }

        perror("msgrcv");
        break;
    }

    printf("Брокер завершает работу и отправляет SIGINT клиентам.\n");
    fflush(stdout);

    notify_all(subscriptions, publishers);
    drain_queue(queue_id);
    remove_queue(queue_id);

    free_subscriptions(subscriptions);
    free_publishers(publishers);

    printf("Брокер завершён, очередь удалена.\n");
    return EXIT_SUCCESS;
}
