#include "operation_api.h"

const char operation_name[] = "Сложение";
const char operation_symbol = '+';
const int operation_order = 1;

double operation(double a, double b) {
    return a + b;
}
