CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
LDLIBS = -ldl

TARGET = calculator
TEST_TARGET = tests
PLUGIN_DIR = operations
PLUGIN_SOURCES = $(wildcard $(PLUGIN_DIR)/*.c)
PLUGIN_LIBRARIES = $(patsubst $(PLUGIN_DIR)/%.c,$(PLUGIN_DIR)/lib%.so,$(PLUGIN_SOURCES))

all: $(PLUGIN_LIBRARIES) $(TARGET)

$(PLUGIN_DIR)/lib%.so: $(PLUGIN_DIR)/%.c operation_api.h
	$(CC) $(CFLAGS) -fPIC -shared $< -I. -o $@

calculator.o: calculator.c calculator.h
	$(CC) $(CFLAGS) -c calculator.c -o calculator.o

main.o: main.c calculator.h
	$(CC) $(CFLAGS) -c main.c -o main.o

$(TARGET): main.o calculator.o
	$(CC) main.o calculator.o $(LDLIBS) -o $(TARGET)

tests.o: tests.c calculator.h
	$(CC) $(CFLAGS) -c tests.c -o tests.o

$(TEST_TARGET): tests.o calculator.o
	$(CC) tests.o calculator.o $(LDLIBS) -o $(TEST_TARGET)

run: all
	./$(TARGET)

test: $(PLUGIN_LIBRARIES) $(TEST_TARGET)
	./$(TEST_TARGET)

lib-info: $(PLUGIN_LIBRARIES)
	@echo "Загружаться будут следующие библиотеки:"
	@ls -1 $(PLUGIN_DIR)/*.so
	@echo ""
	@for library in $(PLUGIN_DIR)/*.so; do \
		echo "Символы $$library:"; \
		nm -D --defined-only $$library | grep -E 'operation($$|_)' || true; \
		echo ""; \
	done

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET) $(PLUGIN_DIR)/*.so

.PHONY: all run test lib-info clean
