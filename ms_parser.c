#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms.h"
#include "bf.h"
#include "utils.h"


void free_exp(Exp *exp) {
    //printf("freeing expression\n");
    if (exp == NULL) return; //recursion end
    switch (exp->type) {
	case EXP_STR:
	    free(exp->str);
	    break;
	case EXP_NUM:
	    break;
	case EXP_OP:
	    free(exp->op.op);
	    free_exp(exp->op.left);
	    free_exp(exp->op.right);
	    break;
	case EXP_EMPTY:
	    break;
	default:
	    raise_error("Unhandled expression type");
    }
    free(exp);
}

void print_full_exp(Exp *exp) {
    print_exp(exp);
    printf("\n");
}

void print_exp(Exp *exp) {
    //printf("printing expression:, %p\n", exp);
    if (exp == NULL) return; //recursion end
    switch (exp->type) {
	case EXP_STR:
	    printf("NAME(%s) ", exp->str);
	    break;
	case EXP_NUM:
	    printf("NUM(%d) ", exp->num);
	    break;
	case EXP_OP:
	    printf("OP( ");
	    print_exp(exp->op.left);
	    printf(",%s, ", exp->op.op);
	    print_exp(exp->op.right);
	    printf(") ");
	    break;
	case EXP_EMPTY:
	    break;
	default:
	    raise_error("unhandled expression type");
    }   
}

int getPrio(char *op) {
    if (strchr("+-", op[0]) && (op[1] == '\0'))
	return 1;
    else if (strchr("*/%", op[0]) && (op[1] == '\0'))
	return 2;
    else
	return 3;
}

Exp *init_exp() {
    Exp *exp = malloc(sizeof(*exp));
    if (exp == NULL)
	raise_error("Error, Bad Malloc on exp");
    exp->type = EXP_EMPTY;
    return exp;
}

/*Exp *get_empty_exp() {
    Exp *exp = init_exp();
    exp->type = EXP_NUM;
    exp->num = 0;
    return exp;
}*/


Exp *parse_exp(int minPrio, Reader *r) {
    //parsing atom:
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
	    left->type = EXP_NUM;
	    left->num = tok->num;
	    //printf("exp got: NUM(%d)\n", tok->num);
	    freeValue(tok);
	    break;
	case VAL_STR:
	    tok = getToken(r);
	    //printf("exp got: STR(%s)\n", tok->str);
	    left = init_exp();
	    left->type = EXP_STR;
	    left->str = stealTokString(tok); // helper fn makes ownership explicit
	    freeValue(tok);
	    break;
	case VAL_DELIM:
	    //printf("exp got: DELIM('%c'/%hhu)\n", tok->ch, tok->ch);
	    if (tok->ch == '\'') {
		acceptToken(peekToken(r), VAL_DELIM, "'");
		freeValue(getToken(r));
		Value *tok = getToken(r);
		if ((tok->type != VAL_STR) || (!tok->str))
		    raise_error("unexpected token in character");
		if (strlen(tok->str) > 1)
		    raise_error("expected only one character");
		left = init_exp();
		left->num = tok->str[0];
		left->type = EXP_NUM;
		freeValue(tok);
		acceptToken(peekToken(r), VAL_DELIM, "'");
		freeValue(getToken(r));
	    } else if (tok->ch == '(') {
		//printf("parenthesis\n");
		acceptToken(peekToken(r), VAL_DELIM, "(");
		freeValue(getToken(r));
		left = parse_exp(0, r);
		//printf("looking for closing parenthesis:\n");
		acceptToken(peekToken(r), VAL_DELIM, ")");
		freeValue(getToken(r));
	    } else
		raise_error("unexpected delimiter in exp");
	    break;
	default:
	    raise_error("Unexpected type");
    }

    while (isAlive(r)) {
	Value *op = peekToken(r);
	if (!op)
	    break; //empty token
    
	if (op->type == VAL_OP) {
	    int prio = getPrio(op->str);
	    if (prio < minPrio)
		break;
	    op = getToken(r);

	    //printf("exp got: OP(%c)\n", op->ch);
	    Exp *exp = init_exp();
	    exp->type = EXP_OP;
	    exp->op.left = left;
	    exp->op.op = stealTokString(op);
	    exp->op.right = parse_exp(prio+1, r);
	    left = (Exp *) exp;
    	    freeValue(op);
    	} else if (op->type == VAL_DELIM) {
	    if (op->ch == ')') {
		//printf("got to end of the parenthesis\n");
		break;
	    } if (op->ch != '(')
		raise_error("unexpected delim in place of operator");
	    int prio = getPrio("*");
	    if (prio < minPrio)
		break;

	    //printf("*parenthesis\n");
	    Exp *exp = init_exp();
	    exp->type = EXP_OP;
	    exp->op.left = left;
	    exp->op.op = "*";
	    exp->op.right = parse_exp(prio + 1, r);
	    left = (Exp *) exp;
	} else if (op->type == VAL_STR) {
	    int prio = getPrio("*");
	    if (prio < minPrio)
		break;

	    Exp *exp = init_exp();
	    exp->type = EXP_OP;
	    exp->op.left = left;
	    exp->op.op = "*";
	    exp->op.right = parse_exp(prio + 1, r);
	    left = (Exp *) exp;
	} else {
	    raise_error("Operator is of wrong type");
	}	

    }
    return left;
}
    



Exp *parse_file(const char *filename) {
    Reader *r = readInFile(filename);
    Exp *out = parse_exp(0, r);
    //printf("after parse_exp\n");
    killReader(r);
    return out;
}
