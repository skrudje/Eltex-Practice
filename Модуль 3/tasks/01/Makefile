CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
TARGET = file_copy
OBJECTS = main.o channels.o transfer.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

main.o: main.c channels.h
	$(CC) $(CFLAGS) -c main.c -o main.o

channels.o: channels.c channels.h transfer.h
	$(CC) $(CFLAGS) -c channels.c -o channels.o

transfer.o: transfer.c transfer.h
	$(CC) $(CFLAGS) -c transfer.c -o transfer.o

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	chmod +x test.sh
	./test.sh

clean:
	rm -f $(TARGET) $(OBJECTS) *.copy *.data *.ready

.PHONY: all run test clean
