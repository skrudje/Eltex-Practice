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

$(TEST_TARGET): tests.o phonebook_test.o
	$(CC) $(CFLAGS) tests.o phonebook_test.o -o $(TEST_TARGET)

tests.o: tests.c phonebook.h
	$(CC) $(CFLAGS) -c tests.c

phonebook_test.o: phonebook.c phonebook.h
	$(CC) $(CFLAGS) -DDATA_FILE=\"test_contacts.dat\" -c phonebook.c -o phonebook_test.o

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET) test_contacts.dat

.PHONY: all run test clean
