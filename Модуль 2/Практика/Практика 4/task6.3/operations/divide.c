#include "operation_api.h"

const char operation_name[] = "Деление";
const char operation_symbol = '/';
const int operation_order = 4;

double operation(double a, double b) {
    return a / b;
}
