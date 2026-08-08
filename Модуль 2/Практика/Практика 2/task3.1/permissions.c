#include "permissions.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int parse_numeric(const char *text, mode_t *mode)
{
    size_t length = strlen(text);
    size_t start = 0;
    mode_t result = 0;

    if (length == 4 && text[0] == '0') {
        start = 1;
    } else if (length != 3) {
        return -1;
    }
    for (size_t i = start; i < length; ++i) {
        if (text[i] < '0' || text[i] > '7') return -1;
        result = (result << 3) | (mode_t)(text[i] - '0');
    }

    *mode = result & 0777;
    return 0;
}

static int parse_symbolic(const char *text, mode_t *mode)
{
    const char expected[3] = {'r', 'w', 'x'};
    mode_t result = 0;

    if (strlen(text) != 9) return -1;

    for (int i = 0; i < 9; ++i) {
        if (text[i] == '-') continue;
        if (text[i] != expected[i % 3]) return -1;
        result |= (mode_t)1 << (8 - i);
    }

    *mode = result;
    return 0;
}

int permissions_parse(const char *text, mode_t *mode)
{
    if (text == NULL || mode == NULL || *text == '\0') return -1;
    if (isdigit((unsigned char)text[0])) return parse_numeric(text, mode);
    return parse_symbolic(text, mode);
}

int permissions_from_file(const char *path, mode_t *mode)
{
    struct stat info;
    if (path == NULL || mode == NULL) return -1;
    if (stat(path, &info) == -1) return -1;
    *mode = info.st_mode & 0777;
    return 0;
}

void permissions_to_symbolic(mode_t mode, char out[10])
{
    const char symbols[3] = {'r', 'w', 'x'};
    mode &= 0777;

    for (int i = 0; i < 9; ++i) {
        mode_t bit = (mode_t)1 << (8 - i);
        out[i] = (mode & bit) ? symbols[i % 3] : '-';
    }
    out[9] = '\0';
}

void permissions_to_binary(mode_t mode, char out[10])
{
    mode &= 0777;
    for (int i = 0; i < 9; ++i) {
        mode_t bit = (mode_t)1 << (8 - i);
        out[i] = (mode & bit) ? '1' : '0';
    }
    out[9] = '\0';
}

void permissions_to_octal(mode_t mode, char out[4])
{
    snprintf(out, 4, "%03o", (unsigned int)(mode & 0777));
}

static mode_t class_mask(char who)
{
    switch (who) {
        case 'u': return 0700;
        case 'g': return 0070;
        case 'o': return 0007;
        case 'a': return 0777;
        default: return 0;
    }
}

static mode_t permission_bits(char who, char permission)
{
    if (who == 'u') {
        if (permission == 'r') return 0400;
        if (permission == 'w') return 0200;
        if (permission == 'x') return 0100;
    } else if (who == 'g') {
        if (permission == 'r') return 0040;
        if (permission == 'w') return 0020;
        if (permission == 'x') return 0010;
    } else if (who == 'o') {
        if (permission == 'r') return 0004;
        if (permission == 'w') return 0002;
        if (permission == 'x') return 0001;
    }
    return 0;
}

static int apply_clause(mode_t *mode, const char *clause)
{
    size_t i = 0;
    mode_t target_mask = 0;
    mode_t bits = 0;
    int has_who = 0;

    while (clause[i] == 'u' || clause[i] == 'g' ||
           clause[i] == 'o' || clause[i] == 'a') {
        target_mask |= class_mask(clause[i]);
        has_who = 1;
        ++i;
    }

    if (!has_who) return -1;

    char operation = clause[i];
    if (operation != '+' && operation != '-' && operation != '=') return -1;
    ++i;

    for (; clause[i] != '\0'; ++i) {
        char p = clause[i];
        if (p != 'r' && p != 'w' && p != 'x') return -1;

        if (target_mask & 0700) bits |= permission_bits('u', p);
        if (target_mask & 0070) bits |= permission_bits('g', p);
        if (target_mask & 0007) bits |= permission_bits('o', p);
    }

    if (operation == '+') *mode |= bits;
    else if (operation == '-') *mode &= ~bits;
    else {
        *mode &= ~target_mask;
        *mode |= bits;
    }

    *mode &= 0777;
    return 0;
}

int permissions_modify(mode_t *mode, const char *command)
{
    char copy[256];
    if (mode == NULL || command == NULL || *command == '\0') return -1;
    if (strlen(command) >= sizeof(copy)) return -1;

    strcpy(copy, command);
    mode_t result = *mode & 0777;

    char *clause = strtok(copy, ",");
    while (clause != NULL) {
        if (*clause == '\0' || apply_clause(&result, clause) == -1) return -1;
        clause = strtok(NULL, ",");
    }
    *mode = result;
    return 0;
}
