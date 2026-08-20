CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L

TARGET = udp_chat
UNIT_TEST = tests
OBJECTS = main.o common.o protocol.o network.o chat.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c chat.h common.h protocol.h
	$(CC) $(CFLAGS) -c main.c -o main.o

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

protocol.o: protocol.c protocol.h
	$(CC) $(CFLAGS) -c protocol.c -o protocol.o

network.o: network.c network.h protocol.h
	$(CC) $(CFLAGS) -c network.c -o network.o

chat.o: chat.c chat.h common.h network.h protocol.h
	$(CC) $(CFLAGS) -c chat.c -o chat.o

tests.o: tests.c common.h network.h protocol.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

$(UNIT_TEST): tests.o common.o protocol.o network.o
	$(CC) $(CFLAGS) tests.o common.o protocol.o network.o -o $(UNIT_TEST)

unit-test: $(UNIT_TEST)
	./$(UNIT_TEST)

integration-test: $(TARGET)
	chmod +x test.sh
	./test.sh

test: unit-test integration-test

clean:
	rm -f *.o $(TARGET) $(UNIT_TEST)

.PHONY: all unit-test integration-test test clean
