CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
TARGET = file_copy

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	chmod +x test.sh
	./test.sh

clean:
	rm -f $(TARGET) *.copy

.PHONY: all run test clean
