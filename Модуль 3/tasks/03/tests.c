#include "queues.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

static void test_name_with_slash(void)
{
    char first[CHAT_QUEUE_NAME_SIZE];
    char second[CHAT_QUEUE_NAME_SIZE];

    assert(queue_build_names("/chat", first, sizeof(first), second, sizeof(second)) == 0);
    assert(strcmp(first, "/chat_1") == 0);
    assert(strcmp(second, "/chat_2") == 0);
}

static void test_name_without_slash(void)
{
    char first[CHAT_QUEUE_NAME_SIZE];
    char second[CHAT_QUEUE_NAME_SIZE];

    assert(queue_build_names("chat", first, sizeof(first), second, sizeof(second)) == 0);
    assert(strcmp(first, "/chat_1") == 0);
    assert(strcmp(second, "/chat_2") == 0);
}

static void test_empty_name(void)
{
    char first[CHAT_QUEUE_NAME_SIZE];
    char second[CHAT_QUEUE_NAME_SIZE];

    errno = 0;
    assert(queue_build_names("", first, sizeof(first), second, sizeof(second)) == -1);
    assert(errno == EINVAL);
}

static void test_name_with_nested_path(void)
{
    char first[CHAT_QUEUE_NAME_SIZE];
    char second[CHAT_QUEUE_NAME_SIZE];

    errno = 0;
    assert(queue_build_names("/tmp/chat", first, sizeof(first), second, sizeof(second)) == -1);
    assert(errno == EINVAL);
}

int main(void)
{
    test_name_with_slash();
    test_name_without_slash();
    test_empty_name();
    test_name_with_nested_path();

    printf("Модульные тесты: 4/4 пройдены.\n");
    return 0;
}
