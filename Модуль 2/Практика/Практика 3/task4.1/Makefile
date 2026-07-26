CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

TARGET = phonebook
TEST_TARGET = tests

all: $(TARGET)

$(TARGET): main.o phonebook.o
	$(CC) $(CFLAGS) main.o phonebook.o -o $(TARGET)

main.o: main.c phonebook.h
	$(CC) $(CFLAGS) -c main.c

phonebook.o: phonebook.c phonebook.h
	$(CC) $(CFLAGS) -c phonebook.c

$(TEST_TARGET): tests.o phonebook.o
	$(CC) $(CFLAGS) tests.o phonebook.o -o $(TEST_TARGET)

tests.o: tests.c phonebook.h
	$(CC) $(CFLAGS) -c tests.c

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET)

.PHONY: all run test clean
