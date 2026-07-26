#ifndef CALCULATOR_H
#define CALCULATOR_H

typedef double (*OperationFunction)(double, double);

typedef struct {
    const char *name;
    char symbol;
    OperationFunction function;
} Operation;

double calc_summ(double a, double b);
double calc_subtract(double a, double b);
double calc_multiply(double a, double b);
double calc_divide(double a, double b);

int operation_add(Operation **operations,
                  int *count,
                  const char *name,
                  char symbol,
                  OperationFunction function);
#endif
