#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "value.h"
#include "reader.h"
#include "structs.h"
#include "utils.h"

bool isOpType(const struct value *v) {
	return v && (v->type == VAL_OP) && v->op != OP_UNKNOWN;
}

bool isSuffixVal(const struct value *v) {
	return isOpType(v) && is_suffix_unary(v->op);
}

bool isPrefixVal(struct value *v) {
	return isOpType(v) && is_prefix_unary(v->op);
}

bool isBinaryOpVal(struct value *v) {
	return isOpType(v) && isBinaryOp(v->op);
}

bool isAssignOpVal(struct value *v) {
	return isOpType(v) && isAssignOp(v->op);
}

bool isValidOp(const struct value *v, const int minPrio) {
	if (isAssignOp(v->op))
		return true;
	if (!isBinaryOp(v->op))
		return false;
	return getPrio(v->op) >= minPrio;
}

//checker functions
bool isWordChar(const int ch) {
    	return ((ch != EOF) && (isalnum(ch) || (ch == '_')));
}

bool isKeyword(const enum key_type  key)
{
	return ((key >= KW_VAR) && (key <= KW_FALSE));
}

bool isElseKey(const struct value *val) {
	bool stmt1 = (val != NULL);
	bool stmt2 = (val->type == VAL_KEYWORD);
	bool stmt3 = (val->num == KW_ELSE);
	return stmt1 && stmt2 && stmt3;
}

bool isTrueFalseKey(const enum key_type  key)
{
    	return ((key == KW_TRUE) || (key == KW_FALSE));
}

bool isDelim(const int delim)
{
   	return ((delim != EOF) && (strchr(DELIMS, delim) != NULL));
}

bool isDelimType(const struct value *v) {
	return v && (v->type == VAL_DELIM);
}

bool isDelimChar(const struct value *v, char match) {
	return isDelimType(v) && (v->ch == match);
}

bool isStrType(struct value *v) {
    	return v && (v->type == VAL_NAME);
}


// init functions
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

struct value *initOpValue(enum operator op, struct reader *r)
{
	struct value *val = initValue();
	if (!val)
		raise_syntax_error("failed to initialize value", r);

	val->type = VAL_OP;
	val->op = op;  
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
	
	if ((tok->type == VAL_KEYWORD) &&
			strcmp(getKeyStr(tok->key), expected)) {
		freeValue(tok);
		raise_syntax_error("Unexpected keyword value", r);
    	} else if ((isStrType(tok) &&
			(!tok->str || strcmp(tok->str, expected)))) {
		freeValue(tok);
		raise_syntax_error("Missing token string", r);
    	} else if ((tok->type == VAL_DELIM) &&
			(tok->ch != expected[0])) {
		freeValue(tok);
		raise_syntax_error("Unexpected token value", r);
	}
	freeValue(tok);
}
