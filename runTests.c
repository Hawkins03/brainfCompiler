#include "ms.h"
#include "utils.h"
#include "test.h"
int main () {
    test_file("tests/test1.txt", "OP(NUM(1), +, NUM(1))");
    test_file("tests/arith1.txt", "NUM(1)");
    test_file("tests/arith2.txt", "OP(NUM(2), +, NUM(3))");
    test_file("tests/arith3.txt", "OP(NUM(1), +, NUM(4))");
    test_file("tests/arith4.txt", "OP(NUM(4), -, NUM(5))");
    test_file("tests/arith5.txt", "OP(NUM(6), -, NUM(7))");
    test_file("tests/arith6.txt", "OP(NUM(8), *, NUM(9))");
    test_file("tests/arith7.txt", "OP(NUM(10), /, NUM(5))");
    test_file("tests/arith8.txt", "OP(NUM(3), %, NUM(2))");
    test_file("tests/arith9.txt", "NUM(55)");
    test_file("tests/arith10.txt", "OP(NUM(10), /, NUM(6))");

    test_file("tests/prio1.txt", "OP(OP(NUM(1), *, NUM(3)), -, NUM(2))");
    test_file("tests/prio2.txt", "OP(NUM(3), +, OP(NUM(4), *, NUM(5)))");
    test_file("tests/prio3.txt", "OP(NUM(4), -, OP(NUM(5), *, NUM(6)))");
    test_file("tests/parenthesis1.txt", "NUM(1)");
    //test_file("tests/parenthesis2.txt", error);
    test_file("tests/parenthesis3.txt", "OP(NUM(4), *, OP(NUM(1), -, NUM(4)))");
    test_file("tests/parenthesis4.txt", "OP(NUM(4), *, NUM(9))");
    test_file("tests/parenthesis5.txt", "OP(NUM(4), *, OP(NUM(1), +, NUM(2)))");
    test_file("tests/parenthesis6.txt", "OP(OP(NUM(9), +, NUM(2)), *, NUM(6))");

    test_file("tests/char1.txt", "NUM(97)");
    //test_file("tests/char2.txt", error);
    test_file("tests/vars1.txt", "OP(NUM(1), *, STR(x))");
    test_file("tests/vars2.txt", "OP(NUM(1), *, STR(y))");
    test_file("tests/vars3.txt", "OP(NUM(1), *, STR(x_yz))");
    test_file("tests/vars4.txt", "OP(NUM(1), *, STR(x_2))");
    //test_file("tests/vars5.txt", error);

    //test_file("tests/binop1.txt", error);
    test_file("tests/binop2.txt", "OP(NUM(1), >, NUM(2))");
    test_file("tests/binop3.txt", "OP(NUM(2), <, NUM(3))");
    test_file("tests/binop4.txt", "OP(NUM(3), ==, NUM(4))");
    test_file("tests/binop5.txt", "OP(NUM(4), !=, NUM(5))");
    test_file("tests/binop6.txt", "OP(NUM(5), >=, NUM(6))");
    test_file("tests/binop7.txt", "OP(NUM(6), <=, NUM(7))");

    test_file("tests/binop8.txt", "OP(OP(NUM(1), *, NUM(2)), >, NUM(5))");
    test_file("tests/binop9.txt", "OP(OP(NUM(1), +, NUM(2)), <, NUM(5))");
    test_file("tests/binop10.txt", "OP(OP(NUM(1), <, NUM(5)), <, NUM(6))");
    test_file("tests/binop11.txt", "OP(OP(NUM(1), <, NUM(5)), <, NUM(6))");
    test_file("tests/binop12.txt", "OP(OP(NUM(1), *, NUM(2)), <, NUM(6))");
    test_file("tests/binop13.txt", "OP(OP(NUM(1), >, NUM(2)), &&, OP(NUM(3), >, NUM(4)))");
    test_file("tests/binop14.txt", "OP(NUM(1), ||, NUM(2))");

}
