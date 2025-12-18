#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "ms.h"
#include "utils.h"

bool isWordChar(const char ch) {
    return (isalnum(ch) || (ch == '_'));
}

bool isOp(const char op) {
    return (strchr(OPS, op) != NULL);
}

bool isBinOp(const char *binop) {
    for (int i = 0; i < BINOPS_COUNT; i++)
	if (!strcmp(BINOPS[i], binop))
	    return true;
    return false;
}

bool isDelim(const char delim) {
    return (strchr(DELIMS, delim) != NULL);
}

bool isKeyword(const char *keyword) {
    for (int i = 0; i < KEYWORDS_COUNT; i++)
	if (!strcmp(KEYWORDS[i], keyword))
	    return true;
    return false;
}

bool matchesBinop(const char ch) {
    return strchr(BINOP_STARTS, ch) != NULL;
}

bool isStrType(Value *v) {
    return ((v->type == VAL_STR) || (v->type == VAL_KEYWORD) || (v->type == VAL_BINOP));
}
Value *initValue() {
    Value *val =  calloc(1, sizeof(*val));
    if (val == NULL)
	raise_error("Error, bad Value malloc");
    val->type = VAL_EMPTY;
    return val;
}

void freeValue(Value *val) {
    if (val->type == VAL_EMPTY)
	raise_error("Error, double freeing value");
    if ((isStrType(val)) && (val->str != NULL))
	free(val->str);
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
    //r->curr_token = getToken(r);
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

char getNextOp(Reader *r) {
    if (peek(r) == EOF) return '\0';
    if (isOp(peek(r)))
	return (char) advance(r);
    return '\0';
}

char *getNextBinOp(char first, Reader *r) {
    if (first != '\0') {
	if (!strchr(BINOP_STARTS, first))
	    return NULL;

	char *out = calloc(3, sizeof(*out));
	out[0] = (char) first;
	if (peek(r) == EOF)
	    raise_error("unexpected: reached end of file while reading in binop");
	if (strchr(BINOP_ENDS, (char) peek(r)))
	    out[1] = (char) advance(r);

	//isBinOp?
	return out;
    } else {
	if (peek(r) == EOF)
	    raise_error("unexpected: reached end of file while reading in binop");
	if (!strchr(BINOP_STARTS, peek(r)) || (peek(r) == '\0'))
	    return NULL;
	char *out = calloc(3, sizeof(*out));
	out[0] = (char) advance(r);
	if (peek(r) == EOF)
	    raise_error("unexpected: reached end of file while reading in binop");
	if (strchr(BINOP_ENDS, peek(r)))
	    out[1] = (char) advance(r);

	return out;
    }
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

Value *getRawToken(Reader *r) {
    Value *val = initValue();
    if (peek(r) == EOF) return NULL;
    //printf("getting raw token, peek(r) = '%c'\n", (char) peek(r));
    if (isalpha(peek(r))) {
	char *out = getNextWord(r);
	if (isKeyword(out))
	    val->type = VAL_KEYWORD;
	else
	    val->type = VAL_STR;
	//printf("STR(%s)\n", out);
	val->str = out;
    } else if (isOp(peek(r))) {
	char out = getNextOp(r);
	val->type = VAL_OP;
	val->ch = out;
	//printf("OP(%c)\n", out);
    } else if (isDelim(peek(r))) {
	char *shared = "=";
	char delim = getNextDelim(r);
	if (strchr(shared, delim)) {
	    char *out = getNextBinOp(delim, r);
	    if (!out) {
		free(val);
		raise_error("invalid binop");
	    }

	    val->type = VAL_BINOP;
	    val->str = out;
	    //printf("BINOP(%s) - delim\n", out);
	} else {
	    val->type = VAL_DELIM;
	    val->ch = delim;
	    //printf("DELIM(%c)\n", delim);
	}
    } else if (matchesBinop(peek(r))) {
	char *out = getNextBinOp('\0', r);
	if (!out) {
	    free(val);
	    raise_error("invalid binop");
	}
	val->type = VAL_BINOP;
	val->str = out;
	//printf("BINOP(%s)\n", out);
    } else if (isdigit(peek(r))) {
	int out = getNextNum(r);
	val->type = VAL_NUM;
	val->num = out;
	//printf("NUM(%d)\n", val->num);
    } else if (!isAlive(r)) {
	free(val);
	return NULL;
    } else {
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
    //printf("returning token %p\n", out);
    return out;
}

Value *peekToken(Reader *r) {
    return r->curr_token;
}

void acceptToken(Value *tok, ValueType type, const char *expected) {
    if (!tok || !tok->str || !expected)
	raise_error("Invalid Null token value");
    if (tok->type != type)
	raise_error("Invalid token type");
    if (isStrType(tok) && strcmp(tok->str, expected))
	raise_error("Unexpected token value");
    else if (!isStrType(tok) && (tok->ch != expected[0]))
	raise_error("Unexpected token value");
}

bool isAlive(Reader *r) {
    return r->alive;
}

void killReader(Reader *r) {
    //printf("killing now\n");
    if (!r) raise_error("Error, double freeing reader");

    if (r->curr_token) {
	Value *tok = r->curr_token;
	if ((tok->type == VAL_STR) || (tok->type == VAL_KEYWORD) || (tok->type == VAL_BINOP))
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
