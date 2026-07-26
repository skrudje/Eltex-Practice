#include <stdlib.h>
#include "calculator.h"

double calc_summ(double a, double b) {
    return a + b;
}
double calc_subtract(double a, double b) {
    return a - b;
}
double calc_multiply(double a, double b) {
    return a * b;
}
double calc_divide(double a, double b) {
    return a / b;
}
int operation_add(Operation **operations,
                  int *count,
                  const char *name,
                  char symbol,
                  OperationFunction function) {
    Operation *new_operations;

    new_operations = realloc(
        *operations,
        (*count + 1) * sizeof(Operation)
    );

    if (new_operations == NULL) {
        return 0;
    }

    *operations = new_operations;
    (*operations)[*count].name = name;
    (*operations)[*count].symbol = symbol;
    (*operations)[*count].function = function;
    (*count)++;

    return 1;
}
