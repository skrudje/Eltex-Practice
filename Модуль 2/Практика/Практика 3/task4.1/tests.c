#include <stdio.h>
#include <string.h>
#include "phonebook.h"

static int passed = 0;
static int failed = 0;

static Contact make_contact(const char *surname, const char *name) {
    Contact contact = {0};

    snprintf(contact.surname, FIELD_SIZE, "%s", surname);
    snprintf(contact.name, FIELD_SIZE, "%s", name);

    return contact;
}

static void check(int condition, const char *name) {
    if (condition) {
        printf("[OK]   %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s\n", name);
        failed++;
    }
}

static int links_are_correct(const PhoneBook *book) {
    const ContactNode *current = book->head;
    const ContactNode *previous = NULL;
    int count = 0;

    while (current != NULL) {
        if (current->prev != previous) {
            return 0;
        }

        previous = current;
        current = current->next;
        count++;
    }

    return previous == book->tail && count == book->count;
}

static void test_initial_state(void) {
    PhoneBook book;

    phonebook_init(&book);

    check(book.head == NULL, "У пустого списка head равен NULL");
    check(book.tail == NULL, "У пустого списка tail равен NULL");
    check(book.count == 0, "У пустого списка count равен 0");
}

static void test_sorted_insert(void) {
    PhoneBook book;
    Contact petrov = make_contact("Petrov", "Petr");
    Contact ivanov = make_contact("Ivanov", "Ivan");
    Contact sidorov = make_contact("Sidorov", "Sergey");

    phonebook_init(&book);
    phonebook_insert(&book, &petrov);
    phonebook_insert(&book, &ivanov);
    phonebook_insert(&book, &sidorov);

    check(book.count == 3, "После добавления count равен 3");
    check(strcmp(book.head->contact.surname, "Ivanov") == 0,
          "Первым стоит Ivanov");
    check(strcmp(book.head->next->contact.surname, "Petrov") == 0,
          "Вторым стоит Petrov");
    check(strcmp(book.tail->contact.surname, "Sidorov") == 0,
          "Последним стоит Sidorov");
    check(links_are_correct(&book), "Связи prev и next заполнены правильно");

    phonebook_clear(&book);
}

static void test_remove_middle(void) {
    PhoneBook book;
    Contact a = make_contact("A", "One");
    Contact b = make_contact("B", "Two");
    Contact c = make_contact("C", "Three");

    phonebook_init(&book);
    phonebook_insert(&book, &a);
    phonebook_insert(&book, &b);
    phonebook_insert(&book, &c);

    check(phonebook_remove(&book, 2) == 1, "Удаляется средний элемент");
    check(book.count == 2, "После удаления count уменьшается");
    check(strcmp(book.head->contact.surname, "A") == 0,
          "Голова списка не изменилась");
    check(strcmp(book.tail->contact.surname, "C") == 0,
          "Хвост списка не изменился");
    check(book.head->next == book.tail && book.tail->prev == book.head,
          "Соседние узлы соединены после удаления");

    phonebook_clear(&book);
}

static void test_remove_edges(void) {
    PhoneBook book;
    Contact a = make_contact("A", "One");
    Contact b = make_contact("B", "Two");

    phonebook_init(&book);
    phonebook_insert(&book, &a);
    phonebook_insert(&book, &b);

    check(phonebook_remove(&book, 1) == 1, "Удаляется первый элемент");
    check(book.head == book.tail, "После удаления остаётся один узел");
    check(book.head->prev == NULL && book.tail->next == NULL,
          "У единственного узла крайние ссылки равны NULL");
    check(phonebook_remove(&book, 1) == 1, "Удаляется последний элемент");
    check(book.head == NULL && book.tail == NULL && book.count == 0,
          "После удаления всех элементов список пуст");
}

static void test_update_and_reorder(void) {
    PhoneBook book;
    Contact a = make_contact("A", "One");
    Contact b = make_contact("B", "Two");
    Contact c = make_contact("C", "Three");
    Contact updated = make_contact("Z", "Last");

    phonebook_init(&book);
    phonebook_insert(&book, &a);
    phonebook_insert(&book, &b);
    phonebook_insert(&book, &c);

    check(phonebook_update(&book, 1, &updated) == 1,
          "Контакт обновляется");
    check(strcmp(book.tail->contact.surname, "Z") == 0,
          "После изменения фамилии узел перемещается в конец");
    check(strcmp(book.head->contact.surname, "B") == 0,
          "Новый первый элемент определён правильно");
    check(links_are_correct(&book),
          "После перемещения двусвязные ссылки не нарушены");

    phonebook_clear(&book);
}

static void test_get_node(void) {
    PhoneBook book;
    Contact a = make_contact("A", "One");
    Contact b = make_contact("B", "Two");
    Contact c = make_contact("C", "Three");

    phonebook_init(&book);
    phonebook_insert(&book, &a);
    phonebook_insert(&book, &b);
    phonebook_insert(&book, &c);

    check(phonebook_get_node(&book, 1) == book.head,
          "Получение первого узла");
    check(phonebook_get_node(&book, 3) == book.tail,
          "Получение последнего узла");
    check(phonebook_get_node(&book, 0) == NULL,
          "Неверный номер возвращает NULL");
    check(phonebook_get_node(&book, 4) == NULL,
          "Номер за границей списка возвращает NULL");

    phonebook_clear(&book);
}

int main(void) {
    test_initial_state();
    test_sorted_insert();
    test_remove_middle();
    test_remove_edges();
    test_update_and_reorder();
    test_get_node();

    printf("\nПройдено: %d\n", passed);
    printf("Ошибок: %d\n", failed);

    return failed == 0 ? 0 : 1;
}
