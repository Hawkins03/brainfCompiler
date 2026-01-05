#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"
#include "parser.h"
#include "utils.h"
#include "test.h"

int test_file(const char *input_file, const char *expected) {
	Stmt *stmt = parse_file(input_file);

	int size = measure_stmt_strlen(stmt) + 1;
	char *act = calloc(size, sizeof(*act));
	if (!act) {
		free_stmt(stmt);
		raise_error("memory allocation failed");
	}

	getStmtStr(act, stmt);
	int status = 0;
	if (strcmp(act, expected)) {
		fprintf(stderr, "input: \"%s\" mismatch\nact: \t\t\"%s\"\nexpected:\t\"%s\"\n\n",
				input_file, act, expected);
		status = 1; // mismatch
	} else {
		printf("input: %s\n match\nact: \t\t\"%s\"\nexpected:\t\"%s\"\n\n",
			   input_file, act, expected);
		status = 0; // success
	}

	free(act);
	free_stmt(stmt);
	return status;
}

size_t measure_exp_strlen(const Exp *exp) {
    if (!exp || (exp->type == EXP_EMPTY))
		return strlen("NULL");
    int size = 0;
    switch (exp->type) {
		case EXP_STR:
			size += strlen("STR()");
			size += strlen(exp->str);
			break;
		case EXP_NUM:
			size += strlen("NUM()");
			char buff1[11] = {0};
			sprintf(buff1, "%d", exp->num);
			size += strlen(buff1);
			break;
		case EXP_OP:
			size += measure_exp_strlen(exp->op.left);
			size += strlen("OP(, <<=, )");
			size += measure_exp_strlen(exp->op.right);
			break;
		case EXP_UNARY:
			size += measure_exp_strlen(exp->op.left);
			size += strlen("UNARY(, ++, )");
			size += measure_exp_strlen(exp->op.right);
			break;
		case EXP_CALL:
			size += strlen("CALL(, )");	
			char buff2[11] = {0};
			sprintf(buff2, "%d", exp->call.key);
			size += strlen(buff2);
			size += measure_exp_strlen(exp->call.call);
			break;
		default:
			break;
    }
    return size;
}

size_t measure_stmt_strlen(const Stmt *stmt) {
	if (!stmt)
		return strlen("NULL");
	int size = 0;
	switch (stmt->type) {
		case STMT_EXPR:
			size += measure_exp_strlen(stmt->exp);
			break;
		case STMT_IF:
			size += strlen("IF( , ); ");
			size += measure_exp_strlen(stmt->ifStmt.cond);
			size += measure_stmt_strlen(stmt->ifStmt.thenStmt);
			if (stmt->ifStmt.elseStmt) {
			size += 2; // for an extra , and space
			size += measure_stmt_strlen(stmt->ifStmt.elseStmt);
			}
			break;
		case STMT_LOOP:
			size += strlen("LOOP( , ); ");
			size += measure_exp_strlen(stmt->loop.cond);
			size += measure_stmt_strlen(stmt->loop.body);
			break;
		default:
			break;
	}
	return size;
}

void getExpStr(char *out, const Exp *exp) {
    if (!exp || !out) return;
    switch (exp->type) {
	case EXP_STR:
	    sprintf(out, "STR(%s)", exp->str);
	    break;
	case EXP_NUM:
	    sprintf(out, "NUM(%d)", exp->num);
	    break;
	case EXP_OP:
	    sprintf(out, "OP(");
	    getExpStr(out + strlen(out), exp->op.left);
	    sprintf(out + strlen(out), ", %s, ", exp->op.op);
	    getExpStr(out + strlen(out), exp->op.right);
	    sprintf(out + strlen(out), ")");
	    break;
	case EXP_UNARY:
	    sprintf(out, "UNARY(");
	    if (!exp->op.left)
		sprintf(out + strlen(out), "NULL");
	    getExpStr(out + strlen(out), exp->op.left);
	    sprintf(out + strlen(out), ", %s, ", exp->op.op);
	    if (!exp->op.right)
		sprintf(out + strlen(out), "NULL");
	    getExpStr(out + strlen(out), exp->op.right);
	    sprintf(out + strlen(out), ")");
	    break;
	case EXP_CALL:
	    sprintf(out, "CALL(%d, ", exp->call.key);
	    getExpStr(out + strlen(out), exp->call.call);
	    sprintf(out + strlen(out), ")");
	    break;
	default: 
	    break;

    }
}

void getStmtStr(char *out, const Stmt *stmt) {
	if (!stmt || !out) return;
	switch (stmt->type) {
		case STMT_EXPR:
			getExpStr(out, stmt->exp);
			break;
		case STMT_IF:
			sprintf(out, "IF(");
			getExpStr(out + strlen(out), stmt->ifStmt.cond);
			sprintf(out + strlen(out), ", ");
			getStmtStr(out + strlen(out), stmt->ifStmt.thenStmt);
			if (stmt->ifStmt.elseStmt) {
				sprintf(out + strlen(out), ", ");
				getStmtStr(out + strlen(out), stmt->ifStmt.elseStmt);
			}
			sprintf(out + strlen(out), "); ");
			break;
		case STMT_LOOP:
			sprintf(out, "LOOP(");
			getExpStr(out + strlen(out), stmt->loop.cond);
			sprintf(out + strlen(out), ", ");
			getStmtStr(out + strlen(out), stmt->loop.body);
			sprintf(out + strlen(out), "); ");
			break;
		default:
			break;
	}
}

