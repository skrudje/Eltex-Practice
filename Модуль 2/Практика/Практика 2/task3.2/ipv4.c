#include "ipv4.h"
#include <stdio.h>
#include <stdlib.h>

int ipv4_parse(const char *text, uint32_t *address)
{
    unsigned int a, b, c, d;
    char extra;

    if (text == NULL || address == NULL) return -1;

    if (sscanf(text, "%u.%u.%u.%u%c", &a, &b, &c, &d, &extra) != 4)
        return -1;

    if (a > 255 || b > 255 || c > 255 || d > 255)
        return -1;

    *address = ((uint32_t)a << 24) |
               ((uint32_t)b << 16) |
               ((uint32_t)c << 8) |
               (uint32_t)d;
    return 0;
}

void ipv4_format(uint32_t address, char out[16])
{
    snprintf(out, 16, "%u.%u.%u.%u",
             (unsigned int)((address >> 24) & 0xFF),
             (unsigned int)((address >> 16) & 0xFF),
             (unsigned int)((address >> 8) & 0xFF),
             (unsigned int)(address & 0xFF));
}

uint32_t ipv4_random(void)
{
    uint32_t address = 0;

    for (int i = 0; i < 4; ++i)
        address = (address << 8) | (uint32_t)(rand() & 0xFF);

    return address;
}

int ipv4_mask_is_valid(uint32_t mask)
{
    uint32_t inverted = ~mask;
    return (inverted & (inverted + 1U)) == 0;
}

int ipv4_same_subnet(uint32_t first, uint32_t second, uint32_t mask)
{
    uint32_t first_network = first & mask;
    uint32_t second_network = second & mask;
    return (first_network ^ second_network) == 0;
}
