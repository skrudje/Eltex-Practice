#include "protocol.h"

#include <arpa/inet.h>
#include <string.h>

void packet_build(ChatPacket *packet,
                  ChatMessageType type,
                  uint32_t sender_id,
                  const char *name,
                  const char *text)
{
    memset(packet, 0, sizeof(*packet));

    packet->magic = htonl(CHAT_MAGIC);
    packet->version = htonl(CHAT_VERSION);
    packet->type = htonl((uint32_t)type);
    packet->sender_id = htonl(sender_id);

    if (name != NULL) {
        strncpy(packet->name, name, CHAT_NAME_SIZE - 1);
    }

    if (text != NULL) {
        strncpy(packet->text, text, CHAT_TEXT_SIZE - 1);
    }
}

int packet_is_valid(const ChatPacket *packet)
{
    uint32_t type;

    if (packet == NULL) {
        return 0;
    }

    if (ntohl(packet->magic) != CHAT_MAGIC ||
        ntohl(packet->version) != CHAT_VERSION) {
        return 0;
    }

    type = ntohl(packet->type);
    if (type < CHAT_JOIN || type > CHAT_LEAVE) {
        return 0;
    }

    if (memchr(packet->name, '\0', CHAT_NAME_SIZE) == NULL ||
        memchr(packet->text, '\0', CHAT_TEXT_SIZE) == NULL) {
        return 0;
    }

    return 1;
}

ChatMessageType packet_get_type(const ChatPacket *packet)
{
    return (ChatMessageType)ntohl(packet->type);
}

uint32_t packet_get_sender_id(const ChatPacket *packet)
{
    return ntohl(packet->sender_id);
}
