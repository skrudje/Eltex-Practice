#ifndef NETWORK_H
#define NETWORK_H

#include "protocol.h"

#include <netinet/in.h>
#include <stdint.h>
#include <sys/types.h>

int create_chat_socket(uint16_t port);
int make_destination(const char *broadcast_ip,
                     uint16_t port,
                     struct sockaddr_in *destination);
ssize_t send_chat_packet(int sockfd,
                         const struct sockaddr_in *destination,
                         const ChatPacket *packet);
ssize_t receive_chat_packet(int sockfd,
                            ChatPacket *packet,
                            struct sockaddr_in *sender);

#endif
