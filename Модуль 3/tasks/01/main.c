#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MESSAGE_FILE 1U
#define MESSAGE_FINISH 2U
#define READY_BYTE 'R'

typedef struct {
    uint32_t type;
    uint32_t name_length;
    uint64_t file_size;
} FileHeader;

static void print_usage(const char *program_name) {
    fprintf(stderr,
            "Использование:\n"
            "  %s файл1 [файл2 ...]\n"
            "  %s -p имя_канала файл1 [файл2 ...]\n",
            program_name,
            program_name);
}

/* Записывает весь буфер, даже если write() записал только его часть. */
static int write_all(int fd, const void *buffer, size_t size) {
    const unsigned char *data = buffer;
    size_t written_total = 0;

    while (written_total < size) {
        ssize_t written = write(fd, data + written_total, size - written_total);

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
 * Возвращает 1 при успехе, 0 при обычном EOF до начала чтения и -1 при ошибке.
 */
static int read_all(int fd, void *buffer, size_t size) {
    unsigned char *data = buffer;
    size_t read_total = 0;

    while (read_total < size) {
        ssize_t received = read(fd, data + read_total, size - read_total);

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

static int send_file(int channel_fd, const char *file_name) {
    int input_fd = open(file_name, O_RDONLY);

    if (input_fd == -1) {
        fprintf(stderr, "Не удалось открыть '%s': %s\n",
                file_name, strerror(errno));
        return 0;
    }

    struct stat file_info;
    if (fstat(input_fd, &file_info) == -1) {
        fprintf(stderr, "Не удалось получить размер '%s': %s\n",
                file_name, strerror(errno));
        close(input_fd);
        return 0;
    }

    if (!S_ISREG(file_info.st_mode)) {
        fprintf(stderr, "'%s' не является обычным файлом.\n", file_name);
        close(input_fd);
        return 0;
    }

    size_t name_length = strlen(file_name) + 1;
    if (name_length > UINT32_MAX) {
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
        fprintf(stderr, "Ошибка передачи заголовка файла '%s': %s\n",
                file_name, strerror(errno));
        close(input_fd);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(input_fd, buffer, sizeof(buffer))) > 0) {
        if (write_all(channel_fd, buffer, (size_t)bytes_read) == -1) {
            fprintf(stderr, "Ошибка передачи файла '%s': %s\n",
                    file_name, strerror(errno));
            close(input_fd);
            return -1;
        }
    }

    if (bytes_read == -1) {
        fprintf(stderr, "Ошибка чтения файла '%s': %s\n",
                file_name, strerror(errno));
        close(input_fd);
        return -1;
    }

    close(input_fd);
    printf("Родитель: файл '%s' передан.\n", file_name);
    return 1;
}

static int receive_files(int data_fd, int ready_fd) {
    char ready = READY_BYTE;

    if (write_all(ready_fd, &ready, sizeof(ready)) == -1) {
        fprintf(stderr, "Дочерний процесс не смог отправить READY: %s\n",
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
            fprintf(stderr, "Ошибка чтения заголовка: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        if (header.type == MESSAGE_FINISH) {
            printf("Дочерний процесс: получена команда завершения.\n");
            return EXIT_SUCCESS;
        }

        if (header.type != MESSAGE_FILE || header.name_length == 0) {
            fprintf(stderr, "Получен некорректный заголовок.\n");
            return EXIT_FAILURE;
        }

        char *source_name = malloc(header.name_length);
        if (source_name == NULL) {
            fprintf(stderr, "Недостаточно памяти для имени файла.\n");
            return EXIT_FAILURE;
        }

        if (read_all(data_fd, source_name, header.name_length) != 1) {
            fprintf(stderr, "Ошибка чтения имени файла.\n");
            free(source_name);
            return EXIT_FAILURE;
        }

        source_name[header.name_length - 1] = '\0';
        char *copy_name = make_copy_name(source_name);
        if (copy_name == NULL) {
            fprintf(stderr, "Недостаточно памяти для имени копии.\n");
            free(source_name);
            return EXIT_FAILURE;
        }

        int output_fd = open(copy_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            fprintf(stderr, "Не удалось создать '%s': %s\n",
                    copy_name, strerror(errno));
        }

        uint64_t remaining = header.file_size;
        char buffer[BUFFER_SIZE];
        int receive_error = 0;

        while (remaining > 0) {
            size_t part_size = remaining < sizeof(buffer)
                                   ? (size_t)remaining
                                   : sizeof(buffer);

            if (read_all(data_fd, buffer, part_size) != 1) {
                fprintf(stderr, "Ошибка получения содержимого '%s'.\n",
                        source_name);
                receive_error = 1;
                break;
            }

            if (output_fd != -1 && write_all(output_fd, buffer, part_size) == -1) {
                fprintf(stderr, "Ошибка записи в '%s': %s\n",
                        copy_name, strerror(errno));
                close(output_fd);
                output_fd = -1;
            }

            remaining -= part_size;
        }

        if (output_fd != -1) {
            close(output_fd);
        }

        if (!receive_error) {
            printf("Дочерний процесс: создан файл '%s'.\n", copy_name);
        }

        free(copy_name);
        free(source_name);

        if (receive_error) {
            return EXIT_FAILURE;
        }
    }
}

static int run_unnamed_pipe_mode(char *file_names[], int file_count) {
    int parent_to_child[2];
    int child_to_parent[2];

    if (pipe(parent_to_child) == -1 || pipe(child_to_parent) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        close(parent_to_child[0]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        close(child_to_parent[1]);
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        int child_result = receive_files(parent_to_child[0], child_to_parent[1]);

        close(parent_to_child[0]);
        close(child_to_parent[1]);
        _exit(child_result);
    }

    close(parent_to_child[0]);
    close(child_to_parent[1]);

    char ready;
    if (read_all(child_to_parent[0], &ready, sizeof(ready)) != 1 ||
        ready != READY_BYTE) {
        fprintf(stderr, "Родитель не получил сообщение READY.\n");
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        waitpid(child_pid, NULL, 0);
        return EXIT_FAILURE;
    }

    printf("Родитель: дочерний процесс готов к приему.\n");

    int parent_error = 0;
    for (int i = 0; i < file_count; i++) {
        if (send_file(parent_to_child[1], file_names[i]) == -1) {
            parent_error = 1;
            break;
        }
    }

    FileHeader finish_header = {
        .type = MESSAGE_FINISH,
        .name_length = 0,
        .file_size = 0
    };

    if (write_all(parent_to_child[1], &finish_header, sizeof(finish_header)) == -1) {
        fprintf(stderr, "Не удалось отправить команду завершения: %s\n",
                strerror(errno));
        parent_error = 1;
    }

    close(parent_to_child[1]);
    close(child_to_parent[0]);

    int child_status;
    if (waitpid(child_pid, &child_status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != EXIT_SUCCESS) {
        fprintf(stderr, "Дочерний процесс завершился с ошибкой.\n");
        return EXIT_FAILURE;
    }

    return parent_error ? EXIT_FAILURE : EXIT_SUCCESS;
}

static int run_named_pipe_mode(const char *channel_name,
                               char *file_names[],
                               int file_count) {
    (void)file_names;
    (void)file_count;

    fprintf(stderr,
            "Режим именованного канала '-p %s' пока оставлен как TODO.\n",
            channel_name);
    return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
    const char *channel_name = NULL;
    int option;

    while ((option = getopt(argc, argv, "p:")) != -1) {
        switch (option) {
            case 'p':
                channel_name = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Не указаны файлы для копирования.\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    char **file_names = &argv[optind];
    int file_count = argc - optind;

    if (channel_name != NULL) {
        return run_named_pipe_mode(channel_name, file_names, file_count);
    }

    return run_unnamed_pipe_mode(file_names, file_count);
}
