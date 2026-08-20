#include "publisher.h"

#include "ipc.h"
#include "protocol.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int run_publisher(const char *topic)
{
    int queue_id;
    char payload[MAX_PAYLOAD_LENGTH];
    char message_text[MAX_MESSAGE_LENGTH];
    pid_t pid = getpid();

    if (strlen(topic) == 0 || strlen(topic) >= MAX_TOPIC_LENGTH) {
        fprintf(stderr, "Некорректная длина темы.\n");
        return EXIT_FAILURE;
    }

    queue_id = connect_to_broker_queue();
    if (queue_id == -1) {
        return EXIT_FAILURE;
    }

    if (install_signal_handlers() == -1) {
        return EXIT_FAILURE;
    }

    printf("Издатель запущен. PID: %ld, тема: '%s'.\n",
           (long)pid, topic);
    printf("Введите сообщения. Ctrl+D завершает ввод.\n");
    fflush(stdout);

    while (!stop_requested) {
        size_t length;

        errno = 0;
        if (fgets(payload, sizeof(payload), stdin) == NULL) {
            if (stop_requested || feof(stdin)) {
                break;
            }

            if (errno == EINTR) {
                clearerr(stdin);
                continue;
            }

            perror("fgets");
            return EXIT_FAILURE;
        }

        length = strlen(payload);
        if (length > 0 && payload[length - 1] == '\n') {
            payload[length - 1] = '\0';
        }

        if (payload[0] == '\0') {
            continue;
        }

        if (build_publish_text(message_text, sizeof(message_text),
                               pid, topic, payload) == -1) {
            fprintf(stderr, "Сообщение слишком длинное.\n");
            continue;
        }

        if (send_queue_text(queue_id, BROKER_MESSAGE_TYPE,
                            message_text) == -1) {
            return EXIT_FAILURE;
        }
    }

    printf("Издатель завершён.\n");
    return EXIT_SUCCESS;
}
