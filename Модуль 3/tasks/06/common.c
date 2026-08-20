#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void trim_newline(char *text)
{
    size_t len;

    if (text == NULL) {
        return;
    }

    len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        len--;
    }
}

int parse_port(const char *text, uint16_t *port)
{
    char *end = NULL;
    long value;

    if (text == NULL || port == NULL) {
        return -1;
    }

    value = strtol(text, &end, 10);
    if (*text == '\0' || end == NULL || *end != '\0') {
        return -1;
    }

    if (value < 1024 || value > 65535) {
        return -1;
    }

    *port = (uint16_t)value;
    return 0;
}

uint32_t make_sender_id(void)
{
    uint32_t pid_part = (uint32_t)getpid();
    uint32_t time_part = (uint32_t)time(NULL);

    return (pid_part << 16) ^ time_part ^ (pid_part * 2654435761u);
}

void make_default_name(char *buffer, size_t size)
{
    const char *user;

    if (buffer == NULL || size == 0) {
        return;
    }

    user = getenv("USER");
    if (user == NULL || *user == '\0') {
        user = "user";
    }

    snprintf(buffer, size, "%s-%ld", user, (long)getpid());
}
