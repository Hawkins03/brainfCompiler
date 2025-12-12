#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms.h"
#include "bf.h"
#include "utils.h"


const char *MS_SYMBOLS = "+-*/%&|=!()[]{}";
const char *MS_OPSYM = "+-*/%!=<>";
const char *MS_OPS[] = {"+", "-", "*", "/", "%"};
const char *MS_BINOPS[] = {"!", "==", "!=", "<", "<=", ">", ">="};
const char *MS_KEYWORDS[] = {"let", "var", "while", "for", "if"};
const int NUM_OPS = 5;
const int NUM_BINOPS = 7;
const int NUM_KEYWORDS = 5;

void free_exp(Exp *exp) {
    if (exp == NULL) return; //recursion end
    switch (exp->type) {
	case EXP_NAME:
	    free(exp->name.value);
	    break;
	case EXP_ATOM:
	    break;
	case EXP_OP:
	case EXP_BINOP:
	    free_exp(exp->op.left);
	    free_exp(exp->op.right);
	    free(exp->op.op);
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
	case EXP_BINOP:
	    printf("(");
	    print_exp(exp->op.left);
	    printf("%s ", exp->op.op);
	    print_exp(exp->op.right);
	    printf(") ");
	    break;
	case EXP_EMPTY:
	    break;
    }   
}

bool isOp(char *op) {
    for (int i = 0; i < NUM_OPS; i++)
	if (!strcmp(MS_OPS[i], op))
	    return true;
    return false;
}

bool isBinOp(char *op) {
    for (int i = 0; i < NUM_BINOPS; i++)
	if (!strcmp(MS_BINOPS[i], op))
	    return true;
    return false;
}

int getPrio(char *op) {
    if (!strcmp(op, "+") || !strcmp(op, "-"))
	return 1;
    if (!strcmp(op, "*") || !strcmp(op, "/") || !strcmp(op, "%") || !strcmp(op, "("))
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
    if (!isAlive(r))
	raise_error("Error, bad reader in parse_exp");

    //printf("parsing expression, minPrio = %d\n", minPrio);

    skip_spaces(r);

    //parsing atom
    Exp *left;
    if (peek(r) == '(') {
	//printf("parsing parenthesis\n");
	accept('(', advance(r), "Error, parse_atom expected '('");
	left = parse_exp(0, r);
	accept(')', advance(r), "Error, parse_atom expected ')'");
    } else if (isdigit(peek(r))) {
	left = init_exp();
	left->type = EXP_ATOM;
	left->atom.value = getNextNum(r);
	//printf("parsing val %d\n", exp->atom.value);
    } else if (isalpha(peek(r))) {
	char *word = getNextWord(r);
	//printf("word: '%s' ", word);
	for (int i = 0; i < NUM_KEYWORDS; i++)
	    if (!strcmp(MS_KEYWORDS[i], word))
		raise_error("ERROR, variable name can't be keyword in parse_atom");
	left = init_exp();
	left->type = EXP_NAME;
	left->name.value = word;
    } else {
	//printf("%c\n", peek(r));
	raise_error("Error, parse_atom encountered erronious value");
    }
    
    if (!isAlive(r))
	return left;

    if (!left)
	return NULL;

    while (isAlive(r)) {
	if (!isAlive(r))
	    return left;
    	skip_spaces(r);
	if (!isAlive(r))
	    return left;

	//printf("loop:\n");
	
	//1. init op.
	char *op = (char *) calloc(3, sizeof(char));
	if (!op)
	    raise_error("Error, bad op malloc");
	
	//2. peek at op
	op[0] = peek(r);

	//check the prio
	int prio = getPrio(op);
	if (prio < minPrio)
	    break;

	//collect the op
	for (int i = 0; i < 2; i++)
	    if (strchr(MS_OPSYM, peek(r)))
		op[i] = advance(r);
	
	if (!isOp(op) && !isBinOp(op) && (strcmp(op, "(")))
	    break;

	//printf("op: %s, peek: %c\n, %d, %d, %d", op, peek(r), isOp(op), isBinOp(op), (strcmp(op, "(")));
	Exp *exp = init_exp();

	//3. set exp type
	if (isOp(op))
	    exp->type = EXP_OP;
	else if (isBinOp(op))
	    exp->type = EXP_BINOP;
	else {
	    free_exp(exp);
	    raise_error("unknown symbol in operator");
	    break;
	}
	
    	exp->op.left = left;
	exp->op.op = (strcmp(op, "(")) ? op :"*";
	//printf("op: %s\n", op);
	exp->op.right = parse_exp(minPrio+1, r);
	left = (Exp *) exp; 
	if (!isAlive(r))
	    return left;
	skip_spaces(r);
	if (!isAlive(r))
	    return left;

	//printf("endloop, left = '");
	//print_atom(left);
	//printf("'\n");
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
