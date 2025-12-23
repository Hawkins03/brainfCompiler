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

void print_full_exp(const Exp *exp) {
    print_exp(exp);
    printf("\n");
}

void print_exp(const Exp *exp) {
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


Exp *init_exp() {
    Exp *exp = malloc(sizeof(*exp));
    if (exp == NULL)
	raise_error("Error, Bad Malloc on exp");
    exp->type = EXP_EMPTY;
    return exp;
}

Exp *init_op(Exp *left, char *op, Exp *right) {
    Exp *out = init_exp();
    out->type = EXP_OP;
    out->op.left = left;
    out->op.op = op;
    out->op.right = right;
    return out;
}

Exp *init_unary(Exp *left, char *op, Exp *right) {
    Exp *out = init_op(left, op, right);
    out->type = EXP_UNARY;
    return out;
}

Exp *init_num(int num) {
    Exp *out = init_exp();
    out->type = EXP_NUM;
    out->num= num;
    return out;
}

Exp *init_str(char *str) {    Exp *out = init_exp();
    out->type = EXP_STR;
    out->str = str;
    return out;
}


Exp *parse_unary(Value *tok, Exp *left, Reader *r) { //TODO: ensure recursion works properly
    if (!isUnaryOp(tok->str))
	return NULL;
    
    if (left->type == EXP_NUM) // unary operatives are unilaterally right associative
        return init_unary(left, stealTokString(tok), parse_atom(r));
    else {
        Exp *out = init_unary(NULL, stealTokString(tok), parse_atom(r));
	if (left) {
	    left->op.right = out;
	    out = left;
	}
	return out;
    }
    return NULL;
}

Exp *parse_char(Reader *r) {
    acceptToken(peekToken(r), VAL_DELIM, "'");
    freeValue(getToken(r));

    Value *next = peekToken(r);
    Exp *out;

    switch (next->type) { //TODO: rework
	case VAL_STR:
	case VAL_OP:
	    if ((!next->str) || (strlen(next->str) > 1))
		raise_error("expected single character");
	    
	    out = init_num(next->str[0]);
	    break;
	case VAL_NUM:
	    if (next->num >= 10)
		raise_error("expected single character");
	    
	    out = init_num(next->num + '0');
	    break;
	case VAL_DELIM:
	    if (next->type == '\'')
		return init_num(0);

	    out = init_num(next->ch);
	    break;
	default:
	    raise_error("invalid character type");
	    break;
    }
    freeValue(getToken(r)); // don't move to parse_atom

    acceptToken(peekToken(r), VAL_DELIM, "'");
    freeValue(getToken(r));

    return out;
}

Exp *parse_parenthesis(Reader *r) {
    //printf("parenthesis\n");
    acceptToken(peekToken(r), VAL_DELIM, "(");
    freeValue(getToken(r));

    Exp *out = parse_op(0, r);
    //printf("looking for closing parenthesis:\n");

    acceptToken(peekToken(r), VAL_DELIM, ")");
    freeValue(getToken(r));
    return out;
}

Exp *parse_atom(Reader *r) {
    Value *tok = peekToken(r);
    if (!tok)
	return NULL;

    Exp *exp = NULL;
    switch (tok->type) {
        case VAL_OP:
    	    exp = parse_unary(tok, NULL, r);
	    if (!exp)
		return NULL;

	    freeValue(getToken(r));
	    break;
    	case VAL_NUM:
	    exp = init_num(tok->num);

	    freeValue(getToken(r));
	    break;
	case VAL_STR:
	    exp = init_str(stealTokString(tok)); // helper fn makes ownership explicit
	    freeValue(getToken(r));
	    return exp;
	    break;
	case VAL_DELIM:
	    if (tok->ch == '\'') {
	        return parse_char(r);
	    } else if (tok->ch == '(') {
	    	return parse_parenthesis(r);
	    } else {
    		return NULL;
	    }
	case VAL_KEYWORD:
	case VAL_EMPTY:
	default:
	    return NULL;
	    break;
    }    
    return exp;
}

Exp *parse_op(int minPrio, Reader *r) {
    Exp *left = parse_atom(r);
    if (!left)
	return NULL;

    while (isAlive(r)) {
	//printVal(peekToken(r));
	Value *op = peekToken(r);
	if (!op)
	    break;

	if (op->type == VAL_OP) {
	    int prio = getPrio(op->str);
	    if ((prio < minPrio) && (prio != RIGHT_ASSOC_PRIO))
		break;

	    char *opStr = stealTokString(op);
	    freeValue(getToken(r));

	    left = init_op(left, opStr, parse_op(prio+1, r));
	} else if ((op->type == VAL_DELIM) && (op->ch == ';')) {
	    freeValue(getToken(r));
	    return left;
	} else {
	    //printf("op->type = %d", op->type);
	    break;
	}
    }
    return left;
}
    



Exp *parse_file(const char *filename) {
    Reader *r = readInFile(filename);
    Exp *out = parse_op(0, r);
    killReader(r);
    return out;
}
