#include "operation_api.h"

const char operation_name[] = "Вычитание";
const char operation_symbol = '-';
const int operation_order = 2;

double operation(double a, double b) {
    return a - b;
}
