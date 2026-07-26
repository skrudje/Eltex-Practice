#include <stdio.h>
#include <string.h>
#include "phonebook.h"

#define DATA_FILE "contacts.dat"

static void read_text(const char *message, char text[], int size) {
    printf("%s", message);

    if (fgets(text, size, stdin) != NULL) {
        text[strcspn(text, "\n")] = '\0';
    }
}

static int read_number(const char *message) {
    char line[30];
    int number;

    while (1) {
        read_text(message, line, sizeof(line));

        if (sscanf(line, "%d", &number) == 1) {
            return number;
        }

        printf("Введите целое число.\n");
    }
}

static void read_required(const char *message, char text[], int size) {
    do {
        read_text(message, text, size);

        if (text[0] == '\0') {
            printf("Поле обязательно для заполнения.\n");
        }
    } while (text[0] == '\0');
}

static void phonebook_save(const Contact book[], int count) {
    FILE *file = fopen(DATA_FILE, "wb");
    if (file == NULL) {
        printf("Не удалось сохранить контакты.\n");
        return;
    }

    fwrite(&count, sizeof(count), 1, file);
    fwrite(book, sizeof(Contact), count, file);
    fclose(file);
}

void phonebook_load(Contact book[], int *count) {
    FILE *file = fopen(DATA_FILE, "rb");
    if (file == NULL) {
        *count = 0;
        return;
    }

    if (fread(count, sizeof(*count), 1, file) != 1 ||
        *count < 0 || *count > MAX_CONTACTS) {
        *count = 0;
        fclose(file);
        printf("Файл контактов повреждён. Создана пустая книга.\n");
        return;
    }

    if (*count > 0 &&
        fread(book, sizeof(Contact), *count, file) != (size_t)*count) {
        *count = 0;
        printf("Не удалось прочитать контакты.\n");
    }

    fclose(file);
}

static void contact_input(Contact *contact) {
    read_required("Фамилия: ", contact->surname, FIELD_SIZE);
    read_required("Имя: ", contact->name, FIELD_SIZE);
    read_text("Отчество: ", contact->patronymic, FIELD_SIZE);
    read_text("Место работы: ", contact->workplace, FIELD_SIZE);
    read_text("Должность: ", contact->position, FIELD_SIZE);
    read_text("Телефон: ", contact->phone, FIELD_SIZE);
    read_text("Электронная почта: ", contact->email, FIELD_SIZE);
    read_text("Социальная сеть: ", contact->social, FIELD_SIZE);
    read_text("Мессенджер: ", contact->messenger, FIELD_SIZE);
}

static void print_field(const char *title, const char *value) {
    if (value[0] != '\0') {
        printf("%s: %s\n", title, value);
    }
}

static void contact_print(const Contact *contact, int number) {
    printf("\n[%d] %s %s", number, contact->surname, contact->name);

    if (contact->patronymic[0] != '\0') {
        printf(" %s", contact->patronymic);
    }

    printf("\n");
    print_field("Место работы", contact->workplace);
    print_field("Должность", contact->position);
    print_field("Телефон", contact->phone);
    print_field("Электронная почта", contact->email);
    print_field("Социальная сеть", contact->social);
    print_field("Мессенджер", contact->messenger);
}

static void show_short_list(const Contact book[], int count) {
    int i;

    for (i = 0; i < count; i++) {
        printf("%d. %s %s\n", i + 1, book[i].surname, book[i].name);
    }
}

static void edit_contact_fields(Contact *contact) {
    int choice;

    do {
        printf("\nЧто изменить?\n");
        printf("1. Фамилия: %s\n", contact->surname);
        printf("2. Имя: %s\n", contact->name);
        printf("3. Отчество: %s\n", contact->patronymic);
        printf("4. Место работы: %s\n", contact->workplace);
        printf("5. Должность: %s\n", contact->position);
        printf("6. Телефон: %s\n", contact->phone);
        printf("7. Электронная почта: %s\n", contact->email);
        printf("8. Социальная сеть: %s\n", contact->social);
        printf("9. Мессенджер: %s\n", contact->messenger);
        printf("0. Завершить редактирование\n");

        choice = read_number("Выберите поле: ");

        switch (choice) {
            case 1:
                read_required("Новая фамилия: ", contact->surname, FIELD_SIZE);
                break;
            case 2:
                read_required("Новое имя: ", contact->name, FIELD_SIZE);
                break;
            case 3:
                read_text("Новое отчество: ", contact->patronymic, FIELD_SIZE);
                break;
            case 4:
                read_text("Новое место работы: ", contact->workplace, FIELD_SIZE);
                break;
            case 5:
                read_text("Новая должность: ", contact->position, FIELD_SIZE);
                break;
            case 6:
                read_text("Новый телефон: ", contact->phone, FIELD_SIZE);
                break;
            case 7:
                read_text("Новая электронная почта: ", contact->email, FIELD_SIZE);
                break;
            case 8:
                read_text("Новая социальная сеть: ", contact->social, FIELD_SIZE);
                break;
            case 9:
                read_text("Новый мессенджер: ", contact->messenger, FIELD_SIZE);
                break;
            case 0:
                break;
            default:
                printf("Такого поля нет.\n");
        }
    } while (choice != 0);
}

void contact_add(Contact book[], int *count) {
    if (*count >= MAX_CONTACTS) {
        printf("Телефонная книга заполнена.\n");
        return;
    }

    printf("\nДобавление контакта\n");
    contact_input(&book[*count]);
    (*count)++;
    phonebook_save(book, *count);

    printf("Контакт добавлен и сохранён.\n");
}

void contact_show_all(const Contact book[], int count) {
    int i;

    if (count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        contact_print(&book[i], i + 1);
    }
}

void contact_edit(Contact book[], int count) {
    int number;

    if (count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    show_short_list(book, count);
    number = read_number("Номер контакта для редактирования: ");

    if (number < 1 || number > count) {
        printf("Контакт с таким номером не найден.\n");
        return;
    }

    edit_contact_fields(&book[number - 1]);
    phonebook_save(book, count);

    printf("Изменения сохранены.\n");
}

void contact_delete(Contact book[], int *count) {
    int number;
    int i;

    if (*count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    show_short_list(book, *count);
    number = read_number("Номер контакта для удаления: ");

    if (number < 1 || number > *count) {
        printf("Контакт с таким номером не найден.\n");
        return;
    }

    for (i = number - 1; i < *count - 1; i++) {
        book[i] = book[i + 1];
    }

    (*count)--;
    phonebook_save(book, *count);

    printf("Контакт удалён.\n");
}
