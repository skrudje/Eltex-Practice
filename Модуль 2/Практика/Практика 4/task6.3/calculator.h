#ifndef CALCULATOR_H
#define CALCULATOR_H

#define OPERATION_NAME_SIZE 64
#define LIBRARY_NAME_SIZE 256

typedef double (*OperationFunction)(double, double);

typedef struct {
    char name[OPERATION_NAME_SIZE];
    char symbol;
    int order;
    OperationFunction function;
    void *library_handle;
    char library_name[LIBRARY_NAME_SIZE];
} Operation;

int operations_load(const char *directory,
                    Operation **operations,
                    int *count);

void operations_unload(Operation **operations, int *count);

#endif
