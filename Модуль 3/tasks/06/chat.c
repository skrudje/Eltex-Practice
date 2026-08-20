#include "chat.h"

#include "common.h"
#include "network.h"

#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile sig_atomic_t stop_requested = 0;

static void handle_signal(int signum)
{
    (void)signum;
    stop_requested = 1;
}

static int setup_signals(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) == -1 ||
        sigaction(SIGTERM, &action, NULL) == -1) {
        return -1;
    }

    return 0;
}

static int send_event(int sockfd,
                      const struct sockaddr_in *destination,
                      ChatMessageType type,
                      uint32_t sender_id,
                      const char *name,
                      const char *text)
{
    ChatPacket packet;

    packet_build(&packet, type, sender_id, name, text);

    if (send_chat_packet(sockfd, destination, &packet) != (ssize_t)sizeof(packet)) {
        perror("sendto");
        return -1;
    }

    return 0;
}

static void print_received(const ChatPacket *packet,
                           const struct sockaddr_in *sender)
{
    ChatMessageType type = packet_get_type(packet);
    char ip[INET_ADDRSTRLEN];

    if (inet_ntop(AF_INET, &sender->sin_addr, ip, sizeof(ip)) == NULL) {
        strcpy(ip, "?");
    }

    if (type == CHAT_JOIN) {
        printf("[system] %s подключился (%s:%u)\n",
               packet->name,
               ip,
               (unsigned)ntohs(sender->sin_port));
    } else if (type == CHAT_MESSAGE) {
        printf("[%s] %s\n", packet->name, packet->text);
    } else if (type == CHAT_LEAVE) {
        printf("[system] %s отключился\n", packet->name);
    }

    fflush(stdout);
}

int run_chat(const ChatConfig *config)
{
    int sockfd;
    struct sockaddr_in destination;
    uint32_t sender_id;
    char line[CHAT_TEXT_SIZE];

    if (config == NULL) {
        return 1;
    }

    if (setup_signals() == -1) {
        perror("sigaction");
        return 1;
    }

    stop_requested = 0;

    sockfd = create_chat_socket(config->port);
    if (sockfd == -1) {
        return 1;
    }

    if (make_destination(config->broadcast_ip,
                         config->port,
                         &destination) == -1) {
        fprintf(stderr, "Некорректный broadcast IPv4-адрес: %s\n",
                config->broadcast_ip);
        close(sockfd);
        return 1;
    }

    sender_id = make_sender_id();

    printf("UDP broadcast chat запущен\n");
    printf("Имя: %s\n", config->name);
    printf("Broadcast: %s:%u\n",
           config->broadcast_ip,
           (unsigned)config->port);
    printf("Введите сообщение и нажмите Enter. /quit — выход.\n");
    fflush(stdout);

    if (send_event(sockfd,
                   &destination,
                   CHAT_JOIN,
                   sender_id,
                   config->name,
                   "") == -1) {
        close(sockfd);
        return 1;
    }

    while (!stop_requested) {
        fd_set readfds;
        int maxfd;
        int ready;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        ready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        if (ready == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            break;
        }

        if (FD_ISSET(sockfd, &readfds)) {
            ChatPacket packet;
            struct sockaddr_in sender;
            ssize_t received;

            memset(&packet, 0, sizeof(packet));
            memset(&sender, 0, sizeof(sender));

            received = receive_chat_packet(sockfd, &packet, &sender);
            if (received == (ssize_t)sizeof(packet) &&
                packet_is_valid(&packet) &&
                packet_get_sender_id(&packet) != sender_id) {
                print_received(&packet, &sender);
            }
        }

        if (stop_requested) {
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(line, sizeof(line), stdin) == NULL) {
                break;
            }

            trim_newline(line);

            if (strcmp(line, "/quit") == 0) {
                break;
            }

            if (line[0] == '\0') {
                continue;
            }

            if (send_event(sockfd,
                           &destination,
                           CHAT_MESSAGE,
                           sender_id,
                           config->name,
                           line) == -1) {
                close(sockfd);
                return 1;
            }
        }
    }

    send_event(sockfd,
               &destination,
               CHAT_LEAVE,
               sender_id,
               config->name,
               "");

    printf("Чат завершён.\n");
    fflush(stdout);
    close(sockfd);
    return 0;
}
