CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
LDLIBS = -pthread -lrt

TARGET = shared_list
UNIT_TEST = tests
OBJECTS = main.o common.o ipc.o producer.o consumer.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LDLIBS)

main.o: main.c producer.h consumer.h ipc.h
	$(CC) $(CFLAGS) -c main.c -o main.o

common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

ipc.o: ipc.c ipc.h common.h
	$(CC) $(CFLAGS) -c ipc.c -o ipc.o

producer.o: producer.c producer.h common.h ipc.h
	$(CC) $(CFLAGS) -c producer.c -o producer.o

consumer.o: consumer.c consumer.h common.h ipc.h
	$(CC) $(CFLAGS) -c consumer.c -o consumer.o

tests.o: tests.c common.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

$(UNIT_TEST): tests.o common.o
	$(CC) $(CFLAGS) tests.o common.o -o $(UNIT_TEST)

unit-test: $(UNIT_TEST)
	./$(UNIT_TEST)

integration-test: $(TARGET)
	chmod +x test.sh
	./test.sh

test: unit-test integration-test

clean-ipc: $(TARGET)
	./$(TARGET) --cleanup

clean:
	rm -f *.o $(TARGET) $(UNIT_TEST)

.PHONY: all unit-test integration-test test clean-ipc clean
