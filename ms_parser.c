#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms.h"
#include "bf.h"
#include "utils.h"


void free_exp(Exp *exp) {
    printf("asdf\n");
    if (exp == NULL) return; //recursion end
    switch (exp->type) {
	case EXP_NAME:
	    free(exp->name.value);
	    break;
	case EXP_ATOM:
	    break;
	case EXP_BINOP:
	    free(exp->binop.binop);
	    free_exp(exp->op.left);
	    free_exp(exp->op.right);
	    break;
	case EXP_OP:
	    free_exp(exp->op.left);
	    free_exp(exp->op.right);
	    break;
	case EXP_EMPTY:
	    break;
    }
    free(exp);
}

void print_full_exp(Exp *exp) {
    print_exp(exp);
    printf("\n");
}

void print_exp(Exp *exp) {
    if (exp == NULL) return; //recursion end
    switch (exp->type) {
	case EXP_NAME:
	    printf("%s ", exp->name.value);
	    break;
	case EXP_ATOM:
	    printf("%d ", exp->atom.value);
	    break;
	case EXP_OP:
	    printf("( ");
	    print_exp(exp->op.left);
	    printf("%c ", exp->op.op);
	    print_exp(exp->op.right);
	    printf(") ");
	    break;
	case EXP_BINOP:
	    printf("( ");
	    print_exp(exp->binop.left);
	    printf("%s ", exp->binop.binop);
	    print_exp(exp->binop.right);
	    printf(") ");
	case EXP_EMPTY:
	    break;
    }   
}

int getPrio(char op) {
    if (strchr("+-", op) != NULL)
	return 1;
    if (strchr("*/%", op) != NULL)
	return 2;
    return 0;
}

Exp *init_exp() {
    Exp *exp = (Exp *) malloc(sizeof(Exp));
    if (exp == NULL)
	raise_error("Error, Bad Malloc on exp");
    exp->type = EXP_EMPTY;
    return exp;
}

Exp *parse_exp(int minPrio, Reader *r) {
    //parsing atom
    Exp *left = NULL;

    Value *tok = peekToken(r);

    if (!tok) {
	return NULL;
	//raise_error("Unexpected end of input");
    }

    switch (tok->type) {
	case VAL_NUM:
	    tok = getToken(r);
	    left = init_exp();
	    left->type = EXP_ATOM;
	    left->atom.value = tok->num;
	    freeValue(tok);
	    break;
	case VAL_NAME:
	    tok = getToken(r);
	    left = init_exp();
	    left->type = EXP_NAME;
	    left->name.value = tok->str;
	    freeValue(tok);
	case VAL_DELIM:
	    if (tok->ch != '(')
		raise_error("unexpected delimiter in exp");

	    acceptToken(r, VAL_DELIM, "(");
	    left = parse_exp(0, r);
	    acceptToken(r, VAL_DELIM, ")");
	    break;
	default:
	    raise_error("Unexpected type");
    }

    while (isAlive(r)) {
	Value *op = peekToken(r);
	if (!op)
	    raise_error("Expected operator or binary operator");
	
	if (op->type == VAL_OP) {
	    int prio = getPrio(op->ch);
	    if (prio < minPrio)
		break;
	    op = getToken(r);
	    
	    Exp *exp = init_exp();
	    exp->type = EXP_OP;
	    exp->op.left = left;
	    exp->op.op = op->ch;
	    exp->op.right = parse_exp(prio+1, r);
	    left = (Exp *) exp;
	    freeValue(op);
	} else if (op->type == VAL_BINOP) {
	    op = getToken(r);

	    Exp *exp = init_exp();
	    exp->type = EXP_BINOP;
	    exp->binop.left = left;
	    exp->binop.binop = op->str;
	    exp->binop.right = parse_exp(minPrio, r);
	    left = (Exp *) exp;
	    freeValue(op);
	} else if (op->type == VAL_DELIM) {
	    if (op->ch != '(')
		raise_error("unexpected delim in place of operator");
	    int prio = getPrio('*');
	    if (prio < minPrio)
		break;

	    Exp *exp = init_exp();
	    exp->op.left = left;
	    exp->op.op = '*';
	    exp->op.right = parse_exp(prio + 1, r);
	    left = (Exp *) exp;
	} else if (op->type == VAL_NAME) {
	    int prio = getPrio('*');
	    if (prio < minPrio)
		break;

	    Exp *exp = init_exp();
	    exp->op.left = left;
	    exp->op.op = '*';
	    exp->op.right = parse_exp(prio + 1, r);
	    left = (Exp *) exp;
	} else {
	    raise_error("Operator is of wrong type");
	}	

    }
    return left;
}
    



Exp *parse_file(char *filename) {
    Reader *r = readInFile(filename);
    Exp *out = parse_exp(0, r);
    //printf("after parse_exp\n");
    killReader(r);
    return out;
}
