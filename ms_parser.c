#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms.h"
#include "bf.h"
#include "utils.h"

const char *MS_OPS = "+-*/%";

void free_exp(MS_Exp *exp) {
    if (exp == NULL) return; //recursion end
    free_exp(exp->next);
    free(exp);
}

void print_exp(MS_Exp *exp) {
    if (exp == NULL) return; //recursion end
    if (exp == exp->next) {
	raise_error("Error, Infinite Loop");
    }
    printf("%d '%c'\n", exp->val, exp->op);
    print_exp(exp->next);
}

MS_Exp *init_exp() {
    MS_Exp *exp = (MS_Exp *) malloc(sizeof(MS_Exp));	
    if (exp == NULL) {
	raise_error("Error, Bad Malloc");
    }
    exp->val = 0;
    exp->op = ' ';
    exp->next = 0;
    return exp;
}

MS_Exp *parse_file(char *filename) {
    Reader *r = readInFile(filename);
    MS_Exp *ret_exp = init_exp();
    MS_Exp *curr_exp = ret_exp;

    while ((r != NULL) && (curr_exp != NULL)) {
	if (isdigit(peek(r))) {
	    curr_exp->val = getNextNum(r);
	} else if (isOp(peek(r))) {
	    curr_exp->op = advance(r);
	    curr_exp->next = init_exp();
	    curr_exp = curr_exp->next;
	    if (curr_exp == NULL) raise_error_and_free("Error, Bad Malloc", r);
	} else if (isAlive(r)) {
	    advance(r);
	}
	if (!isAlive(r)) r = NULL;
    }
    r = NULL;
    return ret_exp;
}
