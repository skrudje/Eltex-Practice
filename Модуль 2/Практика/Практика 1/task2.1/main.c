#include <stdio.h>
#include "phonebook.h"

static int read_menu(void) {
    char line[30];
    int choice;

    while (1) {
        printf("Выберите действие: ");

        if (fgets(line, sizeof(line), stdin) != NULL &&
            sscanf(line, "%d", &choice) == 1) {
            return choice;
        }

        printf("Введите номер пункта меню.\n");
    }
}

int main(void) {
    Contact book[MAX_CONTACTS];
    int count;
    int choice;

    phonebook_load(book, &count);

    do {
        printf("\n\n\n---- Книга контактов ----\n\n");
        printf("1. Добавить контакт\n");
        printf("2. Показать все контакты\n");
        printf("3. Изменить контакт\n");
        printf("4. Удалить контакт\n");
        printf("0. Выход\n");

        choice = read_menu();

        switch (choice) {
            case 1:
                contact_add(book, &count);
                break;
            case 2:
                contact_show_all(book, count);
                break;
            case 3:
                contact_edit(book, count);
                break;
            case 4:
                contact_delete(book, &count);
                break;
            case 0:
                printf("Работа программы завершена.\n");
                break;
            default:
                printf("Такого пункта меню нет.\n");
        }
    } while (choice != 0);

    return 0;
}
