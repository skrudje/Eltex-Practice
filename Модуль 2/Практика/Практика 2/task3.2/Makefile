CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
TARGET = ipv4_sim
TEST_TARGET = tests
OBJECTS = main.o ipv4.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c ipv4.h
	$(CC) $(CFLAGS) -c main.c -o main.o

ipv4.o: ipv4.c ipv4.h
	$(CC) $(CFLAGS) -c ipv4.c -o ipv4.o

tests.o: tests.c ipv4.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

$(TEST_TARGET): tests.o ipv4.o
	$(CC) $(CFLAGS) tests.o ipv4.o -o $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET) 192.168.1.1 255.255.255.0 10

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET)

.PHONY: all run test clean
