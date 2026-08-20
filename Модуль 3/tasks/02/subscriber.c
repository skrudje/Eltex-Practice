#include "subscriber.h"

#include "ipc.h"
#include "protocol.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>

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

static int send_subscription_commands(int queue_id, char *topics[],
                                      int topic_count,
                                      const char *command_name)
{
    int index;
    char text[MAX_MESSAGE_LENGTH];
    pid_t pid = getpid();

    for (index = 0; index < topic_count; ++index) {
        if (build_subscription_text(text, sizeof(text), command_name,
                                    pid, topics[index]) == -1) {
            fprintf(stderr, "Не удалось сформировать сообщение для темы '%s'.\n",
                    topics[index]);
            return -1;
        }

        if (send_queue_text(queue_id, BROKER_MESSAGE_TYPE, text) == -1) {
            return -1;
        }
    }

    return 0;
}

static void print_received_message(const char *text)
{
    const char *separator = strchr(text, '\n');

    if (separator == NULL) {
        printf("Получено: %s\n", text);
    } else {
        printf("[%.*s] %s\n",
               (int)(separator - text), text, separator + 1);
    }
    fflush(stdout);
}

int run_subscriber(char *topics[], int topic_count)
{
    int queue_id;
    int index;
    pid_t pid = getpid();

    for (index = 0; index < topic_count; ++index) {
        if (strlen(topics[index]) == 0 ||
            strlen(topics[index]) >= MAX_TOPIC_LENGTH) {
            fprintf(stderr, "Некорректная тема: '%s'.\n", topics[index]);
            return EXIT_FAILURE;
        }
    }

    queue_id = connect_to_broker_queue();
    if (queue_id == -1) {
        return EXIT_FAILURE;
    }

    if (install_signal_handlers() == -1) {
        return EXIT_FAILURE;
    }

    if (send_subscription_commands(queue_id, topics, topic_count,
                                   "subscribe") == -1) {
        return EXIT_FAILURE;
    }

    printf("Подписчик запущен. PID: %ld. Темы:", (long)pid);
    for (index = 0; index < topic_count; ++index) {
        printf(" %s", topics[index]);
    }
    printf("\nДля завершения нажмите Ctrl+C.\n");
    fflush(stdout);

    while (!stop_requested) {
        QueueMessage message;
        ssize_t received;

        received = msgrcv(queue_id, &message, sizeof(message.text),
                          (long)pid, MSG_NOERROR);

        if (received >= 0) {
            if ((size_t)received >= sizeof(message.text)) {
                message.text[sizeof(message.text) - 1] = '\0';
            } else {
                message.text[received] = '\0';
            }
            print_received_message(message.text);
            continue;
        }

        if (errno == EINTR) {
            continue;
        }

        if (errno == EIDRM || errno == EINVAL) {
            fprintf(stderr, "Подписчик: очередь сообщений недоступна.\n");
            break;
        }

        perror("msgrcv");
        break;
    }

    if (send_subscription_commands(queue_id, topics, topic_count,
                                   "unsubscribe") == -1) {
        if (errno != EIDRM && errno != EINVAL) {
            fprintf(stderr, "Не удалось отправить все уведомления об отписке.\n");
        }
    }

    printf("Подписчик завершён.\n");
    return EXIT_SUCCESS;
}
