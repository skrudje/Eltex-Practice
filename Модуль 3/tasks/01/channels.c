#define _POSIX_C_SOURCE 200809L

#include "channels.h"
#include "transfer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int open_retry(const char *path, int flags) {
    int fd;

    do {
        fd = open(path, flags);
    } while (fd == -1 && errno == EINTR);
    return fd;
}

static int wait_for_child(pid_t child_pid) {
    int child_status;

    if (waitpid(child_pid, &child_status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(child_status) ||
        WEXITSTATUS(child_status) != EXIT_SUCCESS) {
        fprintf(stderr, "Дочерний процесс завершился с ошибкой.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static char *make_fifo_name(const char *base_name, const char *suffix) {
    size_t size = strlen(base_name) + strlen(suffix) + 1;
    char *name = malloc(size);

    if (name == NULL) {
        return NULL;
    }

    snprintf(name, size, "%s%s", base_name, suffix);
    return name;
}

static int create_fifo(const char *path) {
    if (mkfifo(path, 0600) == -1) {
        if (errno == EEXIST) {
            fprintf(stderr,
                    "Канал '%s' уже существует. "
                    "Удалите его или выберите другое имя.\n",
                    path);
        } else {
            fprintf(stderr,
                    "Не удалось создать FIFO '%s': %s\n",
                    path,
                    strerror(errno));
        }
        return -1;
    }
    return 0;
}

static void remove_fifos(const char *data_fifo,
                         const char *ready_fifo,
                         int *result) {
    if (unlink(data_fifo) == -1 && errno != ENOENT) {
        fprintf(stderr,
                "Не удалось удалить FIFO '%s': %s\n",
                data_fifo,
                strerror(errno));
        *result = EXIT_FAILURE;
    }

    if (unlink(ready_fifo) == -1 && errno != ENOENT) {
        fprintf(stderr,
                "Не удалось удалить FIFO '%s': %s\n",
                ready_fifo,
                strerror(errno));
        *result = EXIT_FAILURE;
    }
}

int run_unnamed_pipe_mode(char *file_names[], int file_count) {
    int parent_to_child[2];
    int child_to_parent[2];

    if (pipe(parent_to_child) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    if (pipe(child_to_parent) == -1) {
        perror("pipe");
        close(parent_to_child[0]);
        close(parent_to_child[1]);
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

        int child_result = receive_files(parent_to_child[0],
                                         child_to_parent[1]);

        close(parent_to_child[0]);
        close(child_to_parent[1]);
        _exit(child_result);
    }

    close(parent_to_child[0]);
    close(child_to_parent[1]);

    int result = parent_send_files(parent_to_child[1],
                                   child_to_parent[0],
                                   file_names,
                                   file_count);

    close(parent_to_child[1]);
    close(child_to_parent[0]);

    if (wait_for_child(child_pid) != EXIT_SUCCESS) {
        result = EXIT_FAILURE;
    }
    return result;
}

int run_named_pipe_mode(const char *channel_name,
                        char *file_names[],
                        int file_count) {
    char *data_fifo = make_fifo_name(channel_name, ".data");
    char *ready_fifo = make_fifo_name(channel_name, ".ready");

    if (data_fifo == NULL || ready_fifo == NULL) {
        fprintf(stderr, "Недостаточно памяти для имен каналов.\n");
        free(data_fifo);
        free(ready_fifo);
        return EXIT_FAILURE;
    }

    if (create_fifo(data_fifo) == -1) {
        free(data_fifo);
        free(ready_fifo);
        return EXIT_FAILURE;
    }

    if (create_fifo(ready_fifo) == -1) {
        unlink(data_fifo);
        free(data_fifo);
        free(ready_fifo);
        return EXIT_FAILURE;
    }

    printf("Созданы именованные каналы:\n  %s\n  %s\n",
           data_fifo,
           ready_fifo);

    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("fork");
        int result = EXIT_FAILURE;
        remove_fifos(data_fifo, ready_fifo, &result);
        free(data_fifo);
        free(ready_fifo);
        return result;
    }

    if (child_pid == 0) {
        int data_fd = open_retry(data_fifo, O_RDONLY);
        if (data_fd == -1) {
            fprintf(stderr,
                    "Потомок не смог открыть '%s': %s\n",
                    data_fifo,
                    strerror(errno));
            _exit(EXIT_FAILURE);
        }

        int ready_fd = open_retry(ready_fifo, O_WRONLY);
        if (ready_fd == -1) {
            fprintf(stderr,
                    "Потомок не смог открыть '%s': %s\n",
                    ready_fifo,
                    strerror(errno));
            close(data_fd);
            _exit(EXIT_FAILURE);
        }

        int child_result = receive_files(data_fd, ready_fd);

        close(data_fd);
        close(ready_fd);
        _exit(child_result);
    }

    int data_fd = open_retry(data_fifo, O_WRONLY);
    if (data_fd == -1) {
        fprintf(stderr,
                "Родитель не смог открыть '%s': %s\n",
                data_fifo,
                strerror(errno));
        waitpid(child_pid, NULL, 0);
        int result = EXIT_FAILURE;
        remove_fifos(data_fifo, ready_fifo, &result);
        free(data_fifo);
        free(ready_fifo);
        return result;
    }

    int ready_fd = open_retry(ready_fifo, O_RDONLY);
    if (ready_fd == -1) {
        fprintf(stderr,
                "Родитель не смог открыть '%s': %s\n",
                ready_fifo,
                strerror(errno));
        close(data_fd);
        waitpid(child_pid, NULL, 0);
        int result = EXIT_FAILURE;
        remove_fifos(data_fifo, ready_fifo, &result);
        free(data_fifo);
        free(ready_fifo);
        return result;
    }

    int result = parent_send_files(data_fd,
                                   ready_fd,
                                   file_names,
                                   file_count);

    close(data_fd);
    close(ready_fd);

    if (wait_for_child(child_pid) != EXIT_SUCCESS) {
        result = EXIT_FAILURE;
    }

    remove_fifos(data_fifo, ready_fifo, &result);

    free(data_fifo);
    free(ready_fifo);
    return result;
}
