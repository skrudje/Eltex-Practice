#include "operation_api.h"

const char operation_name[] = "Умножение";
const char operation_symbol = '*';
const int operation_order = 3;

double operation(double a, double b) {
    return a * b;
}
