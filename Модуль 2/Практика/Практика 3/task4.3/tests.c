#include <stdio.h>
#include <stdlib.h>
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

static int tree_is_sorted(const TreeNode *node,
                          const char **previous_surname) {
    if (node == NULL) {
        return 1;
    }

    if (!tree_is_sorted(node->left, previous_surname)) {
        return 0;
    }

    if (*previous_surname != NULL &&
        strcmp(*previous_surname, node->contact.surname) > 0) {
        return 0;
    }

    *previous_surname = node->contact.surname;
    return tree_is_sorted(node->right, previous_surname);
}

static void test_initial_state(void) {
    PhoneBook book;

    phonebook_init(&book);

    check(book.root == NULL, "У пустого дерева root равен NULL");
    check(book.count == 0, "У пустого дерева count равен 0");
    check(book.changes_since_balance == 0,
          "Счётчик изменений сначала равен 0");
    check(phonebook_height(&book) == 0, "Высота пустого дерева равна 0");
}

static void test_sorted_insert(void) {
    PhoneBook book;
    Contact petrov = make_contact("Petrov", "Petr");
    Contact ivanov = make_contact("Ivanov", "Ivan");
    Contact sidorov = make_contact("Sidorov", "Sergey");
    const char *previous = NULL;

    phonebook_init(&book);
    phonebook_insert(&book, &petrov);
    phonebook_insert(&book, &ivanov);
    phonebook_insert(&book, &sidorov);

    check(book.count == 3, "После добавления count равен 3");
    check(strcmp(phonebook_get_node(&book, 1)->contact.surname,
                 "Ivanov") == 0,
          "Первым по порядку стоит Ivanov");
    check(strcmp(phonebook_get_node(&book, 2)->contact.surname,
                 "Petrov") == 0,
          "Вторым по порядку стоит Petrov");
    check(strcmp(phonebook_get_node(&book, 3)->contact.surname,
                 "Sidorov") == 0,
          "Третьим по порядку стоит Sidorov");
    check(tree_is_sorted(book.root, &previous),
          "Симметричный обход выдаёт отсортированный список");

    phonebook_clear(&book);
}

static void test_periodic_balance(void) {
    PhoneBook book;
    Contact contacts[BALANCE_INTERVAL];
    int i;

    phonebook_init(&book);

    for (i = 0; i < BALANCE_INTERVAL; i++) {
        char surname[20];
        snprintf(surname, sizeof(surname), "Name%02d", i);
        contacts[i] = make_contact(surname, "Test");
        phonebook_insert(&book, &contacts[i]);
    }

    check(book.changes_since_balance == 0,
          "После пяти изменений счётчик сбрасывается");
    check(phonebook_height(&book) <= 3,
          "После периодической балансировки высота небольшая");

    phonebook_clear(&book);
}

static void test_manual_balance(void) {
    PhoneBook book;
    Contact a = make_contact("A", "One");
    Contact b = make_contact("B", "Two");
    Contact c = make_contact("C", "Three");
    Contact d = make_contact("D", "Four");

    phonebook_init(&book);
    phonebook_insert(&book, &a);
    phonebook_insert(&book, &b);
    phonebook_insert(&book, &c);
    phonebook_insert(&book, &d);

    check(phonebook_height(&book) == 4,
          "До ручной балансировки дерево вырождено");
    check(phonebook_balance(&book) == 1,
          "Ручная балансировка выполняется");
    check(phonebook_height(&book) == 3,
          "После балансировки высота уменьшается");

    phonebook_clear(&book);
}

static void test_remove(void) {
    PhoneBook book;
    Contact m = make_contact("M", "Root");
    Contact c = make_contact("C", "Left");
    Contact t = make_contact("T", "Right");
    Contact a = make_contact("A", "Leaf");
    const char *previous = NULL;

    phonebook_init(&book);
    phonebook_insert(&book, &m);
    phonebook_insert(&book, &c);
    phonebook_insert(&book, &t);
    phonebook_insert(&book, &a);

    check(phonebook_remove(&book, 3) == 1,
          "Удаляется узел с двумя потомками");
    check(book.count == 3, "После удаления count уменьшается");
    check(phonebook_get_node(&book, 0) == NULL,
          "Номер 0 считается неверным");
    check(phonebook_get_node(&book, 4) == NULL,
          "Номер за границей считается неверным");
    check(tree_is_sorted(book.root, &previous),
          "После удаления дерево остаётся упорядоченным");

    phonebook_clear(&book);
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
    check(book.count == 3,
          "Количество контактов после изменения не меняется");
    check(strcmp(phonebook_get_node(&book, 3)->contact.surname,
                 "Z") == 0,
          "После изменения фамилии контакт переходит в конец");

    phonebook_clear(&book);
}

static void test_save_and_load(void) {
    PhoneBook first;
    PhoneBook second;
    Contact b = make_contact("B", "Two");
    Contact a = make_contact("A", "One");

    remove("test_contacts.dat");
    phonebook_init(&first);
    phonebook_insert(&first, &b);
    phonebook_insert(&first, &a);

    check(phonebook_save(&first) == 1, "Контакты сохраняются в файл");

    phonebook_init(&second);
    phonebook_load(&second);

    check(second.count == 2, "Из файла загружаются два контакта");
    check(strcmp(phonebook_get_node(&second, 1)->contact.surname,
                 "A") == 0,
          "После загрузки контакты остаются упорядоченными");
    check(phonebook_height(&second) <= 2,
          "После загрузки дерево сбалансировано");

    phonebook_clear(&first);
    phonebook_clear(&second);
    remove("test_contacts.dat");
}

int main(void) {
    test_initial_state();
    test_sorted_insert();
    test_periodic_balance();
    test_manual_balance();
    test_remove();
    test_update_and_reorder();
    test_save_and_load();

    printf("\nПройдено: %d\n", passed);
    printf("Ошибок: %d\n", failed);

    return failed == 0 ? 0 : 1;
}
