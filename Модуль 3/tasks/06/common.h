#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>

#define DEFAULT_PORT 51006
#define DEFAULT_BROADCAST "255.255.255.255"

void trim_newline(char *text);
int parse_port(const char *text, uint16_t *port);
uint32_t make_sender_id(void);
void make_default_name(char *buffer, size_t size);

#endif
