#include <ctype.h>
#include <errno.h>
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

Value *initValue() {
    Value *val = (Value *) calloc(1, sizeof(Value));
    if (val == NULL)
	raise_error("Error, bad Value malloc");
    val->type = VAL_EMPTY;
    return val;
}

void freeValue(Value *val) {
    if (val->type == VAL_EMPTY)
	raise_error("Error, double freeing value");
    if ((val->type == VAL_NAME) || (val->type == VAL_BINOP) || (val->type == VAL_KEYWORD))
	free(val->str);
    free(val);
}

Reader *readInFile(const char *filename) {
    if (filename == NULL)
	raise_error("bad filename");
    Reader *r = (Reader *) malloc(sizeof(Reader));
    if (!r)
	raise_error("bad Reader malloc");
    r->fp = fopen(filename, "r");
    if (!r->fp) {
	free(r);
	raise_error("bad file");
    } else if (feof(r->fp))
	raise_error_and_free("EOF on file open", r);
    fseek(r->fp, 0L, SEEK_END);
    r->chars_left = ftell(r->fp);
    fseek(r->fp, 0L, SEEK_SET);

    r->curr_token = getToken(r);
    r->curr = fgetc(r->fp);
    return r;
}

char peek(Reader *r) {
    if (r->curr == EOF) raise_error("peek read EOF");
    return r->curr;
}
char advance(Reader *r) {
    if (r->chars_left-- <= 1) {
	return '\0';
    }
    char out = r->curr;
    //printf("out '%c', %p\n", out, r->fp);
    if (r->fp && !feof(r->fp)) {
	r->curr = fgetc(r->fp);
    }
    if (!r->fp)
	killReader(r);
    if (feof(r->fp))
	killReader(r);
    return out;
}

void accept(Reader *r, char ch, char *message) {
    if (peek(r) != ch)
	raise_error(message);
    else
	advance(r);
}

void skip_spaces(Reader *r) {
    if (!isAlive(r)) return;
    while (r->fp && !feof(r->fp)) {
	//printf("skipping a space\n");
	if (!isspace(peek(r)))
	    return;
	if (advance(r) == '\0') return;
    }
}

char getNextDelim(Reader *r) {
    if (isDelim(peek(r)))
	return advance(r);
    return '\0';
}

char getNextOp(Reader *r) {
    if (isOp(peek(r)))
	return advance(r);
    return '\0';
}

char *getNextBinOp(char first, Reader *r) {
    if (first != '\0') {
	if (!strchr(BINOP_STARTS, first))
	    return "";
	
	char *out = calloc(3, sizeof(char));
	out[0] = advance(r);
	if (strchr(BINOP_ENDS, peek(r)))
	    out[1] = advance(r);

	//isBinOp?
	return out;
    } else {
        if (!strchr(BINOP_STARTS, peek(r)))
	    return "";
	char *out = calloc(3, sizeof(char));
	out[0] = advance(r);
	if (strchr(BINOP_ENDS, peek(r)))
	    out[1] = advance(r);

	return out;
    }
}

int getNextNum(Reader *r) {
    if (!isAlive(r)) raise_error("Error, getting num when Reader died");
    int num = 0;
    while (isdigit(peek(r))) {
	num = (num * 10) + advance(r) - '0';
    }
    //printf("num = %d\n", num);
    return num;
}

char *getNextWord(Reader *r) {
    if (!isAlive(r)) raise_error("Error, getting word when Reader died");
     
    skip_spaces(r);
    //printf("%c\n", peek(r));

    if (!isalpha(peek(r))) raise_error("Invallid first letter in word (first letter must be in the range 'a-zA-Z')");
    char chars[BUFF_SIZE] = {0};
    int len = 0;

    for (int i = 0; i < BUFF_SIZE; i++) {
	if (isalnum(peek(r)) || (peek(r) == '_'))
	    chars[len++] = peek(r);
	else
	    break;

	advance(r);
    }

    char *word = (char *) calloc(len, sizeof(char));
    strncpy(word, chars, len);
    return word;
}

Value *getRawToken(Reader *r) {
    Value *val = initValue();
    if (isalpha(peek(r))) {
	char *out = getNextWord(r);
	if (isKeyword(out))
	    val->type = VAL_KEYWORD;
	else
	    val->type = VAL_NAME;
	val->str = out;
    } else if (isOp(peek(r))) {
	char out = getNextOp(r);
	val->type = VAL_OP;
	val->ch = out;

    } else if (isDelim(peek(r))) {
	char *shared = "=";
	char delim = getNextDelim(r);
	if (strchr(shared, delim)) {
	    char *out = getNextBinOp(delim, r);
	    val->type = VAL_BINOP;
	    val->str = out;
	} else {
	    val->type = VAL_DELIM;
	    val->ch = delim;
	}
    } else if (matchesBinop(peek(r))) {
	char *out = getNextBinOp('\0', r);
	val->type = VAL_BINOP;
	val->str = out;
    } else if (isdigit(peek(r))) {
	int out = getNextNum(r);
	val->type = VAL_NUM;
	val->num = out;
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
    skip_spaces(r);
    Value *out = r->curr_token;
    r->curr_token = getRawToken(r);
    return out;
}

Value *peekToken(Reader *r) {
    return r->curr_token;
}

void _acceptToken(Reader *r, ValueType type, const char *expected, const char *func, const char *file, int line) {
    Value *tok = peekToken(r);
    if (!tok) _raise_error("invalid input to accept_token", func, file, line);
    switch (tok->type) {
	case VAL_BINOP:
	case VAL_NAME:
	case VAL_KEYWORD:
	    if (tok->str != expected) _raise_error("invalid value of accepted token", func, file, line);
	    break;
	case VAL_DELIM:
	case VAL_OP:
	    if (tok->ch != expected[0]) _raise_error("invalid value of accepted token", func, file, line);
	    break;
	default:
	    _raise_error("invalid type", func, file, line);
    }
    freeValue(getToken(r));
}

void _acceptNumToken(Reader *r, int expected, const char *func, const char *file, int line) {
    Value *tok = peekToken(r);
    if (tok->type != VAL_NUM) _raise_error("invalid type in acceptNumToken", func, file, line);
    if (tok->num != expected) _raise_error("invalid value of accepted numeric token", func, file, line);
    freeValue(getToken(r));
}

bool isAlive(Reader *r) {
    return r->chars_left > 0;
}

void killReader(Reader *r) {
    //printf("killing now\n");
    if (!r || (r->chars_left > 0)) raise_error("Error, double freeing reader");
    r->chars_left = 0;
    fclose(r->fp);
    r->fp = NULL;
    free(r);
}

void _raise_error(const char *msg, const char *func, const char *file, int line) {
    fprintf(stderr, "ERROR in %s at %s:%d - %s:%s\n", func, file, line, msg, strerror(errno));
    fflush(stderr);
    exit(EXIT_FAILURE);
}
