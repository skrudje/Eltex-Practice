#ifndef IPV4_H
#define IPV4_H
#include <stdint.h>
int ipv4_parse(const char *text, uint32_t *address);
void ipv4_format(uint32_t address, char out[16]);
uint32_t ipv4_random(void);
int ipv4_mask_is_valid(uint32_t mask);
int ipv4_same_subnet(uint32_t first, uint32_t second, uint32_t mask);
#endif
