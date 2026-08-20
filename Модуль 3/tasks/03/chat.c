#define _POSIX_C_SOURCE 200809L

#include "chat.h"
#include "protocol.h"
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t signal_stop_requested = 0;
static atomic_bool chat_stop_requested = false;
static atomic_bool peer_finished = false;

typedef struct {
    mqd_t receive_queue;
} ReceiverArguments;

static void signal_handler(int signal_number)
{
    (void)signal_number;
    signal_stop_requested = 1;
}

static int install_signal_handler(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    return sigaction(SIGINT, &action, NULL);
}

static int send_message(mqd_t queue,
                        const char *text,
                        unsigned priority)
{
    size_t length = strlen(text) + 1;

    if (length > CHAT_MESSAGE_SIZE) {
        errno = EMSGSIZE;
        return -1;
    }
    return mq_send(queue, text, length, priority);
}

static int send_exit_message(mqd_t queue)
{
    if (send_message(queue, CHAT_EXIT_TEXT, CHAT_EXIT_PRIORITY) == -1) {
        if (errno == EBADF || errno == EINVAL || errno == ENOENT) {
            return 0;
        }
        return -1;
    }
    return 0;
}

static void make_receive_deadline(struct timespec *deadline)
{
    clock_gettime(CLOCK_REALTIME, deadline);
    deadline->tv_sec += 1;
}

static void *receiver_main(void *argument)
{
    ReceiverArguments *receiver_arguments = argument;
    char message[CHAT_MESSAGE_SIZE];

    while (!atomic_load(&chat_stop_requested)) {
        struct timespec deadline;
        unsigned priority = 0;
        ssize_t received;

        make_receive_deadline(&deadline);
        received = mq_timedreceive(receiver_arguments->receive_queue,
                                   message,
                                   sizeof(message),
                                   &priority,
                                   &deadline);

        if (received == -1) {
            if (errno == ETIMEDOUT || errno == EINTR) {
                continue;
            }

            if (errno == EBADF || errno == EINVAL) {
                break;
            }

            perror("mq_timedreceive");
            atomic_store(&chat_stop_requested, true);
            break;
        }

        if ((size_t)received >= sizeof(message)) {
            message[sizeof(message) - 1] = '\0';
        } else {
            message[received] = '\0';
        }

        if (priority == CHAT_EXIT_PRIORITY ||
            strcmp(message, CHAT_EXIT_TEXT) == 0) {
            printf("\nСобеседник завершил чат.\n");
            fflush(stdout);
            atomic_store(&peer_finished, true);
            atomic_store(&chat_stop_requested, true);
            break;
        }

        printf("\nСобеседник: %s\nВы: ", message);
        fflush(stdout);
    }
    return NULL;
}

static int input_loop(mqd_t send_queue)
{
    char message[CHAT_MESSAGE_SIZE];
    struct pollfd input = {
        .fd = STDIN_FILENO,
        .events = POLLIN,
        .revents = 0
    };

    printf("Введите сообщение. Для выхода: %s или Ctrl+C.\n", CHAT_EXIT_COMMAND);
    printf("Вы: ");
    fflush(stdout);

    while (!atomic_load(&chat_stop_requested)) {
        int poll_result;

        if (signal_stop_requested) {
            atomic_store(&chat_stop_requested, true);
            break;
        }

        input.revents = 0;
        poll_result = poll(&input, 1, 200);
        if (poll_result == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("poll");
            return -1;
        }

        if (poll_result == 0) {
            continue;
        }

        if ((input.revents & (POLLHUP | POLLERR | POLLNVAL)) != 0) {
            atomic_store(&chat_stop_requested, true);
            break;
        }

        if ((input.revents & POLLIN) == 0) {
            continue;
        }

        if (fgets(message, sizeof(message), stdin) == NULL) {
            atomic_store(&chat_stop_requested, true);
            break;
        }

        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, CHAT_EXIT_COMMAND) == 0) {
            atomic_store(&chat_stop_requested, true);
            break;
        }

        if (message[0] == '\0') {
            printf("Вы: ");
            fflush(stdout);
            continue;
        }

        if (send_message(send_queue, message, CHAT_NORMAL_PRIORITY) == -1) {
            perror("mq_send");
            return -1;
        }

        printf("Вы: ");
        fflush(stdout);
    }
    return 0;
}

int chat_run(ChatQueues *queues)
{
    ReceiverArguments receiver_arguments;
    pthread_t receiver_thread;
    int result = 0;

    if (queues == NULL) {
        errno = EINVAL;
        return -1;
    }

    signal_stop_requested = 0;
    atomic_store(&chat_stop_requested, false);
    atomic_store(&peer_finished, false);

    if (install_signal_handler() == -1) {
        perror("sigaction");
        return -1;
    }

    receiver_arguments.receive_queue = queues->receive_queue;

    if (pthread_create(&receiver_thread,
                       NULL,
                       receiver_main,
                       &receiver_arguments) != 0) {
        fprintf(stderr, "Не удалось создать поток приема сообщений.\n");
        return -1;
    }

    if (input_loop(queues->send_queue) == -1) {
        result = -1;
        atomic_store(&chat_stop_requested, true);
    }

    if (!atomic_load(&peer_finished)) {
        if (send_exit_message(queues->send_queue) == -1) {
            perror("mq_send завершения");
            result = -1;
        }
    }

    atomic_store(&chat_stop_requested, true);
    pthread_join(receiver_thread, NULL);
    return result;
}
