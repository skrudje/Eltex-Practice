#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define CHAT_MAGIC 0x43483631u
#define CHAT_VERSION 1u
#define CHAT_NAME_SIZE 32
#define CHAT_TEXT_SIZE 512

typedef enum {
    CHAT_JOIN = 1,
    CHAT_MESSAGE = 2,
    CHAT_LEAVE = 3
} ChatMessageType;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t type;
    uint32_t sender_id;
    char name[CHAT_NAME_SIZE];
    char text[CHAT_TEXT_SIZE];
} ChatPacket;

void packet_build(ChatPacket *packet,
                  ChatMessageType type,
                  uint32_t sender_id,
                  const char *name,
                  const char *text);
int packet_is_valid(const ChatPacket *packet);
ChatMessageType packet_get_type(const ChatPacket *packet);
uint32_t packet_get_sender_id(const ChatPacket *packet);

#endif
