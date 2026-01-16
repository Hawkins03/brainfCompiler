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
	return v && (v->type == VAL_OP) && v->num != OP_UNKNOWN;
}

bool isSuffixVal(const struct value *v) {
	return isOpType(v) && is_suffix_unary(v->num);
}

bool isPrefixVal(struct value *v) {
	return isOpType(v) && is_prefix_unary(v->num);
}

bool isBinaryOpVal(struct value *v) {
	return isOpType(v) && isBinaryOp(v->num);
}

bool isAssignOpVal(struct value *v) {
	return isOpType(v) && isAssignOp(v->num);
}

bool isValidOp(const struct value *v, const int minPrio) {
	if (v->type != VAL_OP)
		return false;
	if (isAssignOp(v->num))
		return true;
	if (!isBinaryOp(v->num))
		return false;
	return getPrio(v->num) >= minPrio;
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


static inline void initKeywordValue(enum key_type  key, struct reader *r)
{
	if (isTrueFalseKey(key)) {
		r->curr_token.type = VAL_NUM;
		r->curr_token.num = (key == KW_TRUE);
	} else {
		r->curr_token.type = VAL_KEYWORD;
		r->curr_token.num = key;
	}
}

static inline void initNameValue(char *str, struct reader *r)
{
	enum key_type key = getKeyType(str);
	if (isKeyword(key)) {
		free(str);
		initKeywordValue(key, r);
		return;
	}

	r->curr_token.type = VAL_NAME;
	r->curr_token.str = str;
}

static inline void initOpValue(enum operator op, struct reader *r) {
	r->curr_token.type = VAL_OP;
	r->curr_token.num = op;   
}

static inline void initDelimValue(char delim, struct reader *r) {
	switch (delim) {
	case '\'':
		r->curr_token.num = getCharacterValue(r);
		r->curr_token.type = VAL_NUM;
		break;
	case '"':
		r->curr_token.str = getNextString(r);
		r->curr_token.type = VAL_STR;
		break;
	default:
		r->curr_token.type = VAL_DELIM;
		r->curr_token.ch = delim;
		break;
	}
}

static inline void initNumValue(int num, struct reader *r) {
	r->curr_token.type = VAL_NUM;
	r->curr_token.num = num;
}

//fetching functions
void nextValue(struct reader *r)
{
	skip_spaces(r);

	char ch = peek(r);
	if (ch == EOF || !readerIsAlive(r))
		return;

	if (isalpha(ch))
		initNameValue(getNextWord(r), r);
	else if (matchesOp(ch))
		initOpValue(getNextOp(r), r);
	else if (isDelim(ch))
		initDelimValue(getNextDelim(r), r);
	else if (isdigit(peek(r)))
		initNumValue(getNextNum(r), r);
	else
		raise_syntax_error("Error, unexpected character", r);
}

inline struct value *getValue(struct reader *r)
{
	return &(r->curr_token);
}

void acceptValue(struct reader *r, enum value_type type, const char *expected)
{
	struct value *tok = getValue(r);
	if (!tok)
		raise_syntax_error("Invalid Null token value", r);
	else if (!expected)
		raise_syntax_error("Invalid expected value", r);
	else if (tok->type != type)
		raise_syntax_error("Unexpected token type", r);
	
	if ((tok->type == VAL_KEYWORD) && strcmp(getKeyStr(tok->num), expected))
		raise_syntax_error("Unexpected keyword value", r);
    	else if ((isStrType(tok) && (!tok->str || strcmp(tok->str, expected))))
		raise_syntax_error("Missing token string", r);
    	else if ((tok->type == VAL_DELIM) && (tok->ch != expected[0]))
		raise_syntax_error("Unexpected token value", r);
	nextValue(r);
}
