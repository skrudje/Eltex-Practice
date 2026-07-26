CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
PICFLAGS = -fPIC

TARGET = phonebook
TEST_TARGET = tests
LIBRARY = libphonebook.so
RPATH = -Wl,-rpath,'$$ORIGIN'

all: $(TARGET)

$(LIBRARY): phonebook.o
	$(CC) -shared phonebook.o -o $(LIBRARY)

$(TARGET): main.o $(LIBRARY)
	$(CC) $(CFLAGS) main.o -L. $(RPATH) -lphonebook -o $(TARGET)

$(TEST_TARGET): tests.o $(LIBRARY)
	$(CC) $(CFLAGS) tests.o -L. $(RPATH) -lphonebook -o $(TEST_TARGET)

main.o: main.c phonebook.h
	$(CC) $(CFLAGS) -c main.c -o main.o

phonebook.o: phonebook.c phonebook.h
	$(CC) $(CFLAGS) $(PICFLAGS) -c phonebook.c -o phonebook.o

tests.o: tests.c phonebook.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

library: $(LIBRARY)

library-info: $(TARGET)
	@echo "Динамическая библиотека:"
	file $(LIBRARY)
	@echo ""
	@echo "Подключение библиотеки к программе:"
	ldd ./$(TARGET) | grep libphonebook  

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET) $(LIBRARY)

.PHONY: all library library-info run test clean
