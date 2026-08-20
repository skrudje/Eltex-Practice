#include "common.h"
#include "network.h"
#include "protocol.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

static int passed = 0;
static int failed = 0;

static void check(int condition, const char *name)
{
    if (condition) {
        printf("[OK] %s\n", name);
        passed++;
    } else {
        printf("[FAIL] %s\n", name);
        failed++;
    }
}

static void test_packet(void)
{
    ChatPacket packet;

    packet_build(&packet, CHAT_MESSAGE, 12345u, "Alice", "hello");

    check(packet_is_valid(&packet),
          "Корректный пакет проходит проверку");
    check(packet_get_type(&packet) == CHAT_MESSAGE,
          "Тип сообщения сохраняется");
    check(packet_get_sender_id(&packet) == 12345u,
          "Идентификатор отправителя сохраняется");
    check(strcmp(packet.name, "Alice") == 0 &&
          strcmp(packet.text, "hello") == 0,
          "Имя и текст сохраняются");
}

static void test_invalid_packet(void)
{
    ChatPacket packet;

    packet_build(&packet, CHAT_JOIN, 1u, "Bob", "");
    packet.magic = htonl(0u);
    check(!packet_is_valid(&packet),
          "Пакет с неверной сигнатурой отклоняется");
}

static void test_port(void)
{
    uint16_t port = 0;

    check(parse_port("51006", &port) == 0 && port == 51006,
          "Корректный UDP-порт разбирается");
    check(parse_port("80", &port) == -1,
          "Привилегированный порт отклоняется");
    check(parse_port("70000", &port) == -1,
          "Слишком большой порт отклоняется");
}

static void test_destination(void)
{
    struct sockaddr_in destination;

    check(make_destination("255.255.255.255", 51006, &destination) == 0,
          "Broadcast IPv4-адрес принимается");
    check(ntohs(destination.sin_port) == 51006,
          "Порт назначения сохраняется в сетевой структуре");
    check(make_destination("not-an-ip", 51006, &destination) == -1,
          "Некорректный IPv4-адрес отклоняется");
}

int main(void)
{
    test_packet();
    test_invalid_packet();
    test_port();
    test_destination();

    printf("\nПройдено: %d\n", passed);
    printf("Ошибок: %d\n", failed);

    return failed == 0 ? 0 : 1;
}
