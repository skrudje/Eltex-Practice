#ifndef PERMISSIONS_H
#define PERMISSIONS_H
#include <sys/types.h>
int permissions_parse(const char *text, mode_t *mode);
int permissions_from_file(const char *path, mode_t *mode);
int permissions_modify(mode_t *mode, const char *command);
void permissions_to_symbolic(mode_t mode, char out[10]);
void permissions_to_binary(mode_t mode, char out[10]);
void permissions_to_octal(mode_t mode, char out[4]);
#endif
