#include "parser.h"
#include "utils.h"
#include "test.h"
int main () {
    test_file("tests/atomic1.txt", "STR(x)");
    test_file("tests/atomic2.txt", "STR(abc)");
    test_file("tests/atomic3.txt", "STR(x1)");
    test_file("tests/atomic4.txt", "NUM(123)");
    test_file("tests/atomic5.txt", "UNARY(NULL, !, STR(x))");
    test_file("tests/atomic6.txt", "UNARY(NULL, ~, STR(x))");
    test_file("tests/atomic7.txt", "UNARY(NULL, ++, STR(x))");
    test_file("tests/atomic8.txt", "UNARY(NULL, --, STR(x))");
    test_file("tests/atomic9.txt", "UNARY(NULL, -, STR(x))");
    test_file("tests/atomic10.txt", "UNARY(NULL, -, UNARY(STR(x), ++, NULL))");

    test_file("tests/binary1.txt", "OP(STR(x), +, STR(y))");
	test_file("tests/binary2.txt", "OP(STR(x), -, STR(y))");
	test_file("tests/binary3.txt", "OP(STR(x), *, STR(y))");
	test_file("tests/binary4.txt", "OP(STR(x), /, STR(y))");
	test_file("tests/binary5.txt", "OP(STR(x), %, STR(y))");
	test_file("tests/binary6.txt", "OP(STR(x), |, STR(y))");
	test_file("tests/binary7.txt", "OP(STR(x), ^, STR(y))");
	test_file("tests/binary8.txt", "OP(STR(x), &, STR(y))");
	test_file("tests/binary9.txt", "OP(STR(x), <<, STR(y))");
	test_file("tests/binary10.txt", "OP(STR(x), >>, STR(y))");
	test_file("tests/binary11.txt", "OP(STR(x), <, STR(y))");
	test_file("tests/binary12.txt", "OP(STR(x), <=, STR(y))");
	test_file("tests/binary13.txt", "OP(STR(x), >, STR(y))");
	test_file("tests/binary14.txt", "OP(STR(x), >=, STR(y))");
	test_file("tests/binary15.txt", "OP(STR(x), ==, STR(y))");
	test_file("tests/binary16.txt", "OP(STR(x), !=, STR(y))");
	test_file("tests/binary17.txt", "OP(STR(x), &&, STR(y))");
	test_file("tests/binary18.txt", "OP(STR(x), ||, STR(y))");
	test_file("tests/binary19.txt", "OP(STR(x), =, STR(y))");
	test_file("tests/binary20.txt", "OP(STR(x), +=, STR(y))");
	test_file("tests/binary21.txt", "OP(STR(x), -=, STR(y))");
	test_file("tests/binary22.txt", "OP(STR(x), *=, STR(y))");
	test_file("tests/binary23.txt", "OP(STR(x), /=, STR(y))");
	test_file("tests/binary24.txt", "OP(STR(x), %=, STR(y))");
	test_file("tests/binary25.txt", "OP(STR(x), <<=, STR(y))");
	test_file("tests/binary26.txt", "OP(STR(x), >>=, STR(y))");
	test_file("tests/binary27.txt", "OP(STR(x), &=, STR(y))");
	test_file("tests/binary28.txt", "OP(STR(x), ^=, STR(y))");
	test_file("tests/binary29.txt", "OP(STR(x), |=, STR(y))");
	
	test_file("tests/prec1.txt", "OP(STR(x), +, OP(STR(y), *, STR(z)))");
	test_file("tests/prec2.txt", "OP(OP(STR(x), *, STR(y)), +, STR(z))");
	test_file("tests/prec3.txt", "OP(OP(STR(x), +, STR(y)), <, STR(z))");
	test_file("tests/prec4.txt", "OP(STR(x), <, OP(STR(y), +, STR(z)))");
	test_file("tests/prec5.txt", "OP(OP(STR(x), >, STR(y)), ==, STR(z))");
	test_file("tests/prec6.txt", "OP(STR(x), ==, OP(STR(y), >, STR(z)))");
	test_file("tests/prec7.txt", "OP(OP(STR(x), ==, STR(y)), ==, STR(z))");
	test_file("tests/prec8.txt", "OP(OP(STR(x), &&, STR(y)), ||, STR(z))");
	test_file("tests/prec9.txt", "OP(STR(x), ||, OP(STR(y), &&, STR(z)))");
	test_file("tests/prec10.txt", "OP(OP(OP(OP(OP(STR(a), +, OP(STR(b), *, STR(c))), <, STR(d)), ==, STR(e)), &&, STR(f)), ||, STR(g))");
	test_file("tests/prec11.txt", "OP(OP(OP(OP(OP(STR(a), *, STR(b)), +, STR(c)), <, STR(d)), &&, OP(STR(e), ==, STR(f))), ||, STR(g))");
	test_file("tests/prec12.txt", "OP(OP(STR(a), -, STR(b)), -, STR(c))");
	test_file("tests/prec13.txt", "OP(OP(STR(a), /, STR(b)), /, STR(c))");
	test_file("tests/prec14.txt", "OP(OP(STR(a), &&, STR(b)), &&, STR(c))");
	test_file("tests/prec15.txt", "OP(OP(STR(a), ||, STR(b)), ||, STR(c))");
	test_file("tests/prec16.txt", "OP(STR(a), =, OP(STR(b), =, STR(c)))");
	test_file("tests/prec17.txt", "OP(OP(STR(a), >, STR(b)), <, STR(c))");

	test_file("tests/parenthesis1.txt", "OP(OP(STR(x), +, STR(y)), *, STR(z))");
	test_file("tests/parenthesis2.txt", "OP(STR(x), *, OP(STR(y), +, STR(z)))");
	test_file("tests/parenthesis3.txt", "OP(OP(STR(x), +, STR(y)), <, STR(z))");
	test_file("tests/parenthesis4.txt", "OP(STR(x), >, OP(STR(y), +, STR(z)))");
	test_file("tests/parenthesis5.txt", "OP(OP(STR(x), &&, STR(y)), ||, STR(z))");
	test_file("tests/parenthesis6.txt", "OP(STR(x), &&, OP(STR(y), ||, STR(z)))");
	test_file("tests/parenthesis7.txt", "STR(x)");
	test_file("tests/parenthesis8.txt", "OP(STR(x), +, OP(STR(y), *, OP(STR(z), +, STR(w))))");
}
