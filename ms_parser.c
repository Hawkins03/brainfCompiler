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
	case EXP_NAME:
	    free(exp->name.value);
	    break;
	case EXP_ATOM:
	    break;
	case EXP_BINOP:
	    free(exp->binop.binop);
	    free_exp(exp->binop.left);
	    free_exp(exp->binop.right);
	    break;
	case EXP_OP:
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
	case EXP_NAME:
	    printf("'%s' ", exp->name.value);
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
	    break;
	case EXP_EMPTY:
	    break;
	default:
	    raise_error("unhandled expression type");
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
    Exp *exp = malloc(sizeof(*exp));
    if (exp == NULL)
	raise_error("Error, Bad Malloc on exp");
    exp->type = EXP_EMPTY;
    return exp;
}

Exp *get_empty_exp() {
    Exp *exp = init_exp();
    exp->type = EXP_ATOM;
    exp->atom.value = 0;
    return exp;
}


Exp *parse_exp(int minPrio, Reader *r) {
    //parsing atom
    Exp *left = NULL;

    Value *tok = peekToken(r);

    if (!tok) {
	return get_empty_exp();
	//raise_error("Unexpected end of input");
    }

    switch (tok->type) {
	case VAL_NUM:
	    tok = getToken(r);
	    left = init_exp();
	    left->type = EXP_ATOM;
	    left->atom.value = tok->num;
	    //printf("exp got: NUM(%d)\n", tok->num);
	    freeValue(tok);
	    break;
	case VAL_NAME:
	    tok = getToken(r);
	    //printf("exp got: NAME(%s)\n", tok->str);
	    left = init_exp();
	    left->type = EXP_NAME;
	    left->name.value = stealTokString(tok); // helper fn makes ownership explicit
	    freeValue(tok);
	    break;
	case VAL_DELIM:
	    //printf("exp got: DELIM('%c'/%hhu)\n", tok->ch, tok->ch);
	    if (tok->ch != '(') {
		raise_error("unexpected delimiter in exp");
	    }
	    //printf("parenthesis\n");
	    acceptToken(peekToken(r), "(");
    	    freeValue(getToken(r));
	    left = parse_exp(0, r);
	    //printf("looking for closing parenthesis:\n");
	    acceptToken(peekToken(r), ")");
	    freeValue(getToken(r));
	    break;
	default:
	    raise_error("Unexpected type");
    }

    while (isAlive(r)) {
	Value *op = peekToken(r);
	if (!op)
	    break; //empty token
    
	if (op->type == VAL_OP) {
	    int prio = getPrio(op->ch);
	    if (prio < minPrio)
		break;
	    op = getToken(r);

	    //printf("exp got: OP(%c)\n", op->ch);
	    Exp *exp = init_exp();
	    exp->type = EXP_OP;
	    exp->op.left = left;
	    exp->op.op = op->ch;
	    exp->op.right = parse_exp(prio+1, r);
	    left = (Exp *) exp;
    	    freeValue(op);
	} else if (op->type == VAL_BINOP) {
	    op = getToken(r);
	    if (!op)
		break;

	    //printf("exp got: BINOP(%s)\n", op->str);
	    Exp *exp = init_exp();
	    exp->type = EXP_BINOP;
	    exp->binop.left = left;
	    exp->binop.binop = op->str;
	    exp->binop.right = parse_exp(minPrio, r);
	    left = (Exp *) exp;
    	    freeValue(op);
	} else if (op->type == VAL_DELIM) {
	    if (op->ch == ')') {
		//printf("got to end of the parenthesis\n");
		break;
	    } if (op->ch != '(')
		raise_error("unexpected delim in place of operator");
	    int prio = getPrio('*');
	    if (prio < minPrio)
		break;

	    //printf("*parenthesis\n");
	    Exp *exp = init_exp();
	    exp->type = EXP_OP;
	    exp->op.left = left;
	    exp->op.op = '*';
	    exp->op.right = parse_exp(prio + 1, r);
	    left = (Exp *) exp;
	} else if (op->type == VAL_NAME) {
	    int prio = getPrio('*');
	    if (prio < minPrio)
		break;

	    Exp *exp = init_exp();
	    exp->type = EXP_OP;
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
