#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "ms.h"
#include "utils.h"
#include "test.h"

void test_file(const char *input_file, const char *expected) {
    Exp *exp = parse_file(input_file);
    int size = measure_exp_strlen(exp);
    char *act = calloc(size, sizeof(*act));
    getExpStr(act, exp);
    if (strcmp(act, expected))
	fprintf(stderr, "input: \"%s\" mismatch\nact: \t\t\"%s\"\nexpected:\t\"%s\"\n\n", input_file, act, expected);
    else
	printf("input: %s\n match\nact: \t\t\"%s\"\nexpected:\t\"%s\"\n\n", input_file, act, expected);
}

size_t measure_exp_strlen(const Exp *exp) {
    if (!exp || (exp->type == EXP_EMPTY))
	return 0;
    int size = 0;
    switch (exp->type) {
	//STR, NUM, OP, BINOP
	case (EXP_STR):
	    size += strlen("STR()");
	    size += strlen(exp->str);
	    break;
	case (EXP_NUM):
	    size += strlen("NUM()");
	    char buff[11] = {0};
	    sprintf(buff, "%d", exp->num);
	    size += strlen(buff);
	    break;
	case (EXP_OP):
	    size += measure_exp_strlen(exp->op.left);
	    size += strlen("OP(, +, )");
	    size += measure_exp_strlen(exp->op.right);
	    break;
	case (EXP_BINOP):
	    size += measure_exp_strlen(exp->binop.left);
	    size += strlen("BINOP(, ++, )");
	    size += measure_exp_strlen(exp->binop.right);
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
	    sprintf(out + strlen(out), ", %c, ", exp->op.op);
	    getExpStr(out + strlen(out), exp->op.right);
	    sprintf(out + strlen(out), ")");
	    break;
	case EXP_BINOP:
	    sprintf(out, "BINOP(");
	    getExpStr(out + strlen(out), exp->binop.left);
	    sprintf(out + strlen(out), ", %s, ", exp->binop.binop);
	    getExpStr(out + strlen(out), exp->binop.right);
	    sprintf(out + strlen(out), ")");
	    break;
	default: 
	    break;

    }
}

