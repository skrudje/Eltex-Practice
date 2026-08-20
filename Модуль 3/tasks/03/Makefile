CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
LDLIBS = -pthread -lrt

TARGET = p2p_chat
TEST_TARGET = unit_tests
OBJECTS = main.o chat.o queues.o
TEST_OBJECTS = tests.o queues.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDLIBS) -o $(TARGET)

$(TEST_TARGET): $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) $(LDLIBS) -o $(TEST_TARGET)

main.o: main.c chat.h queues.h
	$(CC) $(CFLAGS) -c main.c -o main.o

chat.o: chat.c chat.h queues.h protocol.h
	$(CC) $(CFLAGS) -c chat.c -o chat.o

queues.o: queues.c queues.h protocol.h
	$(CC) $(CFLAGS) -c queues.c -o queues.o

tests.o: tests.c queues.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

run: $(TARGET)
	./$(TARGET) /my_chat

unit-test: $(TEST_TARGET)
	./$(TEST_TARGET)

test: $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	./test.sh

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET)

.PHONY: all run unit-test test clean
