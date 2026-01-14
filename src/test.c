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
#include "stmt.h"
#include "exp.h"

int test_file(const char *input_file, const char *expected) {
	struct stmt *stmt = parse_file(input_file);

	if (stmt->next == stmt) {
		free_stmt(stmt);
		raise_error("infinite recursion of statement");
	}
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
		status = 1;
	}

	free(act);
	free_stmt(stmt);
	return status;
}

size_t measure_exp_strlen(const struct exp  *exp) {
    if (!exp || (exp->type == EXP_EMPTY))
		return strlen("NULL");
    int size = 0;
    switch (exp->type) {
	case EXP_NAME:
		size += strlen("STR()");
		size += strlen(exp->name);
		break;
	case EXP_ARRAY:
		size += strlen("ARR(, )");
		size += measure_exp_strlen(exp->array.name);
		size += measure_exp_strlen(exp->array.index);
		break;
	case EXP_RIGHTARRAY:
		size += strlen("Nested()");
		for (int i = 0; i < exp->right_array.size - 1; i++) {
			size += measure_exp_strlen(exp->right_array.array + i);
			size += strlen(", ");
		}
		size += measure_exp_strlen(exp->right_array.array) + exp->right_array.size - 1;
		
		break;
	case EXP_NUM:
		size += strlen("NUM()");
		char *buf1 = calloc(11, sizeof(*buf1));
		sprintf(buf1, "%d", exp->num);
		size += strlen(buf1);
		free(buf1);
		break;
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
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
		char *buf2 = calloc(11, sizeof(*buf2));
		sprintf(buf2, "%d", exp->call.key);
		size += strlen(buf2);
		free(buf2);
		size += measure_exp_strlen(exp->call.call);
		break;
	default:
		break;
    	}
    	return size;
}

size_t measure_stmt_strlen(const struct stmt *stmt) {
	if (!stmt)
		return strlen("NULL");
	int size = 0;
	switch (stmt->type) {
	case STMT_VAR:
	case STMT_VAL:
		size += strlen("VAR( , );");
		size += measure_exp_strlen(stmt->var.name);
		size += measure_exp_strlen(stmt->var.value);
		break;
	case STMT_EXPR:
		size += measure_exp_strlen(stmt->exp) + 1;
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
	if (!stmt->next)
		return size;
	return size + ' ' + measure_stmt_strlen(stmt->next);
}

void getExpStr(char *out, const struct exp  *exp) {
    if (!exp || !out) {
		sprintf(out, "NULL");
		return;
	}

    	switch (exp->type) {
	case EXP_NAME:
		sprintf(out, "STR(%s)", exp->name);
		break;
	case EXP_ARRAY:
		sprintf(out, "ARR(");
		getExpStr(out + strlen(out), exp->array.name);
		sprintf(out + strlen(out), ", ");
		getExpStr(out + strlen(out), exp->array.index);
		sprintf(out + strlen(out), ")");
		break;
	case EXP_RIGHTARRAY:
		sprintf(out, "NESTED(");
		for (int i = 0; i < exp->right_array.size - 1; i++) {
			getExpStr(out + strlen(out), exp->right_array.array + i);
			sprintf(out + strlen(out), ", ");
		}
		getExpStr(out + strlen(out), exp->right_array.array + exp->right_array.size - 1);
		sprintf(out + strlen(out), ")");
		break;
	case EXP_NUM:
		sprintf(out, "NUM(%d)", exp->num);
		break;
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
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

void getStmtStr(char *out, const struct stmt *stmt) {
	if (!stmt || !out)
		return;
	switch (stmt->type) {
	case STMT_EXPR:
		getExpStr(out, stmt->exp);
		sprintf(out + strlen(out), ";");
		break;
	case STMT_VAR:
	case STMT_VAL:
		char *name = (stmt->type == STMT_VAR) ? "VAR" : "VAL";
		sprintf(out, "%s(", name);
		getExpStr(out + strlen(out), stmt->var.name);
		sprintf(out + strlen(out), ", ");
		getExpStr(out + strlen(out), stmt->var.value);
		sprintf(out + strlen(out), ");");
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
		sprintf(out + strlen(out), ");");
		break;
	case STMT_LOOP:
		sprintf(out, "LOOP(");
		getExpStr(out + strlen(out), stmt->loop.cond);
		sprintf(out + strlen(out), ", ");
		getStmtStr(out + strlen(out), stmt->loop.body);
		sprintf(out + strlen(out), ");");
		break;
	default:
		break;
	}
	if (stmt->next) {
		sprintf(out + strlen(out), " ");
		getStmtStr(out + strlen(out), stmt->next);
	}
}

