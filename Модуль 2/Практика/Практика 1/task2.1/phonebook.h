#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#define MAX_CONTACTS 100
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

void phonebook_load(Contact book[], int *count);
void contact_add(Contact book[], int *count);
void contact_show_all(const Contact book[], int count);
void contact_edit(Contact book[], int count);
void contact_delete(Contact book[], int *count);

#endif
