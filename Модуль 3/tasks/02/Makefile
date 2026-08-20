CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L

TARGET = pubsub
OBJECTS = main.o ipc.o protocol.o broker.o publisher.o subscriber.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

main.o: main.c broker.h publisher.h subscriber.h
	$(CC) $(CFLAGS) -c main.c -o main.o

ipc.o: ipc.c ipc.h protocol.h
	$(CC) $(CFLAGS) -c ipc.c -o ipc.o

protocol.o: protocol.c protocol.h
	$(CC) $(CFLAGS) -c protocol.c -o protocol.o

broker.o: broker.c broker.h ipc.h protocol.h
	$(CC) $(CFLAGS) -c broker.c -o broker.o

publisher.o: publisher.c publisher.h ipc.h protocol.h
	$(CC) $(CFLAGS) -c publisher.c -o publisher.o

subscriber.o: subscriber.c subscriber.h ipc.h protocol.h
	$(CC) $(CFLAGS) -c subscriber.c -o subscriber.o

run-broker: $(TARGET)
	./$(TARGET) -b

test: $(TARGET)
	bash ./test.sh

clean:
	rm -f $(OBJECTS) $(TARGET) *.log

.PHONY: all run-broker test clean
