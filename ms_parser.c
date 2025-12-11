#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms.h"
#include "bf.h"
#include "utils.h"

const char *MS_OPS = "+-*/%";


void free_atom(MS_Atom *atom) {
    if (atom == NULL) return;
    free_exp((MS_Exp *) atom->exp);
    atom->exp = NULL;
    free(atom);
}

void free_exp(MS_Exp *exp) {
    if (exp == NULL) return; //recursion end
    free_atom(exp->left);
    exp->left = NULL;
    free_atom(exp->right);
    exp->right = NULL;
    free(exp);
}

void print_full_exp(MS_Atom *atom) {
    print_atom(atom);
    printf("\n");
}

void print_atom(MS_Atom *atom) {
    if (atom == NULL) return;

    if (atom->val) {
	printf("%d ", atom->val);
    } else if (atom->exp) {
	printf("(");
	print_exp((MS_Exp *) atom->exp);
	printf(")");
    }
}

void print_exp(MS_Exp *exp) {
    if (exp == NULL) return; //recursion end
    
    print_atom(exp->left);	
    printf(" %c ", exp->op);
    print_atom(exp->right);
}

bool isOp(char op) {
    return strchr(MS_OPS, op);
}

int getPrio(char op) {
    switch (op) {
	case '+':
	case '-':
	    return 1;
	case '*':
	case '/':
	case '%':
	    return 2;
	default:
	    raise_error("error, getPrio encountered bad op");
	    exit(EXIT_FAILURE);
    }
}

MS_Atom *init_atom() {
    MS_Atom *atom = (MS_Atom *) malloc(sizeof(MS_Atom));
    if (atom == NULL)
	raise_error("Error, bad malloc on atom.");
    atom->val = 0;
    atom->exp = NULL;
    return atom;
}

MS_Exp *init_exp() {
    MS_Exp *exp = (MS_Exp *) malloc(sizeof(MS_Exp));	
    if (exp == NULL)
	raise_error("Error, Bad Malloc on exp");
    exp->op = ' ';
    exp->left = NULL;
    exp->right= NULL;
    exp->prev = NULL;
    return exp;
}

MS_Atom *parse_atom(Reader *r) {
    skip_spaces(r);
    if (!isAlive(r))
	return NULL;
    if (peek(r) == '(') {
	//printf("parsing parenthesis\n");
	accept('(', advance(r), "Error, parse_atom expected '('");
	MS_Atom *atom = parse_exp(0, r);
	accept(')', advance(r), "Error, parse_atom expected ')'");
	return atom;
    } else if (isdigit(peek(r))) {
	MS_Atom *atom = init_atom();
	atom->val = getNextNum(r);
	//printf("parsing val %d\n", atom->val);
	return atom;
    } else {
	raise_error("Error, parse_atom did not start with parenthesis or a digit");
	exit(EXIT_FAILURE);
    }
}

MS_Atom *parse_exp(int minPrio, Reader *r) {
    if (!isAlive(r))
	raise_error("Error, bad reader in parse_exp");

    //printf("parsing expression, minPrio = %d\n", minPrio);

    MS_Atom *left = parse_atom(r);
    //printf("left_val: '");
    //print_atom(left);
    //printf("'\n");

    if (!left)
	return NULL;

    while (isAlive(r)) {
	//printf("loop:\n");
	skip_spaces(r);
	char op = peek(r);
	//printf("op = %c\n", op);

	if (!isOp(op) && op != '(')
	    break;

	//printf("prio = %d\n", getPrio(op));
    

	int prio;
	if (op == '(')
	    prio = getPrio('*');
	else
	    prio = getPrio(op);


	if (prio < minPrio)
	    break;

    	if (op != '(')
	    advance(r); //op

	MS_Exp *exp = init_exp();
	
	exp->left = left;
	exp->op = (op=='(') ? op :'*';
	exp->right = parse_exp(minPrio+1, r);

	left = init_atom();
	left->exp = (struct MS_Exp *) exp;
	//printf("endloop, left = '");
	//print_atom(left);
	//printf("'\n");
    }
    return left;
}
    
MS_Atom *parse_file(char *filename) {
    Reader *r = readInFile(filename);
    MS_Atom *out = parse_exp(0, r);
    return out;
}
