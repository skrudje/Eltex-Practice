#ifndef CHAT_H
#define CHAT_H

#include <stdint.h>

#include "protocol.h"

typedef struct {
    char name[CHAT_NAME_SIZE];
    char broadcast_ip[16];
    uint16_t port;
} ChatConfig;

int run_chat(const ChatConfig *config);

#endif
