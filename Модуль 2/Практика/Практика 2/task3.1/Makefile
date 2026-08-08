CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
TARGET = permissions
TEST_TARGET = tests
OBJECTS = main.o permissions.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c permissions.h
	$(CC) $(CFLAGS) -c main.c -o main.o

permissions.o: permissions.c permissions.h
	$(CC) $(CFLAGS) -c permissions.c -o permissions.o

tests.o: tests.c permissions.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

$(TEST_TARGET): tests.o permissions.o
	$(CC) $(CFLAGS) tests.o permissions.o -o $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET)

.PHONY: all run test clean
