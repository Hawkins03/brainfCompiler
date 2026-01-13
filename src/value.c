#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "value.h"
#include "reader.h"
#include "structs.h"

const char *OPS[][12] = {         					// lowest priority to highest priority:
    {"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|=", NULL},	// assignment
	{",", NULL},
	{"||", NULL},                       					// logical or
    {"&&", NULL},                       					// logical and
    {"|", NULL},                        					// bitwiase or
    {"^", NULL},                        					// bitwise xor
    {"&", NULL},                        					// bitwise and
    {"==", "!=", NULL},                 					// equivalence operators
    {"<", ">", "<=", ">=", NULL},       					// relational operators
    {"<<", ">>"},                       					// bitwise shifts
    {"+", "-", NULL},                   					// addition / subtraction
    {"*", "/", "%", NULL},              					// multiplication, division, modulo
    {"!", "~", "++", "--", NULL}							// unary
};


value_t *initValue() {
    return calloc(1, sizeof(value_t));
}

void print_empty_value(const value_t *tok) {
    if (!tok || (tok->type != VAL_EMPTY))
        return;
    printf("EMPTY()\n");
}

void print_keyword_value(const value_t *tok) {
    if (!tok || (tok->type != VAL_KEYWORD))
        return;
    printf("KEYWORD(%s)\n", getKeyStr(tok->key));
}

void print_name_value(const value_t *tok) {
    if (!tok || (tok->type != VAL_NAME))
        return;
    printf("NAME(%s)\n", tok->str);
}

void print_str_value(const value_t *tok) {
    if (!tok || (tok->type != VAL_STR))
        return;
    printf("STR(%s)\n", tok->str);
}

void print_op_value(const value_t *tok) {
    if (!tok || (tok->type != VAL_OP))
        return;
    printf("OP(%s)\n", tok->str);
}

void print_delim_value(const value_t *tok) {
    if (!tok || (tok->type != VAL_DELIM))
        return;
    printf("DELIM(%c)\n", tok->ch);
}

void print_num_value(const value_t *tok) {
    if (!tok || (tok->type != VAL_NUM))
        return;
    printf("NUM(%d)\n", tok->num);
}


void print_value(const value_t *tok) {
    if (!tok)
		printf("NULL()\n");
    tok->print_fn(tok);
}

void free_value(value_t *val) {
	if ((isStrType(val)) && (val->str != NULL))
		free(val->str);
	free(val);
}




bool isWordChar(const int ch) {
    return ((ch != EOF) && (isalnum(ch) || (ch == '_')));
}

int get_prio(const char *op) {
	for (int y = 0; y < NUM_PRIOS; y++)
		for (int x = 0; OPS[y][x] != NULL; x++)
			if (!strcmp(OPS[y][x], op))
				return y;
	return -1;
}

bool isOp(const char *op) {
	return get_prio(op) > -1; 
}

bool isRightAssoc(const int prio) {
	//technically unary ops are right assoc, but they shouldn't show up in the normal op check
	return (prio == SET_OP_PRIO);
}

bool matchesOp(const int op) { // NOT !strchr(). That causes an error
    return (op != EOF) && strchr(OP_START, op);
}

bool isUnaryOp(const char *op) {
    int prio = get_prio(op);
    return ((prio == ADD_OP_PRIO) || (prio == UNARY_OP_PRIO));
}

bool isSuffixOp(value_t *tok) {
    if (!tok || (tok->type != VAL_OP) || !(tok->str))
		return false;

	for (int i = 0; i < SUFFIX_OPS_LEN; i++)
		if (!strcmp(SUFFIX_OPS[i], tok->str))
			return true;

    return false;
}

bool isValidKey(const key_t key) {
	return ((key >= KW_VAR) && (key <= KW_FALSE));
}

bool isDelim(const int delim) {
    return ((delim != EOF) && (strchr(DELIMS, delim) != NULL));
}

bool isStrType(value_t *v) {
    return v && ((v->type == VAL_NAME) || (v->type == VAL_OP));
}