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
	stmt_t *stmt = parse_file(input_file);

	int size = measure_stmt_strlen(stmt) + 1;
	char *act = calloc(size, sizeof(*act));
	if (!act) {
		free_stmt(stmt);
		fprintf(stderr, "memory allocation failed");
		exit(EXIT_FAILURE);
	}

	getStmtStr(act, stmt);
	int status = 0;
	if (strcmp(act, expected)) {
		fprintf(stderr, "input: \"%s\" mismatch\nact: \t\t\"%s\"\nexpected:\t\"%s\"\n\n",
				input_file, act, expected);
		status = 1;
	} else {
		printf("input: %s\n match\nact: \t\t\"%s\"\nexpected:\t\"%s\"\n\n",
			   input_file, act, expected);
		status = 0;
	}

	free(act);
	free_stmt(stmt);
	return status;
}

size_t measure_exp_strlen(const exp_t *exp) {
    if (!exp || (exp->type == EXP_EMPTY))
		return strlen("NULL");
    int size = 0;
    switch (exp->type) {
		case EXP_STR:
			size += strlen("STR()");
			size += strlen(exp->str);
			break;
		case EXP_ARRAY:
			size += strlen("ARR(, )");
			size += measure_exp_strlen(exp->arr.arr_name);
			size += measure_exp_strlen(exp->arr.index);
			break;
		case EXP_INDEX:
			size += strlen("INDEX(, )");
			size += measure_exp_strlen(exp->index.index);
			size += measure_exp_strlen(exp->index.next);
			break;
		case EXP_INITLIST:
			size += strlen("INITLIST()");
			size += measure_exp_strlen(exp->initlist);
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

size_t measure_stmt_strlen(const stmt_t *stmt) {
	if (!stmt)
		return strlen("NULL");
	int size = 0;
	switch (stmt->type) {
		case STMT_VAR:
		case STMT_VAL:
			size += strlen("VAR( , )");
			size += measure_exp_strlen(stmt->var.name);
			size += measure_exp_strlen(stmt->var.value);
			break;
		case STMT_EXPR:
			size += measure_exp_strlen(stmt->exp);
			break;
		case STMT_IF:
			size += strlen("IF( , ); ");
			size += measure_exp_strlen(stmt->ifStmt.cond);
			size += measure_stmt_strlen(stmt->ifStmt.thenStmt);
			if (stmt->ifStmt.elseStmt) {
			size += strlen(", ");
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

void getExpStr(char *out, const exp_t *exp) {
    if (!exp || !out) {
		sprintf(out, "NULL");
		return;
	}

    switch (exp->type) {
		case EXP_STR:
			sprintf(out, "STR(%s)", exp->str);
			break;
		case EXP_ARRAY:
			sprintf(out, "ARR(");
			getExpStr(out + strlen(out), exp->arr.arr_name);
			sprintf(out + strlen(out), ", ");
			getExpStr(out + strlen(out), exp->arr.index);
			sprintf(out + strlen(out), ")");
			break;
		case EXP_INDEX:
			sprintf(out, "INDEX(");
			getExpStr(out + strlen(out), exp->index.index);
			sprintf(out + strlen(out), ", ");
			getExpStr(out + strlen(out), exp->index.next);
			sprintf(out + strlen(out), ")");
			break;
		case EXP_INITLIST:
			sprintf(out, "INITLIST(");
			getExpStr(out + strlen(out), exp->initlist);
			sprintf(out + strlen(out), ")");
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
			getExpStr(out + strlen(out), exp->op.left);
			sprintf(out + strlen(out), ", %s, ", exp->op.op);
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

void getStmtStr(char *out, const stmt_t *stmt) {
	if (!stmt || !out) return;
	switch (stmt->type) {
		case STMT_EXPR:
			getExpStr(out, stmt->exp);
			break;
		case STMT_VAR:
		case STMT_VAL:
			char *name = (stmt->type == STMT_VAR) ? "VAR" : "VAL";
			sprintf(out, "%s(", name);
			getExpStr(out + strlen(out), stmt->var.name);
			sprintf(out + strlen(out), ", ");
			getExpStr(out + strlen(out), stmt->var.value);
			sprintf(out + strlen(out), ")");
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

