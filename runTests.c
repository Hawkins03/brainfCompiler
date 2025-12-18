#include "ms.h"
#include "utils.h"
#include "test.h"
int main () {
    test_file("tests/test1.txt", "OP(NUM(1), +, NUM(1))");
}
