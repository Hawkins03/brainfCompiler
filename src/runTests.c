#include "parser.h"
#include "utils.h"
#include "test.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int run_one(const char *file, const char *expected, int catch_errors,
				   int *total, int *passed, int *failed, int *errors) {
	(*total)++;
	if (catch_errors) {
		pid_t pid = fork();
		if (pid == -1) {
			perror("fork");
			return -1;
		}
		if (pid == 0) {
			/* child */
			int res = test_file(file, expected);
			_exit(res);
		} else {
			int status = 0;
			waitpid(pid, &status, 0);
			if (WIFEXITED(status)) {
				int code = WEXITSTATUS(status);
				if (code == 0) {
					(*passed)++;
				} else if (code == 1) {
					(*failed)++;
				} else {
					(*errors)++;
				}
			} else {
				(*errors)++;
			}
		}
	} else {
		/* no catching: run directly (may abort on runtime error)
		   test_file returns 0 on success, 1 on mismatch */
		int res = test_file(file, expected);
		if (res == 0)
			(*passed)++;
		else
			(*failed)++;
	}
	return 0;
}

int main(int argc, char **argv) {
	int catch_errors = 0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--catch-errors") || !strcmp(argv[i], "-c"))
			catch_errors = 1;
	}

	int total = 0, passed = 0, failed = 0, errors = 0;

	run_one("tests/atomic1.txt", "STR(x)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic2.txt", "STR(abc)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic3.txt", "STR(x1)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic4.txt", "NUM(123)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic5.txt", "UNARY(NULL, !, STR(x))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic6.txt", "UNARY(NULL, ~, STR(x))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic7.txt", "UNARY(NULL, ++, STR(x))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic8.txt", "UNARY(NULL, --, STR(x))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic9.txt", "UNARY(NULL, -, STR(x))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic10.txt", "UNARY(NULL, -, UNARY(STR(x), ++, NULL))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic11.txt", "NUM(97)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic12.txt", "NUM(49)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic13.txt", "NUM(33)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic14.txt", "NUM(32)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/atomic15.txt", "NUM(0)", catch_errors, &total, &passed, &failed, &errors);

	run_one("tests/true1.txt", "NUM(1)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/false1.txt", "NUM(0)", catch_errors, &total, &passed, &failed, &errors);

	run_one("tests/binary1.txt", "OP(STR(x), +, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary2.txt", "OP(STR(x), -, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary3.txt", "OP(STR(x), *, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary4.txt", "OP(STR(x), /, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary5.txt", "OP(STR(x), %, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary6.txt", "OP(STR(x), |, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary7.txt", "OP(STR(x), ^, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary8.txt", "OP(STR(x), &, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary9.txt", "OP(STR(x), <<, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary10.txt", "OP(STR(x), >>, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary11.txt", "OP(STR(x), <, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary12.txt", "OP(STR(x), <=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary13.txt", "OP(STR(x), >, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary14.txt", "OP(STR(x), >=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary15.txt", "OP(STR(x), ==, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary16.txt", "OP(STR(x), !=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary17.txt", "OP(STR(x), &&, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary18.txt", "OP(STR(x), ||, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary19.txt", "OP(STR(x), =, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary20.txt", "OP(STR(x), +=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary21.txt", "OP(STR(x), -=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary22.txt", "OP(STR(x), *=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary23.txt", "OP(STR(x), /=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary24.txt", "OP(STR(x), %=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary25.txt", "OP(STR(x), <<=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary26.txt", "OP(STR(x), >>=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary27.txt", "OP(STR(x), ,, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary28.txt", "OP(STR(x), &=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary29.txt", "OP(STR(x), ^=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/binary30.txt", "OP(STR(x), |=, STR(y))", catch_errors, &total, &passed, &failed, &errors);
    
	run_one("tests/prec1.txt", "OP(STR(x), +, OP(STR(y), *, STR(z)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec2.txt", "OP(OP(STR(x), *, STR(y)), +, STR(z))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec3.txt", "OP(OP(STR(x), +, STR(y)), <, STR(z))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec4.txt", "OP(STR(x), <, OP(STR(y), +, STR(z)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec5.txt", "OP(OP(STR(x), >, STR(y)), ==, STR(z))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec6.txt", "OP(STR(x), ==, OP(STR(y), >, STR(z)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec7.txt", "OP(OP(STR(x), ==, STR(y)), ==, STR(z))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec8.txt", "OP(OP(STR(x), &&, STR(y)), ||, STR(z))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec9.txt", "OP(STR(x), ||, OP(STR(y), &&, STR(z)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec10.txt", "OP(OP(OP(OP(OP(STR(a), +, OP(STR(b), *, STR(c))), <, STR(d)), ==, STR(e)), &&, STR(f)), ||, STR(g))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec11.txt", "OP(OP(OP(OP(OP(STR(a), *, STR(b)), +, STR(c)), <, STR(d)), &&, OP(STR(e), ==, STR(f))), ||, STR(g))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec12.txt", "OP(OP(STR(a), -, STR(b)), -, STR(c))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec13.txt", "OP(OP(STR(a), /, STR(b)), /, STR(c))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec14.txt", "OP(OP(STR(a), &&, STR(b)), &&, STR(c))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec15.txt", "OP(OP(STR(a), ||, STR(b)), ||, STR(c))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec16.txt", "OP(STR(a), =, OP(STR(b), =, STR(c)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/prec17.txt", "OP(OP(STR(a), >, STR(b)), <, STR(c))", catch_errors, &total, &passed, &failed, &errors);

	run_one("tests/parenthesis1.txt", "OP(OP(STR(x), +, STR(y)), *, STR(z))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/parenthesis2.txt", "OP(STR(x), *, OP(STR(y), +, STR(z)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/parenthesis3.txt", "OP(OP(STR(x), +, STR(y)), <, STR(z))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/parenthesis4.txt", "OP(STR(x), >, OP(STR(y), +, STR(z)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/parenthesis5.txt", "OP(OP(STR(x), &&, STR(y)), ||, STR(z))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/parenthesis6.txt", "OP(STR(x), &&, OP(STR(y), ||, STR(z)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/parenthesis7.txt", "STR(x)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/parenthesis8.txt", "OP(STR(x), +, OP(STR(y), *, OP(STR(z), +, STR(w))))", catch_errors, &total, &passed, &failed, &errors);

	run_one("tests/stmt_var.txt", "VAR(STR(x), NUM(3))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/stmt_if.txt", "IF(OP(STR(x), >, NUM(0)), OP(STR(x), =, OP(STR(x), -, NUM(1))), OP(STR(x), =, OP(STR(x), +, NUM(1)))); ", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/stmt_while.txt", "LOOP(OP(STR(x), <, NUM(10)), OP(STR(x), =, OP(STR(x), +, NUM(1)))); ", catch_errors, &total, &passed, &failed, &errors);

	run_one("tests/call_print.txt", "CALL(6, STR(x))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/call_input.txt", "CALL(7, NULL)", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/call_break.txt", "CALL(8, NULL)", catch_errors, &total, &passed, &failed, &errors);

	run_one("tests/array1.txt", "ARR(STR(x), INDEX(NUM(3), NULL))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/array2.txt", "ARR(STR(x), INDEX(NUM(1), INDEX(STR(y), NULL)))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/init1.txt", "INITLIST(STR(x))", catch_errors, &total, &passed, &failed, &errors);

	run_one("tests/string_simple.txt", "INITLIST(OP(NUM(97), ,, OP(NUM(98), ,, NUM(99))))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/string_newline.txt", "INITLIST(OP(NUM(97), ,, OP(NUM(10), ,, NUM(98))))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/string_quote.txt", "INITLIST(OP(NUM(97), ,, OP(NUM(34), ,, NUM(98))))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/string_backslash.txt", "INITLIST(OP(NUM(97), ,, OP(NUM(92), ,, NUM(98))))", catch_errors, &total, &passed, &failed, &errors);
	run_one("tests/string_tab.txt", "INITLIST(OP(NUM(97), ,, OP(NUM(9), ,, NUM(98))))", catch_errors, &total, &passed, &failed, &errors);

	printf("\n===== Test Summary =====\n");
	printf("Total: %d\n", total);
	printf("Passed: %d\n", passed);
	printf("Failed (mismatches): %d\n", failed);
	printf("Runtime errors (caught): %d\n", errors);
	if (catch_errors) {
		printf("(Ran with --catch-errors; runtime errors were isolated per-test)\n");
	}
	return (failed > 0 || errors > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
