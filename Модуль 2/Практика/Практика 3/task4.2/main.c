#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "priority_queue.h"

static void read_text(const char *message, char text[], int size) {
    printf("%s", message);
    fflush(stdout);
    if (fgets(text, size, stdin) != NULL) {
        text[strcspn(text, "\n")] = '\0';
    }
}

static int read_int(const char *message) {
    char line[50];
    int value;
    while (1) {
        read_text(message, line, sizeof(line));
        if (sscanf(line, "%d", &value) == 1) {
            return value;
        }
        printf("Введите целое число.\n");
    }
}

static int read_priority(const char *message) {
    int priority;
    while (1) {
        priority = read_int(message);
        if (priority >= 0 && priority <= 255) {
            return priority;
        }
        printf("Приоритет должен быть от 0 до 255.\n");
    }
}

static void print_message(const Message *message) {
    printf("Получено сообщение: ID=%d, приоритет=%d, текст=\"%s\"\n",
           message->id,
           message->priority,
           message->text);
}

static void generate_messages(PriorityQueue *queue, int *next_id) {
    int amount = read_int("Сколько сообщений создать: ");
    int i;
    if (amount <= 0) {
        printf("Количество должно быть больше нуля.\n");
        return;
    }

    for (i = 0; i < amount; i++) {
        int priority = rand() % 256;
        char text[MESSAGE_TEXT_SIZE];
        snprintf(text, sizeof(text), "Автоматическое сообщение %d", *next_id);
        if (queue_push(queue, *next_id, priority, text)) {
            (*next_id)++;
        }
    }

    printf("Создано сообщений: %d\n", amount);
}

int main(void) {
    PriorityQueue queue;
    Message message;
    int choice;
    int next_id = 1;

    queue_init(&queue);
    srand((unsigned int)time(NULL));

    do {
        printf("\n\n\n----- Очередь с приоритетом -----\n\n");
        printf("1. Добавить сообщение\n");
        printf("2. Извлечь первое сообщение\n");
        printf("3. Извлечь сообщение с указанным приоритетом\n");
        printf("4. Извлечь сообщение с приоритетом не ниже заданного\n");
        printf("5. Показать очередь\n");
        printf("6. Сгенерировать случайные сообщения\n");
        printf("0. Выход\n\n");

        choice = read_int("Выберите действие: ");

        switch (choice) {
            case 1: {
                int priority;
                char text[MESSAGE_TEXT_SIZE];

                priority = read_priority("Приоритет от 0 до 255: ");
                read_text("Текст сообщения: ", text, sizeof(text));

                if (queue_push(&queue, next_id, priority, text)) {
                    printf("Сообщение добавлено. ID=%d\n", next_id);
                    next_id++;
                } else {
                    printf("Не удалось добавить сообщение.\n");
                }
                break;
            }

            case 2:
                if (queue_pop_first(&queue, &message)) {
                    print_message(&message);
                } else {
                    printf("Очередь пуста.\n");
                }
                break;

            case 3: {
                int priority = read_priority("Укажите приоритет: ");

                if (queue_pop_priority(&queue, priority, &message)) {
                    print_message(&message);
                } else {
                    printf("Сообщение с таким приоритетом не найдено.\n");
                }
                break;
            }

            case 4: {
                int min_priority = read_priority("Минимальный приоритет: ");

                if (queue_pop_min_priority(&queue, min_priority, &message)) {
                    print_message(&message);
                } else {
                    printf("Подходящее сообщение не найдено.\n");
                }
                break;
            }

            case 5:
                queue_print(&queue);
                printf("Всего сообщений: %d\n", queue_count(&queue));
                break;

            case 6:
                generate_messages(&queue, &next_id);
                break;

            case 0:
                break;

            default:
                printf("Такого пункта меню нет.\n");
        }
    } while (choice != 0);

    queue_clear(&queue);
    printf("Работа программы завершена.\n");

    return 0;
}
