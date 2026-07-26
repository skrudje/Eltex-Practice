#include <stdio.h>
#include <string.h>
#include "priority_queue.h"

static int passed = 0;
static int failed = 0;

static void check_int(const char *name, int actual, int expected) {
    if (actual == expected) {
        printf("[OK]   %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s: получено %d, ожидалось %d\n",
               name, actual, expected);
        failed++;
    }
}

static void check_text(const char *name, const char *actual, const char *expected) {
    if (strcmp(actual, expected) == 0) {
        printf("[OK]   %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s: получено \"%s\", ожидалось \"%s\"\n",
               name, actual, expected);
        failed++;
    }
}

int main(void) {
    PriorityQueue queue;
    Message message;

    queue_init(&queue);
    check_int("Новая очередь пуста", queue_is_empty(&queue), 1);
    check_int("Начальное количество", queue_count(&queue), 0);

    check_int("Добавление приоритета 10",
              queue_push(&queue, 1, 10, "Низкий"), 1);
    check_int("Добавление приоритета 200",
              queue_push(&queue, 2, 200, "Высокий"), 1);
    check_int("Добавление приоритета 100",
              queue_push(&queue, 3, 100, "Средний"), 1);
    check_int("Количество после добавления", queue_count(&queue), 3);
    check_int("Первым стоит максимальный приоритет",
              queue.head->message.priority, 200);

    check_int("Извлечение первого", queue_pop_first(&queue, &message), 1);
    check_int("Извлечён приоритет 200", message.priority, 200);
    check_text("Текст первого сообщения", message.text, "Высокий");

    check_int("Добавление первого одинакового приоритета",
              queue_push(&queue, 4, 150, "Первое 150"), 1);
    check_int("Добавление второго одинакового приоритета",
              queue_push(&queue, 5, 150, "Второе 150"), 1);

    check_int("Извлечение точного приоритета",
              queue_pop_priority(&queue, 150, &message), 1);
    check_text("FIFO при одинаковом приоритете",
               message.text, "Первое 150");

    check_int("Повторное извлечение приоритета 150",
              queue_pop_priority(&queue, 150, &message), 1);
    check_text("Второе сообщение того же приоритета",
               message.text, "Второе 150");

    check_int("Извлечение с порогом 90",
              queue_pop_min_priority(&queue, 90, &message), 1);
    check_int("Порог вернул приоритет 100", message.priority, 100);

    check_int("Нет сообщения с приоритетом 250",
              queue_pop_priority(&queue, 250, &message), 0);
    check_int("Нет сообщения не ниже 50",
              queue_pop_min_priority(&queue, 50, &message), 0);

    check_int("Запрет отрицательного приоритета",
              queue_push(&queue, 6, -1, "Ошибка"), 0);
    check_int("Запрет приоритета больше 255",
              queue_push(&queue, 7, 256, "Ошибка"), 0);

    check_int("В очереди остался один элемент", queue_count(&queue), 1);
    check_int("Извлечение последнего элемента",
              queue_pop_first(&queue, &message), 1);
    check_int("Последний приоритет равен 10", message.priority, 10);
    check_int("Очередь снова пуста", queue_is_empty(&queue), 1);
    check_int("Извлечение из пустой очереди",
              queue_pop_first(&queue, &message), 0);

    queue_push(&queue, 8, 1, "Один");
    queue_push(&queue, 9, 2, "Два");
    queue_clear(&queue);
    check_int("Очистка очереди", queue_is_empty(&queue), 1);
    check_int("Количество после очистки", queue_count(&queue), 0);

    printf("\nПройдено: %d\n", passed);
    printf("Ошибок: %d\n", failed);

    return failed == 0 ? 0 : 1;
}
