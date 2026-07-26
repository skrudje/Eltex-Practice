CC = gcc
AR = ar
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
ARFLAGS = rcs

TARGET = phonebook
TEST_TARGET = tests
LIBRARY = libphonebook.a

all: $(TARGET)

$(LIBRARY): phonebook.o
	$(AR) $(ARFLAGS) $(LIBRARY) phonebook.o

$(TARGET): main.o $(LIBRARY)
	$(CC) $(CFLAGS) main.o -L. -lphonebook -o $(TARGET)

$(TEST_TARGET): tests.o $(LIBRARY)
	$(CC) $(CFLAGS) tests.o -L. -lphonebook -o $(TEST_TARGET)

main.o: main.c phonebook.h
	$(CC) $(CFLAGS) -c main.c -o main.o

phonebook.o: phonebook.c phonebook.h
	$(CC) $(CFLAGS) -c phonebook.c -o phonebook.o

tests.o: tests.c phonebook.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

library: $(LIBRARY)

library-info: $(LIBRARY)
	@echo "Содержимое статической библиотеки $(LIBRARY):"
	$(AR) -t $(LIBRARY)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET) $(LIBRARY)

.PHONY: all library library-info run test clean
