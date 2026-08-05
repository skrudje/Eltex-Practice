#ifndef TRANSFER_H
#define TRANSFER_H

#define _POSIX_C_SOURCE 200809L

#include <stdint.h>

#define BUFFER_SIZE 4096
#define MAX_FILE_NAME 4096U
#define READY_BYTE 'R'

#define MESSAGE_FILE 1U
#define MESSAGE_FINISH 2U

typedef struct {
    uint32_t type;
    uint32_t name_length;
    uint64_t file_size;
} FileHeader;

int parent_send_files(int data_fd,
                      int ready_fd,
                      char *file_names[],
                      int file_count);

int receive_files(int data_fd, int ready_fd);

#endif
