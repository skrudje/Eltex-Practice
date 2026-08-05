#define _POSIX_C_SOURCE 200809L

#include "transfer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Записывает весь буфер, даже если write() записал только его часть. */
static int write_all(int fd, const void *buffer, size_t size) {
    const unsigned char *data = buffer;
    size_t written_total = 0;

    while (written_total < size) {
        ssize_t written = write(fd,
                                data + written_total,
                                size - written_total);

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        written_total += (size_t)written;
    }

    return 0;
}

/*
 * Читает ровно size байтов.
 * Возвращает 1 при успехе, 0 при EOF до начала чтения и -1 при ошибке.
 */
static int read_all(int fd, void *buffer, size_t size) {
    unsigned char *data = buffer;
    size_t read_total = 0;

    while (read_total < size) {
        ssize_t received = read(fd,
                                data + read_total,
                                size - read_total);

        if (received == 0) {
            return read_total == 0 ? 0 : -1;
        }

        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        read_total += (size_t)received;
    }

    return 1;
}

static char *make_copy_name(const char *source_name) {
    const char suffix[] = ".copy";
    size_t result_size = strlen(source_name) + sizeof(suffix);
    char *result = malloc(result_size);

    if (result == NULL) {
        return NULL;
    }

    snprintf(result, result_size, "%s%s", source_name, suffix);
    return result;
}

static int send_one_file(int channel_fd, const char *file_name) {
    int input_fd = open(file_name, O_RDONLY);

    if (input_fd == -1) {
        fprintf(stderr,
                "Не удалось открыть '%s': %s\n",
                file_name,
                strerror(errno));
        return 0;
    }

    struct stat file_info;
    if (fstat(input_fd, &file_info) == -1) {
        fprintf(stderr,
                "Не удалось получить размер '%s': %s\n",
                file_name,
                strerror(errno));
        close(input_fd);
        return 0;
    }

    if (!S_ISREG(file_info.st_mode)) {
        fprintf(stderr, "'%s' не является обычным файлом.\n", file_name);
        close(input_fd);
        return 0;
    }

    size_t name_length = strlen(file_name) + 1;
    if (name_length > MAX_FILE_NAME) {
        fprintf(stderr, "Слишком длинное имя файла: '%s'.\n", file_name);
        close(input_fd);
        return 0;
    }

    FileHeader header = {
        .type = MESSAGE_FILE,
        .name_length = (uint32_t)name_length,
        .file_size = (uint64_t)file_info.st_size
    };

    if (write_all(channel_fd, &header, sizeof(header)) == -1 ||
        write_all(channel_fd, file_name, name_length) == -1) {
        fprintf(stderr,
                "Ошибка передачи заголовка файла '%s': %s\n",
                file_name,
                strerror(errno));
        close(input_fd);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(input_fd, buffer, sizeof(buffer))) > 0) {
        if (write_all(channel_fd, buffer, (size_t)bytes_read) == -1) {
            fprintf(stderr,
                    "Ошибка передачи файла '%s': %s\n",
                    file_name,
                    strerror(errno));
            close(input_fd);
            return -1;
        }
    }

    if (bytes_read == -1) {
        fprintf(stderr,
                "Ошибка чтения файла '%s': %s\n",
                file_name,
                strerror(errno));
        close(input_fd);
        return -1;
    }

    close(input_fd);
    printf("Родитель: файл '%s' передан.\n", file_name);
    return 1;
}

static int receive_one_file(int data_fd, const FileHeader *header) {
    char *source_name = malloc(header->name_length);

    if (source_name == NULL) {
        fprintf(stderr, "Недостаточно памяти для имени файла.\n");
        return EXIT_FAILURE;
    }

    if (read_all(data_fd, source_name, header->name_length) != 1) {
        fprintf(stderr, "Ошибка чтения имени файла.\n");
        free(source_name);
        return EXIT_FAILURE;
    }

    source_name[header->name_length - 1] = '\0';

    char *copy_name = make_copy_name(source_name);
    if (copy_name == NULL) {
        fprintf(stderr, "Недостаточно памяти для имени копии.\n");
        free(source_name);
        return EXIT_FAILURE;
    }

    int output_fd = open(copy_name,
                         O_WRONLY | O_CREAT | O_TRUNC,
                         0644);

    if (output_fd == -1) {
        fprintf(stderr,
                "Не удалось создать '%s': %s\n",
                copy_name,
                strerror(errno));
    }

    uint64_t remaining = header->file_size;
    char buffer[BUFFER_SIZE];
    int result = EXIT_SUCCESS;

    while (remaining > 0) {
        size_t part_size = remaining < sizeof(buffer)
                               ? (size_t)remaining
                               : sizeof(buffer);

        if (read_all(data_fd, buffer, part_size) != 1) {
            fprintf(stderr,
                    "Ошибка получения содержимого '%s'.\n",
                    source_name);
            result = EXIT_FAILURE;
            break;
        }

        /* Канал читается полностью, даже если файл создать не удалось. */
        if (output_fd != -1 &&
            write_all(output_fd, buffer, part_size) == -1) {
            fprintf(stderr,
                    "Ошибка записи в '%s': %s\n",
                    copy_name,
                    strerror(errno));
            close(output_fd);
            output_fd = -1;
        }

        remaining -= part_size;
    }

    if (output_fd != -1) {
        close(output_fd);
    }

    if (result == EXIT_SUCCESS) {
        printf("Дочерний процесс: создан файл '%s'.\n", copy_name);
    }

    free(copy_name);
    free(source_name);
    return result;
}

int receive_files(int data_fd, int ready_fd) {
    char ready = READY_BYTE;

    if (write_all(ready_fd, &ready, sizeof(ready)) == -1) {
        fprintf(stderr,
                "Дочерний процесс не смог отправить READY: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    while (1) {
        FileHeader header;
        int header_result = read_all(data_fd, &header, sizeof(header));

        if (header_result == 0) {
            fprintf(stderr, "Канал закрыт без сообщения о завершении.\n");
            return EXIT_FAILURE;
        }

        if (header_result == -1) {
            fprintf(stderr,
                    "Ошибка чтения заголовка: %s\n",
                    strerror(errno));
            return EXIT_FAILURE;
        }

        if (header.type == MESSAGE_FINISH) {
            printf("Дочерний процесс: получена команда завершения.\n");
            return EXIT_SUCCESS;
        }

        if (header.type != MESSAGE_FILE ||
            header.name_length == 0 ||
            header.name_length > MAX_FILE_NAME) {
            fprintf(stderr, "Получен некорректный заголовок.\n");
            return EXIT_FAILURE;
        }

        if (receive_one_file(data_fd, &header) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }
}

int parent_send_files(int data_fd,
                      int ready_fd,
                      char *file_names[],
                      int file_count) {
    char ready;

    if (read_all(ready_fd, &ready, sizeof(ready)) != 1 ||
        ready != READY_BYTE) {
        fprintf(stderr, "Родитель не получил сообщение READY.\n");
        return EXIT_FAILURE;
    }

    printf("Родитель: дочерний процесс готов к приему.\n");

    int result = EXIT_SUCCESS;

    for (int i = 0; i < file_count; i++) {
        if (send_one_file(data_fd, file_names[i]) == -1) {
            result = EXIT_FAILURE;
            break;
        }
    }

    FileHeader finish_header = {
        .type = MESSAGE_FINISH,
        .name_length = 0,
        .file_size = 0
    };

    if (write_all(data_fd, &finish_header, sizeof(finish_header)) == -1) {
        fprintf(stderr,
                "Не удалось отправить команду завершения: %s\n",
                strerror(errno));
        result = EXIT_FAILURE;
    }

    return result;
}
