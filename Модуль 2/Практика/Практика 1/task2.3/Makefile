CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

all: calculator

calculator: main.c calculator.c calculator.h
	$(CC) $(CFLAGS) main.c calculator.c -o calculator

tests: tests.c calculator.c calculator.h
	$(CC) $(CFLAGS) tests.c calculator.c -o tests

run: calculator
	./calculator

test: tests
	./tests

clean:
	rm -f calculator tests
