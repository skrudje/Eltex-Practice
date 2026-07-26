#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "phonebook.h"

#define DATA_FILE "contacts.dat"
#define MAX_FILE_CONTACTS 10000

static void read_text(const char *message, char text[], int size) {
    printf("%s", message);
    fflush(stdout);

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

static int contact_compare(const Contact *first, const Contact *second) {
    int result = strcmp(first->surname, second->surname);

    if (result == 0) {
        result = strcmp(first->name, second->name);
    }

    if (result == 0) {
        result = strcmp(first->patronymic, second->patronymic);
    }

    return result;
}

static void insert_existing_node(PhoneBook *book, ContactNode *node) {
    ContactNode *current = book->head;

    while (current != NULL &&
           contact_compare(&current->contact, &node->contact) <= 0) {
        current = current->next;
    }

    if (book->head == NULL) {
        book->head = node;
        book->tail = node;
    } else if (current == book->head) {
        node->next = book->head;
        book->head->prev = node;
        book->head = node;
    } else if (current == NULL) {
        node->prev = book->tail;
        book->tail->next = node;
        book->tail = node;
    } else {
        node->prev = current->prev;
        node->next = current;
        current->prev->next = node;
        current->prev = node;
    }
}

static void detach_node(PhoneBook *book, ContactNode *node) {
    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        book->head = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    } else {
        book->tail = node->prev;
    }

    node->prev = NULL;
    node->next = NULL;
}

static int phonebook_save(const PhoneBook *book) {
    FILE *file = fopen(DATA_FILE, "wb");
    const ContactNode *current;

    if (file == NULL) {
        printf("Не удалось сохранить контакты.\n");
        return 0;
    }

    if (fwrite(&book->count, sizeof(book->count), 1, file) != 1) {
        fclose(file);
        printf("Не удалось сохранить контакты.\n");
        return 0;
    }

    current = book->head;
    while (current != NULL) {
        if (fwrite(&current->contact, sizeof(Contact), 1, file) != 1) {
            fclose(file);
            printf("Не удалось сохранить контакты.\n");
            return 0;
        }
        current = current->next;
    }

    fclose(file);
    return 1;
}

void phonebook_init(PhoneBook *book) {
    book->head = NULL;
    book->tail = NULL;
    book->count = 0;
}

int phonebook_insert(PhoneBook *book, const Contact *contact) {
    ContactNode *node = malloc(sizeof(ContactNode));

    if (node == NULL) {
        return 0;
    }

    node->contact = *contact;
    node->prev = NULL;
    node->next = NULL;

    insert_existing_node(book, node);
    book->count++;

    return 1;
}

ContactNode *phonebook_get_node(PhoneBook *book, int number) {
    ContactNode *current;
    int position;

    if (number < 1 || number > book->count) {
        return NULL;
    }

    if (number <= book->count / 2) {
        current = book->head;
        for (position = 1; position < number; position++) {
            current = current->next;
        }
    } else {
        current = book->tail;
        for (position = book->count; position > number; position--) {
            current = current->prev;
        }
    }

    return current;
}

int phonebook_remove(PhoneBook *book, int number) {
    ContactNode *node = phonebook_get_node(book, number);

    if (node == NULL) {
        return 0;
    }

    detach_node(book, node);
    free(node);
    book->count--;

    return 1;
}

int phonebook_update(PhoneBook *book, int number, const Contact *contact) {
    ContactNode *node = phonebook_get_node(book, number);

    if (node == NULL) {
        return 0;
    }

    detach_node(book, node);
    node->contact = *contact;
    insert_existing_node(book, node);

    return 1;
}

void phonebook_clear(PhoneBook *book) {
    ContactNode *current = book->head;

    while (current != NULL) {
        ContactNode *next = current->next;
        free(current);
        current = next;
    }

    phonebook_init(book);
}

void phonebook_load(PhoneBook *book) {
    FILE *file = fopen(DATA_FILE, "rb");
    int count;
    int i;

    if (file == NULL) {
        return;
    }

    if (fread(&count, sizeof(count), 1, file) != 1 ||
        count < 0 || count > MAX_FILE_CONTACTS) {
        fclose(file);
        printf("Файл контактов повреждён. Создана пустая книга.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        Contact contact;

        if (fread(&contact, sizeof(Contact), 1, file) != 1 ||
            !phonebook_insert(book, &contact)) {
            fclose(file);
            phonebook_clear(book);
            printf("Не удалось прочитать контакты.\n");
            return;
        }
    }

    fclose(file);
}

static void contact_input(Contact *contact) {
    memset(contact, 0, sizeof(*contact));

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

static void show_short_list(const PhoneBook *book) {
    const ContactNode *current = book->head;
    int number = 1;

    while (current != NULL) {
        printf("%d. %s %s\n",
               number, current->contact.surname, current->contact.name);
        current = current->next;
        number++;
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

void contact_add(PhoneBook *book) {
    Contact contact;

    printf("\nДобавление контакта\n");
    contact_input(&contact);

    if (!phonebook_insert(book, &contact)) {
        printf("Не удалось добавить контакт.\n");
        return;
    }

    phonebook_save(book);
    printf("Контакт добавлен и сохранён.\n");
}

void contact_show_all(const PhoneBook *book) {
    const ContactNode *current = book->head;
    int number = 1;

    if (book->count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    while (current != NULL) {
        contact_print(&current->contact, number);
        current = current->next;
        number++;
    }
}

void contact_edit(PhoneBook *book) {
    ContactNode *node;
    Contact updated;
    int number;

    if (book->count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    show_short_list(book);
    number = read_number("Номер контакта для редактирования: ");
    node = phonebook_get_node(book, number);

    if (node == NULL) {
        printf("Контакт с таким номером не найден.\n");
        return;
    }

    updated = node->contact;
    edit_contact_fields(&updated);
    phonebook_update(book, number, &updated);
    phonebook_save(book);

    printf("Изменения сохранены.\n");
}

void contact_delete(PhoneBook *book) {
    int number;

    if (book->count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    show_short_list(book);
    number = read_number("Номер контакта для удаления: ");

    if (!phonebook_remove(book, number)) {
        printf("Контакт с таким номером не найден.\n");
        return;
    }

    phonebook_save(book);
    printf("Контакт удалён.\n");
}
