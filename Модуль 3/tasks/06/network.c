#include "network.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int create_chat_socket(uint16_t port)
{
    int sockfd;
    int enabled = 1;
    struct sockaddr_in local_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                   &enabled, sizeof(enabled)) == -1) {
        perror("setsockopt SO_REUSEADDR");
        close(sockfd);
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
                   &enabled, sizeof(enabled)) == -1) {
        perror("setsockopt SO_BROADCAST");
        close(sockfd);
        return -1;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&local_addr,
             sizeof(local_addr)) == -1) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int make_destination(const char *broadcast_ip,
                     uint16_t port,
                     struct sockaddr_in *destination)
{
    if (broadcast_ip == NULL || destination == NULL) {
        return -1;
    }

    memset(destination, 0, sizeof(*destination));
    destination->sin_family = AF_INET;
    destination->sin_port = htons(port);

    if (inet_pton(AF_INET, broadcast_ip, &destination->sin_addr) != 1) {
        return -1;
    }

    return 0;
}

ssize_t send_chat_packet(int sockfd,
                         const struct sockaddr_in *destination,
                         const ChatPacket *packet)
{
    return sendto(sockfd,
                  packet,
                  sizeof(*packet),
                  0,
                  (const struct sockaddr *)destination,
                  sizeof(*destination));
}

ssize_t receive_chat_packet(int sockfd,
                            ChatPacket *packet,
                            struct sockaddr_in *sender)
{
    socklen_t sender_len = sizeof(*sender);

    return recvfrom(sockfd,
                    packet,
                    sizeof(*packet),
                    0,
                    (struct sockaddr *)sender,
                    &sender_len);
}
