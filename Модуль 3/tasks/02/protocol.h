#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <sys/types.h>

#define QUEUE_KEY_FILE "/tmp/eltex_module3_pubsub.key"
#define QUEUE_PROJECT_ID 'M'

#define BROKER_MESSAGE_TYPE 1L
#define MAX_TOPIC_LENGTH 64
#define MAX_PAYLOAD_LENGTH 768
#define MAX_MESSAGE_LENGTH 1024

typedef struct {
    long type;
    char text[MAX_MESSAGE_LENGTH];
} QueueMessage;

typedef enum {
    COMMAND_INVALID = 0,
    COMMAND_SUBSCRIBE,
    COMMAND_UNSUBSCRIBE,
    COMMAND_SEND
} CommandType;

typedef struct {
    CommandType type;
    pid_t pid;
    char topic[MAX_TOPIC_LENGTH];
    char payload[MAX_PAYLOAD_LENGTH];
} ParsedCommand;

int build_subscription_text(char *buffer, size_t size,
                            const char *command, pid_t pid,
                            const char *topic);
int build_publish_text(char *buffer, size_t size, pid_t pid,
                       const char *topic, const char *payload);
int parse_command(const char *text, ParsedCommand *command);

#endif
