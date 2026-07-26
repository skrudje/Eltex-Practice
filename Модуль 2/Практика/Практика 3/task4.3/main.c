#include <stdio.h>
#include "phonebook.h"

static int read_menu(void) {
    char line[30];
    int choice;

    while (1) {
        printf("Выберите действие: ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) != NULL &&
            sscanf(line, "%d", &choice) == 1) {
            return choice;
        }

        printf("Введите номер пункта меню.\n");
    }
}

int main(void) {
    PhoneBook book;
    int choice;

    phonebook_init(&book);
    phonebook_load(&book);

    do {
        printf("\n\n\n---- Книга контактов ---\n\n");
        printf("Контактов: %d\n", book.count);
        printf("Высота дерева: %d\n\n", phonebook_height(&book));
        printf("1. Добавить контакт\n");
        printf("2. Показать все контакты\n");
        printf("3. Изменить контакт\n");
        printf("4. Удалить контакт\n");
        printf("5. Балансировать дерево сейчас\n");
        printf("6. Показать дерево\n");
        printf("0. Выход\n");

        choice = read_menu();

        switch (choice) {
            case 1:
                contact_add(&book);
                break;
            case 2:
                contact_show_all(&book);
                break;
            case 3:
                contact_edit(&book);
                break;
            case 4:
                contact_delete(&book);
                break;
            case 5:
                if (phonebook_balance(&book)) {
                    phonebook_save(&book);
                    printf("Дерево сбалансировано.\n");
                } else {
                    printf("Не удалось выполнить балансировку.\n");
                }
                break;
            case 6:
                phonebook_print_tree(&book);
                break;
            case 0:
                printf("Работа программы завершена.\n");
                break;
            default:
                printf("Такого пункта меню нет.\n");
        }
    } while (choice != 0);

    phonebook_clear(&book);
    return 0;
}
