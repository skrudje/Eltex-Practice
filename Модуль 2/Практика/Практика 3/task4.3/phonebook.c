#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "phonebook.h"

#ifndef DATA_FILE
#define DATA_FILE "contacts.dat"
#endif

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

static TreeNode *create_node(const Contact *contact) {
    TreeNode *node = malloc(sizeof(TreeNode));

    if (node == NULL) {
        return NULL;
    }

    node->contact = *contact;
    node->left = NULL;
    node->right = NULL;

    return node;
}

static void insert_existing_node(TreeNode **root, TreeNode *node) {
    if (*root == NULL) {
        *root = node;
        return;
    }

    if (contact_compare(&node->contact, &(*root)->contact) < 0) {
        insert_existing_node(&(*root)->left, node);
    } else {
        insert_existing_node(&(*root)->right, node);
    }
}

static void clear_nodes(TreeNode *node) {
    if (node == NULL) {
        return;
    }

    clear_nodes(node->left);
    clear_nodes(node->right);
    free(node);
}

static void collect_nodes(TreeNode *node, TreeNode *nodes[], int *index) {
    if (node == NULL) {
        return;
    }

    collect_nodes(node->left, nodes, index);
    nodes[*index] = node;
    (*index)++;
    collect_nodes(node->right, nodes, index);
}

static TreeNode *build_balanced(TreeNode *nodes[], int left, int right) {
    int middle;
    TreeNode *root;

    if (left > right) {
        return NULL;
    }

    middle = left + (right - left) / 2;
    root = nodes[middle];
    root->left = build_balanced(nodes, left, middle - 1);
    root->right = build_balanced(nodes, middle + 1, right);

    return root;
}

static void register_change(PhoneBook *book) {
    book->changes_since_balance++;

    if (book->changes_since_balance >= BALANCE_INTERVAL) {
        phonebook_balance(book);
    }
}

static TreeNode *get_node_in_order(TreeNode *node, int target, int *position) {
    TreeNode *found;

    if (node == NULL) {
        return NULL;
    }

    found = get_node_in_order(node->left, target, position);
    if (found != NULL) {
        return found;
    }

    (*position)++;
    if (*position == target) {
        return node;
    }

    return get_node_in_order(node->right, target, position);
}

static TreeNode *remove_exact_node(TreeNode *root,
                                   TreeNode *target,
                                   int *removed) {
    TreeNode *replacement;
    TreeNode *successor;

    if (root == NULL || *removed) {
        return root;
    }

    if (root == target) {
        if (root->left == NULL) {
            replacement = root->right;
            free(root);
            *removed = 1;
            return replacement;
        }

        if (root->right == NULL) {
            replacement = root->left;
            free(root);
            *removed = 1;
            return replacement;
        }

        successor = root->right;
        while (successor->left != NULL) {
            successor = successor->left;
        }

        root->contact = successor->contact;
        root->right = remove_exact_node(root->right, successor, removed);
        return root;
    }

    root->left = remove_exact_node(root->left, target, removed);
    if (!*removed) {
        root->right = remove_exact_node(root->right, target, removed);
    }

    return root;
}

static int node_height(const TreeNode *node) {
    int left_height;
    int right_height;

    if (node == NULL) {
        return 0;
    }

    left_height = node_height(node->left);
    right_height = node_height(node->right);

    return 1 + (left_height > right_height ? left_height : right_height);
}

static int write_tree(FILE *file, const TreeNode *node) {
    if (node == NULL) {
        return 1;
    }

    if (!write_tree(file, node->left)) {
        return 0;
    }

    if (fwrite(&node->contact, sizeof(Contact), 1, file) != 1) {
        return 0;
    }

    return write_tree(file, node->right);
}

void phonebook_init(PhoneBook *book) {
    book->root = NULL;
    book->count = 0;
    book->changes_since_balance = 0;
}

int phonebook_insert(PhoneBook *book, const Contact *contact) {
    TreeNode *node = create_node(contact);

    if (node == NULL) {
        return 0;
    }

    insert_existing_node(&book->root, node);
    book->count++;
    register_change(book);

    return 1;
}

TreeNode *phonebook_get_node(PhoneBook *book, int number) {
    int position = 0;

    if (number < 1 || number > book->count) {
        return NULL;
    }

    return get_node_in_order(book->root, number, &position);
}

int phonebook_remove(PhoneBook *book, int number) {
    TreeNode *target = phonebook_get_node(book, number);
    int removed = 0;

    if (target == NULL) {
        return 0;
    }

    book->root = remove_exact_node(book->root, target, &removed);

    if (!removed) {
        return 0;
    }

    book->count--;
    register_change(book);
    return 1;
}

int phonebook_update(PhoneBook *book, int number, const Contact *contact) {
    TreeNode *target = phonebook_get_node(book, number);
    TreeNode *new_node;
    int removed = 0;

    if (target == NULL) {
        return 0;
    }

    new_node = create_node(contact);
    if (new_node == NULL) {
        return 0;
    }

    book->root = remove_exact_node(book->root, target, &removed);
    if (!removed) {
        free(new_node);
        return 0;
    }

    insert_existing_node(&book->root, new_node);
    register_change(book);

    return 1;
}

int phonebook_balance(PhoneBook *book) {
    TreeNode **nodes;
    int index = 0;

    if (book->count < 2) {
        book->changes_since_balance = 0;
        return 1;
    }

    nodes = malloc((size_t)book->count * sizeof(TreeNode *));
    if (nodes == NULL) {
        return 0;
    }

    collect_nodes(book->root, nodes, &index);
    book->root = build_balanced(nodes, 0, book->count - 1);
    book->changes_since_balance = 0;

    free(nodes);
    return 1;
}

int phonebook_height(const PhoneBook *book) {
    return node_height(book->root);
}

void phonebook_clear(PhoneBook *book) {
    clear_nodes(book->root);
    phonebook_init(book);
}

int phonebook_save(const PhoneBook *book) {
    FILE *file = fopen(DATA_FILE, "wb");

    if (file == NULL) {
        printf("Не удалось сохранить контакты.\n");
        return 0;
    }

    if (fwrite(&book->count, sizeof(book->count), 1, file) != 1 ||
        !write_tree(file, book->root)) {
        fclose(file);
        printf("Не удалось сохранить контакты.\n");
        return 0;
    }

    fclose(file);
    return 1;
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
    phonebook_balance(book);
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

static void print_tree(const TreeNode *node, int *number) {
    if (node == NULL) {
        return;
    }

    print_tree(node->left, number);
    contact_print(&node->contact, *number);
    (*number)++;
    print_tree(node->right, number);
}


#define MAX_TREE_PRINT_HEIGHT 6

static int contact_id(TreeNode *ordered[], int count, const TreeNode *node) {
    int i;

    for (i = 0; i < count; i++) {
        if (ordered[i] == node) {
            return i + 1;
        }
    }

    return 0;
}

static void put_label(char *canvas,
                      int rows,
                      int width,
                      int row,
                      int center,
                      const char *label) {
    int length = (int)strlen(label);
    int start = center - length / 2;
    int i;

    if (row < 0 || row >= rows) {
        return;
    }

    for (i = 0; i < length; i++) {
        int column = start + i;

        if (column >= 0 && column < width) {
            canvas[row * width + column] = label[i];
        }
    }
}

static int edge_rows_for_level(int height, int level) {
    int shift = height - level - 2;

    if (shift < 0) {
        return 0;
    }

    return 1 << shift;
}

static void place_tree_top_down(const TreeNode *node,
                                TreeNode *ordered[],
                                int count,
                                char *canvas,
                                int rows,
                                int width,
                                int height,
                                int level,
                                int row,
                                int left,
                                int right) {
    int center;
    int edge_rows;
    char label[32];

    if (node == NULL || left > right || level >= height) {
        return;
    }

    center = left + (right - left) / 2;
    snprintf(label, sizeof(label), "[%d]", contact_id(ordered, count, node));
    put_label(canvas, rows, width, row, center, label);

    edge_rows = edge_rows_for_level(height, level);

    if (node->left != NULL) {
        int child_center = left + (center - 1 - left) / 2;
        int step;

        for (step = 1; step <= edge_rows; step++) {
            int branch_row = row + step;
            int branch_column = center -
                ((center - child_center) * step) / (edge_rows + 1);

            if (branch_row < rows && branch_column >= 0 && branch_column < width) {
                canvas[branch_row * width + branch_column] = '/';
            }
        }

        place_tree_top_down(node->left,
                            ordered,
                            count,
                            canvas,
                            rows,
                            width,
                            height,
                            level + 1,
                            row + edge_rows + 1,
                            left,
                            center - 1);
    }

    if (node->right != NULL) {
        int child_center = center + 1 + (right - center - 1) / 2;
        int step;

        for (step = 1; step <= edge_rows; step++) {
            int branch_row = row + step;
            int branch_column = center +
                ((child_center - center) * step) / (edge_rows + 1);

            if (branch_row < rows && branch_column >= 0 && branch_column < width) {
                canvas[branch_row * width + branch_column] = '\\';
            }
        }

        place_tree_top_down(node->right,
                            ordered,
                            count,
                            canvas,
                            rows,
                            width,
                            height,
                            level + 1,
                            row + edge_rows + 1,
                            center + 1,
                            right);
    }
}

void phonebook_print_tree(const PhoneBook *book) {
    TreeNode **ordered;
    char *canvas;
    int index = 0;
    int height;
    int digits = 1;
    int slot_width;
    int leaf_count;
    int width;
    int rows;
    int row;
    int value;

    if (book->root == NULL) {
        printf("Бинарное дерево пусто.\n");
        return;
    }

    height = phonebook_height(book);

    if (height > MAX_TREE_PRINT_HEIGHT) {
        printf("Дерево имеет высоту %d и слишком широкое для наглядного вывода.\n",
               height);
        printf("Сначала выполните балансировку или уменьшите количество контактов.\n");
        return;
    }

    value = book->count;
    while (value >= 10) {
        digits++;
        value /= 10;
    }

    slot_width = digits + 4;
    leaf_count = 1 << (height - 1);
    width = leaf_count * slot_width;
    rows = height + (1 << (height - 1)) - 1;

    ordered = malloc((size_t)book->count * sizeof(TreeNode *));
    canvas = malloc((size_t)rows * (size_t)width);

    if (ordered == NULL || canvas == NULL) {
        free(ordered);
        free(canvas);
        printf("Не удалось подготовить вывод дерева.\n");
        return;
    }

    collect_nodes(book->root, ordered, &index);
    memset(canvas, ' ', (size_t)rows * (size_t)width);

    place_tree_top_down(book->root,
                        ordered,
                        book->count,
                        canvas,
                        rows,
                        width,
                        height,
                        0,
                        0,
                        0,
                        width - 1);

    printf("\nБинарное дерево контактов\n");
    printf("ID совпадает с номером контакта в алфавитном списке.\n\n");

    for (row = 0; row < rows; row++) {
        int end = width - 1;

        while (end >= 0 && canvas[row * width + end] == ' ') {
            end--;
        }

        if (end >= 0) {
            fwrite(&canvas[row * width], 1, (size_t)end + 1, stdout);
        }
        printf("\n");
    }

    free(canvas);
    free(ordered);
}

static void print_short_tree(const TreeNode *node, int *number) {
    if (node == NULL) {
        return;
    }

    print_short_tree(node->left, number);
    printf("%d. %s %s\n",
           *number, node->contact.surname, node->contact.name);
    (*number)++;
    print_short_tree(node->right, number);
}

static void show_short_list(const PhoneBook *book) {
    int number = 1;
    print_short_tree(book->root, &number);
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
    int number = 1;

    if (book->count == 0) {
        printf("Телефонная книга пуста.\n");
        return;
    }

    print_tree(book->root, &number);
}

void contact_edit(PhoneBook *book) {
    TreeNode *node;
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

    if (!phonebook_update(book, number, &updated)) {
        printf("Не удалось изменить контакт.\n");
        return;
    }

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
