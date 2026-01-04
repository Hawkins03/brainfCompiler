#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"
#include "parser.h"
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



bool isWordChar(const char ch) {
    return (isalnum(ch) || (ch == '_'));
}

bool isOp(const char *op) {
    for (int y = 0; y < NUM_PRIOS; y++)
	for (int x = 0; OPS[y][x] != NULL; x++)
	    if (!strcmp(OPS[y][x], op))
		return true;
    return false;
}

int getPrio(const char *op) {
    for (int y = 0; y < NUM_PRIOS; y++)
	for (int x = 0; OPS[y][x] != NULL; x++)
	    if (!strcmp(OPS[y][x], op))
		return y;
    return -1;
}

bool isRightAssoc(int prio) {
    return ((prio == 0) || (prio == 11));
}

bool matchesOp(const char op) {
    return (strchr(OP_START, op) != NULL);
}

bool isUnaryOp(const char *op) {
    int prio = getPrio(op);
    return ((prio == 10) || (prio == 12));
}

bool isSuffixOp(Value *tok) {
    if (!tok || (tok->type != VAL_OP) || !(tok->str))
	return false;

    for (int i = 0; i < SUFFIX_OPS_LEN; i++)
	if (!strcmp(SUFFIX_OPS[i], tok->str))
	    return true;

    return false;
}

bool isDelim(const char delim) {
    return (strchr(DELIMS, delim) != NULL);
}

bool isStrType(Value *v) {
    return ((v->type == VAL_STR) || (v->type == VAL_OP));
}

Value *initValue() {
    Value *val =  calloc(1, sizeof(*val));
    if (val == NULL)
	raise_error("Error, bad Value malloc");
    val->type = VAL_EMPTY;
    return val;
}

void freeValue(Value *val) {
    //printf("freeing value: %p\n", val);
    if (val->type == VAL_EMPTY)
	raise_error("Error, double freeing value");
    if ((isStrType(val)) && (val->str != NULL))
	free(val->str);
    val->type = VAL_EMPTY;
    free(val);
}

Reader *readInFile(const char *filename) {
    if (filename == NULL)
	raise_error("bad filename");

    //printf("start of readInFile\n");
    Reader *r = calloc(1, sizeof(*r));
    if (!r)
	raise_error("bad Reader malloc");
    r->fp = fopen(filename, "r");
    if (!r->fp) {
	free(r);
	raise_error("bad file");
    } else if (feof(r->fp)) {
	free(r);
	raise_error("EOF on file open");
    }
    r->alive = true;
    r->curr_token = NULL;
    r->curr = fgetc(r->fp);
    getToken(r); //preloading token (aka first crank);

    //printf("end of readInFile\n");
    return r;
}

int peek(Reader *r) {
    //printf("peeking, alive = %d, out = '%c'\n", isAlive(r), r->curr);
    if (!isAlive(r)) return EOF;
    return r->curr;
}

int advance(Reader *r) {
    if (!isAlive(r)) return EOF;

    int out = r->curr;
    //printf("out '%c', %p\n", out, r->fp);
    if (r->fp && !feof(r->fp))
	r->curr = fgetc(r->fp);
    
    if (!r->fp)
	r->alive = false;
    else if (feof(r->fp))
	r->alive = false;
    return out;
}

void skip_spaces(Reader *r) {
    if (!isAlive(r)) return;
    while (r->fp && !feof(r->fp)) {
	//printf("skipping a space\n");
	if (peek(r) == EOF) return;
	if (!isspace((char) peek(r)))
	    return;
	if (advance(r) == '\0') return;
    }
}

char *stealTokString(Value *tok) {
    char *s = tok->str;
    tok->str = NULL;
    return s;
}

char getNextDelim(Reader *r) {
    if (peek(r) == EOF) return '\0';
    if (isDelim(peek(r)) && (peek(r) != '\0'))
	return (char) advance(r);
    return '\0';
}

char *getNextOp(Reader *r) { //operates under assumption that all ops can be made from smaller ops : i.e. <<=, <<, <
    if (peek(r) == EOF)
        raise_error("unexpected: reached end of file while reading in binop");
    if (!matchesOp(peek(r)))
        return NULL;
    char *out = calloc(MAX_OP_LEN + 1, sizeof(*out));
    for (int i = 0; ((i < MAX_OP_LEN) && (peek(r) != EOF)); i++) {
    	out[i] = (char) peek(r);
	if (!isOp(out)) {
	    out[i] = '\0';
	    break;
	} else
	    advance(r);
    }
    return out;
}

int getNextNum(Reader *r) {
    if (!isAlive(r)) raise_error("Error, getting num when Reader died");
    int num = 0;
    while (isdigit(peek(r))) {
	num = (num * 10) + (advance(r) - '0');
    }
    return num;
}

char *getNextWord(Reader *r) {
    if (!isAlive(r)) raise_error("Error, getting word when Reader died");

    if (!isalpha(peek(r))) raise_error("Invallid first letter in word (first letter must be in the range 'a-zA-Z')");
    char chars[MAX_WORD_LEN + 1] = {0};
    int len = 0;

    for (int i = 0; i < MAX_WORD_LEN; i++) {
	if ((peek(r) == '\0') || (peek(r) == EOF)) break;
	if (isalnum(peek(r)) || (peek(r) == '_'))
	    chars[len++] = (char) advance(r);
	else
	    break;
    }

    char *word = calloc(len+1, sizeof(*word));
    strncpy(word, chars, len);
    word[len] = 0;
    return word;
}

KeyType getKeyType(char *keyword) {
	if (!keyword)
		return -1;
	for (int i = 0; i < KEYWORDS_COUNT; i++) {
		if (!strcmp(KEYWORDS[i], keyword))
			return (KeyType) i;
	}
	return -1;
}

const char *getKeyStr(KeyType key) {
	return KEYWORDS[key];
}

int getCharacterValue(Reader *r) { //i.e. 'x' it gives the int value of whatever x is
	int out = advance(r);
	if (out == '\'')
		return 0;

	if (advance(r) != '\'')
		raise_error("missing ending single quote");
	return out;
}

Value *getRawToken(Reader *r) {
    Value *val = initValue();
    if (peek(r) == EOF) return NULL;
    //printf("getting raw token, peek(r) = '%c'\n", (char) peek(r));
    if (isalpha(peek(r))) {
		char *out = getNextWord(r);
		KeyType key = getKeyType(out);
		if (key != -1) {
			val->type = VAL_KEYWORD;
			val->key = key;
			free(out);
		} else {
			val->type = VAL_STR;
			val->str = out;
		}
    } else if (matchesOp(peek(r))) {
		char *out = getNextOp(r);
		val->type = VAL_OP;
		val->str = out;
		//printf("OP(%c)\n", out);
    } else if (isDelim(peek(r))) {
		char delim = getNextDelim(r);
		if (delim == '\'') {
			val->num = getCharacterValue(r);
			val->type = VAL_NUM;
		} else {
			val->type = VAL_DELIM;
			val->ch = delim;
		}
        //printf("DELIM(%c)\n", delim);
    } else if (isdigit(peek(r))) {
		int out = getNextNum(r);
		val->type = VAL_NUM;
		val->num = out;
		//printf("NUM(%d)\n", val->num);
	} else if (!isAlive(r)) {
		free(val);
		return NULL;
    } else {
		printf("val = %c, %d\n", peek(r), matchesOp(peek(r)));
		free(val);
		raise_error("Error, unexpected character");
    }
    return val;
}

Value *getToken(Reader *r) {
    //printf("getting token: %d\n", isAlive(r));
    if (!isAlive(r)) return NULL;

    skip_spaces(r);

    Value *out = r->curr_token;
    r->curr_token = getRawToken(r);
    return out;
}

Value *peekToken(Reader *r) {
    if (!isAlive(r)) return NULL;
    return r->curr_token;
}

void acceptToken(Reader *r, ValueType type, const char *expected) {
	Value *tok = getToken(r);
    if (!tok)
		raise_error("Invalid Null token value");
	if (!expected)
		raise_error("Invalid expected value");
    if (tok->type != type) {
		printf("type = %d vs %d\n", type, tok->type);
		raise_error("Invalid token type");
	}
	
	if (tok->type == VAL_KEYWORD) {
		if (strcmp(getKeyStr(tok->key), expected))
			raise_error("Unexpected keyword value");
    } else if (isStrType(tok)) {
		if (!tok->str || strcmp(tok->str, expected))
			raise_error("Unexpected token value");
    } else if (tok->type == VAL_DELIM) {
		if (tok->ch != expected[0])
			raise_error("Unexpected token value");
	}
	freeValue(tok);
}

bool isAlive(Reader *r) {
    return r->alive;
}

void printVal(Value *tok) {
    if (!tok)
	printf("NULL()\n");
    switch (tok->type) {
	case VAL_KEYWORD:
	    printf("KEYWORD(%s)\n", getKeyStr(tok->key));
	    break;
	case VAL_STR:
	    printf("STR(%s)\n", tok->str);
	    break;
	case VAL_OP:
	    printf("OP(%s)\n", tok->str);
	    break;
	case VAL_DELIM:
	    printf("DELIM(%c)\n", tok->ch);
	    break;
	case VAL_NUM:
	    printf("NUM(%d)\n", tok->num);
	    break;
	case VAL_EMPTY:
	    printf("EMPTY()\n");
	    break;
	default:
	    raise_error("invalid value type\n");
	    break;
    }
}


void killReader(Reader *r) {
    //printf("killing now\n");
    if (!r) raise_error("Error, double freeing reader");

    if (r->curr_token) {
	Value *tok = r->curr_token;
	if (((tok->type == VAL_STR) || (tok->type == VAL_OP)) && (tok->str))
	    free(r->curr_token->str);
	freeValue(r->curr_token);
    }
    r->curr_token = NULL;

    fclose(r->fp);
    r->fp = NULL;
    free(r);
}

void _raise_error(const char *msg, const char *func, const char *file, int line) {
    fprintf(stderr, "ERROR in %s at %s:%d - %s:%s\n", func, file, line, msg, strerror(errno));
    fflush(stderr);
    exit(EXIT_FAILURE);
}
