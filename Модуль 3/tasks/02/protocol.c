#include "protocol.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int copy_part(char *destination, size_t destination_size,
                     const char *start, size_t length)
{
    if (length == 0 || length >= destination_size) {
        return -1;
    }

    memcpy(destination, start, length);
    destination[length] = '\0';
    return 0;
}

int build_subscription_text(char *buffer, size_t size,
                            const char *command, pid_t pid,
                            const char *topic)
{
    int written;

    written = snprintf(buffer, size, "%s,%ld,%s",
                       command, (long)pid, topic);

    if (written < 0 || (size_t)written >= size) {
        return -1;
    }

    return 0;
}

int build_publish_text(char *buffer, size_t size, pid_t pid,
                       const char *topic, const char *payload)
{
    int written;

    written = snprintf(buffer, size, "send,%ld,%s\n%s",
                       (long)pid, topic, payload);

    if (written < 0 || (size_t)written >= size) {
        return -1;
    }

    return 0;
}

int parse_command(const char *text, ParsedCommand *command)
{
    const char *first_comma;
    const char *second_comma;
    const char *topic_end;
    char command_name[32];
    char pid_text[32];
    char *end_pointer;
    long pid_value;

    if (text == NULL || command == NULL) {
        return -1;
    }

    memset(command, 0, sizeof(*command));

    first_comma = strchr(text, ',');
    if (first_comma == NULL) {
        return -1;
    }

    second_comma = strchr(first_comma + 1, ',');
    if (second_comma == NULL) {
        return -1;
    }

    if (copy_part(command_name, sizeof(command_name),
                  text, (size_t)(first_comma - text)) == -1) {
        return -1;
    }

    if (copy_part(pid_text, sizeof(pid_text), first_comma + 1,
                  (size_t)(second_comma - first_comma - 1)) == -1) {
        return -1;
    }

    errno = 0;
    pid_value = strtol(pid_text, &end_pointer, 10);
    if (errno != 0 || *end_pointer != '\0' || pid_value <= 0) {
        return -1;
    }
    command->pid = (pid_t)pid_value;

    if (strcmp(command_name, "send") == 0) {
        topic_end = strchr(second_comma + 1, '\n');
        if (topic_end == NULL) {
            return -1;
        }

        if (copy_part(command->topic, sizeof(command->topic),
                      second_comma + 1,
                      (size_t)(topic_end - second_comma - 1)) == -1) {
            return -1;
        }

        if (strlen(topic_end + 1) >= sizeof(command->payload)) {
            return -1;
        }

        strcpy(command->payload, topic_end + 1);
        command->type = COMMAND_SEND;
        return 0;
    }

    if (strlen(second_comma + 1) == 0 ||
        strlen(second_comma + 1) >= sizeof(command->topic)) {
        return -1;
    }
    strcpy(command->topic, second_comma + 1);

    if (strcmp(command_name, "subscribe") == 0) {
        command->type = COMMAND_SUBSCRIBE;
        return 0;
    }

    if (strcmp(command_name, "unsubscribe") == 0) {
        command->type = COMMAND_UNSUBSCRIBE;
        return 0;
    }

    return -1;
}
