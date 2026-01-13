#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "value.h"
#include "reader.h"
#include "structs.h"
#include "utils.h"

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


//checker functions
bool isWordChar(const int ch)
{
    return ((ch != EOF) && (isalnum(ch) || (ch == '_')));
}

int getPrio(const char *op)
{
	for (int y = 0; y < NUM_PRIOS; y++) {
		for (int x = 0; OPS[y][x] != NULL; x++) {
			if (!strcmp(OPS[y][x], op))
				return y;
        }
    }
	return -1;
}

bool isOp(const char *op)
{
	return getPrio(op) > -1; 
}

bool isRightAssoc(const int prio)
{
	//technically unary ops are right assoc, but they shouldn't show up in the normal op check
	return (prio == SET_OP_PRIO);
}

bool matchesOp(const int op)
{
    return (op != EOF) && strchr(OP_START, op);
}

bool isUnaryOp(const char *op)
{
    int prio = getPrio(op);
    return ((prio == ADD_OP_PRIO) || (prio == UNARY_OP_PRIO));
}

bool isSuffixOp(struct value *tok)
{
    if (!tok || (tok->type != VAL_OP) || !(tok->str))
		return false;

	for (int i = 0; i < SUFFIX_OPS_LEN; i++)
		if (!strcmp(SUFFIX_OPS[i], tok->str))
			return true;

    return false;
}

bool isKeyword(const enum key_type  key)
{
	return ((key >= KW_VAR) && (key <= KW_FALSE));
}

bool isTrueFalseKey(const enum key_type  key)
{
    return ((key == KW_TRUE) || (key == KW_FALSE));
}

bool isDelim(const int delim)
{
    return ((delim != EOF) && (strchr(DELIMS, delim) != NULL));
}

bool isStrType(struct value *v)
{
    return v && ((v->type == VAL_NAME) || (v->type == VAL_OP));
}

bool isValidOp(const struct value *op, const int minPrio)
{
    if ((op == NULL) || (op->type != VAL_OP) || !op->str)
        return false;

    int prio = getPrio(op->str);
    return (((minPrio >= 0) && (prio >= minPrio) && (prio != UNARY_OP_PRIO)) || (prio == SET_OP_PRIO));
}

struct value *initValue()
{
    return calloc(1, sizeof(struct value));
}

struct value *initKeywordValue(enum key_type  key, struct reader *r)
{
    struct value *val = initValue();
    if (!val)
        raise_syntax_error("failed to allocate value", r);

    if (isTrueFalseKey(key)) {
        val->type = VAL_NUM;
        val->num = (key == KW_TRUE);
    } else {
        val->type = VAL_KEYWORD;
        val->key = key;
    }
    return val;
}

struct value *initNameValue(char *str, struct reader *r)
{
    enum key_type  key = getKeyType(str);
    if (isKeyword(key)) {
        free(str);
        return initKeywordValue(key, r);
    }

    struct value *val = initValue();
    if (!val)
        raise_syntax_error("failed to initialize value", r);

    val->type = VAL_NAME;
    val->str = str;
    return val;
}

struct value *initOpValue(char *op, struct reader *r)
{
    struct value *val = initValue();
    if (!val)
        raise_syntax_error("failed to initialize value", r);

    val->type = VAL_OP;
	val->str = op;  
    return val;  
}

struct value *initDelimValue(char delim, struct reader *r) {
    struct value *val = initValue();
    if (!val)
        raise_syntax_error("failed to initialize value", r);


    switch (delim) {
    case '\'':
        val->num = getCharacterValue(r);
        val->type = VAL_NUM;
        return val;
    case '"':
        val->str = getNextString(r);
        val->type = VAL_STR;
        break;
    default:
        val->type = VAL_DELIM;
        val->ch = delim;
        break;
    }
    return val;
}

struct value *initNumValue(int num, struct reader *r) {
    struct value *val = initValue();
    if (!val)
        raise_syntax_error("failed to initialize value", r);

    val->type = VAL_NUM;
	val->num = num;
    return val;
}

void freeValue(struct value *val)
{
    if (!val)
        return;
	else if ((isStrType(val)) && (val->str != NULL))
		free(val->str);
	free(val);
}

//fetching functions
struct value *getRawValue(struct reader *r)
{
    char ch = peek(r);
    if (ch == EOF || !readerIsAlive(r))
		return NULL;

    if (isalpha(ch))
		return initNameValue(getNextWord(r), r);
    else if (matchesOp(ch))
		return initOpValue(getNextOp(r), r);
    else if (isDelim(ch))
		return initDelimValue(getNextDelim(r), r);
    else if (isdigit(peek(r)))
		return initNumValue(getNextNum(r), r);
	else
		raise_syntax_error("Error, unexpected character", r);
    return NULL;
}

struct value *getValue(struct reader *r)
{
    if (!readerIsAlive(r))
		return NULL;

    skip_spaces(r);

    struct value *out = r->curr_token;
    r->curr_token = getRawValue(r);
    return out;
}

struct value *peekValue(struct reader *r)
{
    if (!readerIsAlive(r))
		return NULL;
    return r->curr_token;
}

void acceptValue(struct reader *r, enum value_type type, const char *expected)
{
	struct value *tok = getValue(r);
	if (!tok) {
		raise_syntax_error("Invalid Null token value", r);
	} else if (!expected) {
		freeValue(tok);
		raise_syntax_error("Invalid expected value", r);
	} else if (tok->type != type) {
		freeValue(tok);
		raise_syntax_error("Unexpected token type", r);
	}
	
	if (tok->type == VAL_KEYWORD) {
		if (strcmp(getKeyStr(tok->key), expected)) {
			freeValue(tok);
			raise_syntax_error("Unexpected keyword value", r);
		}
    } else if (isStrType(tok)) {
		if (!tok->str || strcmp(tok->str, expected)) {
			freeValue(tok);
			raise_syntax_error("Missing token string", r);
		}
    } else if (tok->type == VAL_DELIM) {
		if (tok->ch != expected[0]) {
			freeValue(tok);
			raise_syntax_error("Unexpected token value", r);
		}
	}
	freeValue(tok);
}
