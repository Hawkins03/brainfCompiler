#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "interp.h"
#include "parser.h"
#include "utils.h"
#include "test.h"
#include "stmt.h"
#include "exp.h"

struct strbuf {
    char *buf;
    size_t len;
    size_t cap;
};

void strbuf_init(struct strbuf *sb) {
    sb->cap = 256;
    sb->len = 0;
    sb->buf = malloc(sb->cap);
    if (!sb->buf) {
        raise_error(ERR_NO_MEM);
    }
}

void strbuf_ensure(struct strbuf *sb, size_t needed) {
    while (sb->len + needed >= sb->cap) {
        sb->cap *= 2;
        char *new_buf = realloc(sb->buf, sb->cap);
        if (!new_buf) {
            free(sb->buf);
            raise_error(ERR_NO_MEM);
        }
        sb->buf = new_buf;
    }
}

void strbuf_appendf(struct strbuf *sb, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    
    if (needed < 0) {
        raise_error(ERR_INTERNAL);
    }
    
    strbuf_ensure(sb, needed + 1);
    va_start(ap, fmt);
    sb->len += vsprintf(sb->buf + sb->len, fmt, ap);
    va_end(ap);
}

void strbuf_free(struct strbuf *sb) {
    if (sb && sb->buf) {
        free(sb->buf);
        sb->buf = NULL;
        sb->len = 0;
        sb->cap = 0;
    }
}

// Forward declarations
void build_exp_str(struct strbuf *sb, const struct exp *exp);
void build_stmt_str(struct strbuf *sb, const struct stmt *stmt);

void build_exp_str(struct strbuf *sb, const struct exp *exp) {
    if (!exp) {
        strbuf_appendf(sb, "NULL");
        return;
    }
    
    switch (exp->type) {
    case EXP_EMPTY:
        strbuf_appendf(sb, "EMPTY()");
        break;
    case EXP_NAME:
        strbuf_appendf(sb, "STR(%s)", exp->name);
        break;
    case EXP_ARRAY_REF:
        strbuf_appendf(sb, "ARR(");
        build_exp_str(sb, exp->array_ref->name);
        strbuf_appendf(sb, ", ");
        build_exp_str(sb, exp->array_ref->index);
        strbuf_appendf(sb, ")");
        break;
    case EXP_ARRAY_LIT:
        strbuf_appendf(sb, "ARR_LIT(");
        for (int i = 0; i < exp->array_lit->size; i++) {
            if (i > 0)
                strbuf_appendf(sb, ", ");

            build_exp_str(sb, exp->array_lit->array + i);
        }
        strbuf_appendf(sb, ")");
        break;
    case EXP_NUM:
        strbuf_appendf(sb, "NUM(%d)", exp->num);
        break;
    case EXP_BINARY_OP:
    case EXP_ASSIGN_OP:
        strbuf_appendf(sb, "OP(");
        build_exp_str(sb, exp->op->left);
        strbuf_appendf(sb, ", %s, ", getOpStr(exp->op->op));
        build_exp_str(sb, exp->op->right);
        strbuf_appendf(sb, ")");
        break;
    case EXP_UNARY:
        strbuf_appendf(sb, "UNARY(");
        if (exp->unary->is_prefix) {
            strbuf_appendf(sb, "%s, ", getOpStr(exp->unary->op));
            build_exp_str(sb, exp->unary->operand);
        } else {
            build_exp_str(sb, exp->unary->operand);
            strbuf_appendf(sb, ", %s", getOpStr(exp->unary->op));
        }
        strbuf_appendf(sb, ")");
        break;
    case EXP_CALL:
        strbuf_appendf(sb, "CALL(%d, ", exp->call->key);
        build_exp_str(sb, exp->call->arg);
        strbuf_appendf(sb, ")");
        break;
    }
}

void build_stmt_str(struct strbuf *sb, const struct stmt *stmt) {
    if (!stmt) {
	strbuf_appendf(sb, "NULL");
        return;
    }
    
    switch (stmt->type) {
    case STMT_EMPTY:
        strbuf_appendf(sb, "EMPTY();");
        break;
    case STMT_EXPR:
        build_exp_str(sb, stmt->exp);
        strbuf_appendf(sb, ";");
        break;
    case STMT_VAR: {
        const char *name = (stmt->var->is_mutable) ? "VAR" : "VAL";
        strbuf_appendf(sb, "%s(", name);
        build_exp_str(sb, stmt->var->name);
        strbuf_appendf(sb, ", ");
        build_exp_str(sb, stmt->var->value);
        strbuf_appendf(sb, ");");
        break;
    }
    case STMT_IF:
        strbuf_appendf(sb, "IF(");
        build_exp_str(sb, stmt->ifStmt->cond);
        strbuf_appendf(sb, ", ");
        build_stmt_str(sb, stmt->ifStmt->thenStmt);
        if (stmt->ifStmt->elseStmt) {
            strbuf_appendf(sb, ", ");
            build_stmt_str(sb, stmt->ifStmt->elseStmt);
        }
        strbuf_appendf(sb, ");");
        break;
    case STMT_LOOP:
        strbuf_appendf(sb, "LOOP(");
        build_exp_str(sb, stmt->loop->cond);
        strbuf_appendf(sb, ", ");
        build_stmt_str(sb, stmt->loop->body);
        strbuf_appendf(sb, ");");
        break;
    }
    
    if (stmt->next && stmt->next != stmt) {
        strbuf_appendf(sb, " ");
        build_stmt_str(sb, stmt->next);
    }
}

int test_file(const char *input_file, const char *expected) {
    struct stmt *stmt = parse_file(input_file);

    if (stmt->next == stmt) {
        free_stmt(stmt);
        raise_error(ERR_INF_REC);
    }
    
    struct strbuf sb;
    strbuf_init(&sb);
    build_stmt_str(&sb, stmt);
    
    int status = 0;
    if (strcmp(sb.buf, expected)) {
        fprintf(stderr, "input: \"%s\" mismatch\nact: \t\t\"%s\"\nexpected:\t\"%s\"\n\n",
                input_file, sb.buf, expected);
        status = 1;
    }

    strbuf_free(&sb);
    free_stmt(stmt);
    return status;
}

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
					fprintf(stderr, "Runtime error in test: %s\n", file);
				}
			} else {
				(*errors)++;
				fprintf(stderr, "Runtime error (signal) in test: %s\n", file);
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

int run_parser_tests(int catch_errors) {
	int total = 0, passed = 0, failed = 0, errors = 0;

	run_one("parser_tests/atomic1.txt", "STR(x);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic2.txt", "STR(abc);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic3.txt", "STR(x1);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic4.txt", "NUM(123);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic5.txt", "UNARY(!, STR(x));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic6.txt", "UNARY(~, STR(x));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic7.txt", "UNARY(++, STR(x));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic8.txt", "UNARY(--, STR(x));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic9.txt", "UNARY(-, STR(x));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic10.txt", "UNARY(-, UNARY(STR(x), ++));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic11.txt", "NUM(97);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic12.txt", "NUM(49);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic13.txt", "NUM(33);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic14.txt", "NUM(32);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/atomic15.txt", "NUM(0);", catch_errors, &total, &passed, &failed, &errors);

	run_one("parser_tests/true1.txt", "NUM(1);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/false1.txt", "NUM(0);", catch_errors, &total, &passed, &failed, &errors);

	run_one("parser_tests/binary1.txt", "OP(STR(x), +, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary2.txt", "OP(STR(x), -, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary3.txt", "OP(STR(x), *, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary4.txt", "OP(STR(x), /, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary5.txt", "OP(STR(x), %, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary6.txt", "OP(STR(x), |, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary7.txt", "OP(STR(x), ^, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary8.txt", "OP(STR(x), &, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary9.txt", "OP(STR(x), <<, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary10.txt", "OP(STR(x), >>, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary11.txt", "OP(STR(x), <, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary12.txt", "OP(STR(x), <=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary13.txt", "OP(STR(x), >, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary14.txt", "OP(STR(x), >=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary15.txt", "OP(STR(x), ==, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary16.txt", "OP(STR(x), !=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary17.txt", "OP(STR(x), &&, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary18.txt", "OP(STR(x), ||, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary19.txt", "OP(STR(x), =, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary20.txt", "OP(STR(x), +=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary21.txt", "OP(STR(x), -=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary22.txt", "OP(STR(x), *=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary23.txt", "OP(STR(x), /=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary24.txt", "OP(STR(x), %=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary25.txt", "OP(STR(x), <<=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary26.txt", "OP(STR(x), >>=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary27.txt", "OP(STR(x), &=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary28.txt", "OP(STR(x), ^=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/binary29.txt", "OP(STR(x), |=, STR(y));", catch_errors, &total, &passed, &failed, &errors);
    
	run_one("parser_tests/prec1.txt", "OP(STR(x), +, OP(STR(y), *, STR(z)));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec2.txt", "OP(OP(STR(x), *, STR(y)), +, STR(z));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec3.txt", "OP(OP(STR(x), +, STR(y)), <, STR(z));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec4.txt", "OP(STR(x), <, OP(STR(y), +, STR(z)));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec5.txt", "OP(OP(STR(x), >, STR(y)), ==, STR(z));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec6.txt", "OP(STR(x), ==, OP(STR(y), >, STR(z)));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec7.txt", "OP(OP(STR(x), ==, STR(y)), ==, STR(z));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec8.txt", "OP(OP(STR(x), &&, STR(y)), ||, STR(z));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec9.txt", "OP(STR(x), ||, OP(STR(y), &&, STR(z)));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec10.txt", "OP(OP(OP(OP(OP(STR(a), +, OP(STR(b), *, STR(c))), <, STR(d)), ==, STR(e)), &&, STR(f)), ||, STR(g));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec11.txt", "OP(OP(OP(OP(OP(STR(a), *, STR(b)), +, STR(c)), <, STR(d)), &&, OP(STR(e), ==, STR(f))), ||, STR(g));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec12.txt", "OP(OP(STR(a), -, STR(b)), -, STR(c));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec13.txt", "OP(OP(STR(a), /, STR(b)), /, STR(c));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec14.txt", "OP(OP(STR(a), &&, STR(b)), &&, STR(c));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec15.txt", "OP(OP(STR(a), ||, STR(b)), ||, STR(c));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec16.txt", "OP(STR(a), =, OP(STR(b), =, STR(c)));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/prec17.txt", "OP(OP(STR(a), >, STR(b)), <, STR(c));", catch_errors, &total, &passed, &failed, &errors);

	run_one("parser_tests/parenthesis1.txt", "OP(OP(STR(x), +, STR(y)), *, STR(z));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/parenthesis2.txt", "OP(STR(x), *, OP(STR(y), +, STR(z)));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/parenthesis3.txt", "OP(OP(STR(x), +, STR(y)), <, STR(z));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/parenthesis4.txt", "OP(STR(x), >, OP(STR(y), +, STR(z)));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/parenthesis5.txt", "OP(OP(STR(x), &&, STR(y)), ||, STR(z));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/parenthesis6.txt", "OP(STR(x), &&, OP(STR(y), ||, STR(z)));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/parenthesis7.txt", "STR(x);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/parenthesis8.txt", "OP(STR(x), +, OP(STR(y), *, OP(STR(z), +, STR(w))));", catch_errors, &total, &passed, &failed, &errors);

	run_one("parser_tests/stmt_var.txt", "VAR(STR(x), NUM(3));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/stmt_if.txt", "IF(OP(STR(x), >, NUM(0)), OP(STR(x), =, OP(STR(x), -, NUM(1)));, OP(STR(x), =, OP(STR(x), +, NUM(1))););", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/stmt_while.txt", "LOOP(OP(STR(x), <, NUM(10)), OP(STR(x), =, OP(STR(x), +, NUM(1))););", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/stmt_for.txt", "VAR(STR(i), NUM(0)); LOOP(OP(STR(i), <, NUM(10)), CALL(6, STR(i)); UNARY(STR(i), ++););", catch_errors, &total, &passed, &failed, &errors);

	run_one("parser_tests/call_print.txt", "CALL(6, STR(x));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/call_input.txt", "CALL(7, NULL);", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/call_break.txt", "CALL(8, NULL);", catch_errors, &total, &passed, &failed, &errors);

	run_one("parser_tests/array1.txt", "ARR(STR(x), NUM(3));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/array2.txt", "ARR(ARR(STR(x), NUM(1)), STR(y));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/array3.txt", "ARR(ARR(ARR(STR(x), NUM(1)), NUM(2)), NUM(3));", catch_errors, &total, &passed, &failed, &errors);

	run_one("parser_tests/array4.txt", "ARR_LIT(NUM(0));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/array5.txt", "ARR_LIT(ARR_LIT(NUM(1), NUM(2)), NUM(3));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/array6.txt", "VAR(ARR(ARR(STR(x), NUM(1)), NUM(3)), ARR_LIT(ARR_LIT(NUM(1), NUM(2)), ARR_LIT(NUM(4), NUM(6))));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/array7.txt", "VAR(ARR(STR(x), NULL), NULL);",  catch_errors, &total, &passed, &failed, &errors);

	run_one("parser_tests/string_simple.txt", "ARR_LIT(NUM(97), NUM(98), NUM(99));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/string_newline.txt", "ARR_LIT(NUM(97), NUM(10), NUM(98));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/string_quote.txt", "ARR_LIT(NUM(97), NUM(34), NUM(98));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/string_backslash.txt", "ARR_LIT(NUM(97), NUM(92), NUM(98));", catch_errors, &total, &passed, &failed, &errors);
	run_one("parser_tests/string_tab.txt", "ARR_LIT(NUM(97), NUM(9), NUM(98));", catch_errors, &total, &passed, &failed, &errors);

	printf("\n===== Test Summary =====\n");
	printf("Total: %d\n", total);
	printf("Passed: %d\n", passed);
	printf("Failed (mismatches): %d\n", failed);
	printf("Runtime errors (caught): %d\n", errors);
	if (catch_errors) {
		printf("(Ran with --catch-errors; runtime errors were isolated per-test)\n");
	}
	printf("========================\n\n");
	return (failed > 0 || errors > 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}



static jmp_buf test_error_jmp;
static enum err_type caught_error = ERR_OK;

static void test_error_handler(enum err_type err_code) {
	caught_error = err_code;
	longjmp(test_error_jmp, 1);
}


bool test_error(const char *filename, enum err_type expected_err) {
	void (*old_handler)(enum err_type) = error_exit_handler;
	error_exit_handler = test_error_handler;
	caught_error = ERR_OK;
	
	if (setjmp(test_error_jmp) == 0)
		check_file_semantics((char *)filename);
	
	error_exit_handler = old_handler;
	
	bool passed = (caught_error == expected_err);
	
	if (!passed) {
		fprintf(stderr, "FAIL: %s\n", filename);
		if (expected_err == ERR_OK) {
		fprintf(stderr, "  Expected: No error\n");
		fprintf(stderr, "  Got:      Error code %d\n", caught_error);
		} else if (caught_error == ERR_OK) {
		fprintf(stderr, "  Expected: Error code %d\n", expected_err);
		fprintf(stderr, "  Got:      No error\n");
		} else {
		fprintf(stderr, "  Expected: Error code %d\n", expected_err);
		fprintf(stderr, "  Got:      Error code %d\n", caught_error);
		}
	} else {
		printf("PASS: %s\n", filename);
	}
	
	return passed;
}


// Parse error expectation from filename
// Format: test_ERR_NO_VAR.lang or test_valid.lang
static enum err_type parse_expected_error(const char *filename) {
	if (strstr(filename, "_valid") || strstr(filename, "_OK")) {
		return ERR_OK;
	}
	
	// Check for each error type in the filename
	if (strstr(filename, "_ERR_NO_FILE")) return ERR_NO_FILE;
	if (strstr(filename, "_ERR_EOF")) return ERR_EOF;
	if (strstr(filename, "_ERR_NO_ARGS")) return ERR_NO_ARGS;
	if (strstr(filename, "_ERR_NO_MEM")) return ERR_NO_MEM;
	if (strstr(filename, "_ERR_REFREE")) return ERR_REFREE;
	if (strstr(filename, "_ERR_UNEXP_CHAR")) return ERR_UNEXP_CHAR;
	if (strstr(filename, "_ERR_INV_ESC")) return ERR_INV_ESC;
	if (strstr(filename, "_ERR_UNMATCHED_BRACKET")) return ERR_UNMATCHED_BRACKET;
	if (strstr(filename, "_ERR_UNMATCHED_PAREN")) return ERR_UNMATCHED_PAREN;
	if (strstr(filename, "_ERR_UNMATCHED_BRACE")) return ERR_UNMATCHED_BRACE;
	if (strstr(filename, "_ERR_UNMATCHED_QUOTE")) return ERR_UNMATCHED_QUOTE;
	if (strstr(filename, "_ERR_BIG_NUM")) return ERR_BIG_NUM;
	if (strstr(filename, "_ERR_TOO_LONG")) return ERR_TOO_LONG;
	if (strstr(filename, "_ERR_INV_TYPE")) return ERR_INV_TYPE;
	if (strstr(filename, "_ERR_INV_VAL")) return ERR_INV_VAL;
	if (strstr(filename, "_ERR_INV_OP")) return ERR_INV_OP;
	if (strstr(filename, "_ERR_INV_EXP")) return ERR_INV_EXP;
	if (strstr(filename, "_ERR_INV_STMT")) return ERR_INV_STMT;
	if (strstr(filename, "_ERR_BAD_ELSE")) return ERR_BAD_ELSE;
	if (strstr(filename, "_ERR_REDEF")) return ERR_REDEF;
	if (strstr(filename, "_ERR_NO_VAR")) return ERR_NO_VAR;
	if (strstr(filename, "_ERR_IMMUT")) return ERR_IMMUT;
	if (strstr(filename, "_ERR_INV_ARR")) return ERR_INV_ARR;
	if (strstr(filename, "_ERR_INF_REC")) return ERR_INF_REC;
	if (strstr(filename, "_ERR_INTERNAL")) return ERR_INTERNAL;

	return ERR_OK;
}

int run_error_tests(const char *test_dir) {
    DIR *dir = opendir(test_dir);
    if (!dir) {
        fprintf(stderr, "Failed to open test directory: %s\n", test_dir);
        return -1;
    }
    
    int total = 0;
    int passed = 0;
    int failed = 0;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip directories and non-test files
        if (entry->d_type == DT_DIR)
            continue;
        
        const char *name = entry->d_name;
        
        // Skip hidden files and non-source files
        if (name[0] == '.')
            continue;
        
        // Build full path
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", test_dir, name);
        
        // Parse expected error from filename
        enum err_type expected = parse_expected_error(name);
        
        // Run test
        total++;
        if (test_error(filepath, expected)) {
            passed++;
        } else {
            failed++;
        }
    }
    
    closedir(dir);
    
    printf("\n");
    printf("=================================\n");
    printf("Test Results:\n");
    printf("  Total:  %d\n", total);
    printf("  Passed: %d\n", passed);
    printf("  Failed: %d\n", failed);
    printf("=================================\n");
    
    return failed;
}