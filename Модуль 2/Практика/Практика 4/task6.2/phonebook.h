#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#define FIELD_SIZE 100

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

typedef struct ContactNode {
    Contact contact;
    struct ContactNode *prev;
    struct ContactNode *next;
} ContactNode;

typedef struct {
    ContactNode *head;
    ContactNode *tail;
    int count;
} PhoneBook;

void phonebook_init(PhoneBook *book);
void phonebook_load(PhoneBook *book);
void phonebook_clear(PhoneBook *book);

int phonebook_insert(PhoneBook *book, const Contact *contact);
ContactNode *phonebook_get_node(PhoneBook *book, int number);
int phonebook_remove(PhoneBook *book, int number);
int phonebook_update(PhoneBook *book, int number, const Contact *contact);

void contact_add(PhoneBook *book);
void contact_show_all(const PhoneBook *book);
void contact_edit(PhoneBook *book);
void contact_delete(PhoneBook *book);

#endif
