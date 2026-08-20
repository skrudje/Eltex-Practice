#define _POSIX_C_SOURCE 200809L
#include "common.h"
#include <errno.h>
#include <stdlib.h>
#include <time.h>

void sleep_milliseconds(long milliseconds)
{
    struct timespec delay;
    if (milliseconds <= 0) return;
    delay.tv_sec = milliseconds / 1000;
    delay.tv_nsec = (milliseconds % 1000) * 1000000L;
    while (nanosleep(&delay, &delay) == -1 && errno == EINTR) {}
}

long get_delay_from_env(const char *name, long default_value)
{
    const char *text = getenv(name);
    char *end;
    long value;
    if (text == NULL || *text == '\0') return default_value;
    value = strtol(text, &end, 10);
    if (*end != '\0' || value < 0) return default_value;
    return value;
}

void find_min_max(const int *values, int count, int *minimum, int *maximum)
{
    int min_value = values[0];
    int max_value = values[0];
    for (int i = 1; i < count; ++i) {
        if (values[i] < min_value) min_value = values[i];
        if (values[i] > max_value) max_value = values[i];
    }
    *minimum = min_value;
    *maximum = max_value;
}
