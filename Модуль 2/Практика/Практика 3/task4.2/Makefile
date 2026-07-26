CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

PROGRAM = priority_queue
TEST_PROGRAM = tests

all: $(PROGRAM)

$(PROGRAM): main.o priority_queue.o
	$(CC) $(CFLAGS) main.o priority_queue.o -o $(PROGRAM)

main.o: main.c priority_queue.h
	$(CC) $(CFLAGS) -c main.c

priority_queue.o: priority_queue.c priority_queue.h
	$(CC) $(CFLAGS) -c priority_queue.c

$(TEST_PROGRAM): tests.o priority_queue.o
	$(CC) $(CFLAGS) tests.o priority_queue.o -o $(TEST_PROGRAM)

tests.o: tests.c priority_queue.h
	$(CC) $(CFLAGS) -c tests.c

run: $(PROGRAM)
	./$(PROGRAM)

test: $(TEST_PROGRAM)
	./$(TEST_PROGRAM)

clean:
	rm -f *.o $(PROGRAM) $(TEST_PROGRAM)

.PHONY: all run test clean
