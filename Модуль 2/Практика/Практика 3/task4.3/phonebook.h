#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#define FIELD_SIZE 100
#define BALANCE_INTERVAL 5

typedef struct {
    char surname[FIELD_SIZE];
    char name[FIELD_SIZE];
    char patronymic[FIELD_SIZE];
    char workplace[FIELD_SIZE];
    char position[FIELD_SIZE];
    char phone[FIELD_SIZE];
    char email[FIELD_SIZE];
    char social[FIELD_SIZE];
    char messenger[FIELD_SIZE];
} Contact;

typedef struct TreeNode {
    Contact contact;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

typedef struct {
    TreeNode *root;
    int count;
    int changes_since_balance;
} PhoneBook;

void phonebook_init(PhoneBook *book);
void phonebook_load(PhoneBook *book);
int phonebook_save(const PhoneBook *book);
void phonebook_clear(PhoneBook *book);

int phonebook_insert(PhoneBook *book, const Contact *contact);
TreeNode *phonebook_get_node(PhoneBook *book, int number);
int phonebook_remove(PhoneBook *book, int number);
int phonebook_update(PhoneBook *book, int number, const Contact *contact);
int phonebook_balance(PhoneBook *book);
int phonebook_height(const PhoneBook *book);
void phonebook_print_tree(const PhoneBook *book);

void contact_add(PhoneBook *book);
void contact_show_all(const PhoneBook *book);
void contact_edit(PhoneBook *book);
void contact_delete(PhoneBook *book);

#endif
