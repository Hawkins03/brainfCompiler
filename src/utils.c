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


// Reader functions
bool isAlive(Reader *r) {
    return r->alive;
}

Reader *readInFile(const char *filename) {
    if (filename == NULL)
		raise_error("bad filename");

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
    return r;
}

void killReader(Reader *r) {
    if (!r) raise_error("Error, double freeing reader");

    if (r->curr_token)
		freeValue(r->curr_token);

    r->curr_token = NULL;

    fclose(r->fp);
    r->fp = NULL;
    free(r);
}

int peek(Reader *r) {
    if (!isAlive(r))
		return EOF;
	else
    	return r->curr;
}

int advance(Reader *r) {
    if (!isAlive(r))
		return EOF;

	int out = r->curr;
	if (r->fp && !feof(r->fp))
		r->curr = fgetc(r->fp);

	if (!r->fp)
		r->alive = false;
	else if (feof(r->fp))
		r->alive = false;
	return out;
}

void skip_spaces(Reader *r) {
	if (!isAlive(r))
		return;
	while (((peek(r) == '\n') || isspace(peek(r))) && isAlive(r))
		advance(r);
}

char* strdup(const char* s) {
    size_t len = strlen(s);
    char* result = calloc(len + 1, sizeof(*result));
    if (result == NULL) {
		raise_error("failed to allocate space on the heap");
    }
    strcpy(result, s);
    return result;
}


// Checker Functions
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

bool isSuffixOp(value_t *tok) {
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

bool isStrType(value_t *v) {
    return ((v->type == VAL_NAME) || (v->type == VAL_OP));
}

bool hasNextStmt(Reader *r) {
	value_t *tok = peekToken(r);
	return (tok != NULL) && !((tok->type == VAL_DELIM) && (tok->ch == '}'));
}


// value_t Functions
value_t *initValue() {
    value_t *val =  calloc(1, sizeof(*val));
    if (val == NULL)
		raise_error("Error, bad value_t malloc");
    val->type = VAL_EMPTY;
    return val;
}

void printVal(value_t *tok) {
    if (!tok)
		printf("NULL()\n");
    switch (tok->type) {
		case VAL_KEYWORD:
			printf("KEYWORD(%s)\n", getKeyStr(tok->key));
			break;
		case VAL_NAME:
			printf("NAME(%s)\n", tok->str);
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
			raise_error("invalid value_t type\n");
			break;
    }
}

void freeValue(value_t *val) {
	if (val == NULL)
		return;
	if (val->type == VAL_EMPTY)
		raise_error("Error, double freeing value");
	if ((isStrType(val)) && (val->str != NULL))
		free(val->str);
	val->type = VAL_EMPTY;
	free(val);
}


// Tokenization Functions
char *stealTokString(value_t *tok) {
    char *s = tok->str;
    tok->str = NULL;
    return s;
}

char getNextDelim(Reader *r) {
    if (peek(r) == EOF)
		return '\0';
    else if (isDelim(peek(r)) && (peek(r) != '\0'))
		return (char) advance(r);
    else
		return '\0';
}

char *getNextOp(Reader *r) { //
    if (peek(r) == EOF)
        raise_error("unexpected: reached end of file while reading in binop");
    if (!matchesOp(peek(r)))
        return NULL;
    char *out = calloc(MAX_OP_LEN + 1, sizeof(*out));
	if (!out)
		raise_error("failed to allocate space on the heap");
    for (int i = 0; ((i < MAX_OP_LEN) && (peek(r) != EOF)); i++) {
    	out[i] = (char) peek(r); // no touchey. It works right now, and isn't unsatable, just a bit inelegant
		if (!isOp(out)) {
			out[i] = '\0';
			break;
		} else {
			advance(r);
		}
    }
	if (strlen(out) < MAX_OP_LEN) {
		char *temp = realloc(out, strlen(out) + 1);
		if (!temp) {
			free(out);
			raise_error("failed to reallocate space on the heap");
		}
		out = temp;
	}
    return out;
}

int getNextNum(Reader *r) {
    if (!isAlive(r))
		raise_error("Error, getting num when Reader died");
    int num = 0;
    while (isdigit(peek(r)))
		num = (num * 10) + (advance(r) - '0');

    return num;
}

char *getNextWord(Reader *r) {
    if (!isAlive(r))
		raise_error("Error, getting word when Reader died");

    if (!isalpha(peek(r)))
		raise_error("Invallid first letter in word (first letter must be in the range 'a-zA-Z')");
    
	char chars[MAX_WORD_LEN + 1] = {0};
    int len = 0;

    for (int i = 0; i < MAX_WORD_LEN; i++) {
		if ((peek(r) == '\0') || (peek(r) == EOF))
			break;
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

key_t getKeyType(char *keyword) {
	if (!keyword)
		return -1;
	for (int i = 0; KEYWORDS[i] != NULL; i++)
		if (!strcmp(KEYWORDS[i], keyword))
			return (key_t) i;
	
	return -1;
}

const char *getKeyStr(key_t key) { //TODO: ensure that key is a valid KeyType
	return KEYWORDS[key];
}

int getCharacterValue(Reader *r) { //i.e. 'x' it gives the int value_t of whatever x is
	int out = advance(r);
	if (out == '\'')
		return 0;

	if (advance(r) != '\'')
		raise_error("missing ending single quote");
	return out;
}

char *getStringValue(Reader *r) {
	if (!isAlive(r) || !peek(r) || (peek(r) == EOF))
		return NULL;
	size_t cap = 16;
	size_t len = 0;
	char *buf = calloc(cap, sizeof(*buf));
	if (!buf)
		raise_error("ran out of memory space on the heap");
	while (true) {
		int nextCh = peek(r);

		if (nextCh == EOF) {
				free(buf);
				raise_error("got EOF before end of string");
		} else if ( nextCh == '"') {
			advance(r);
			break;
		} else if (nextCh == '\\') {
			advance(r); // consume '\'
			nextCh = advance(r);
			
			switch (nextCh) {
				case 'n':  
					nextCh = '\n';
					break;
				case 't':  
					nextCh = '\t';
					break;
				case 'r':  
					nextCh = '\r';
					break;
				case '"':  
					nextCh = '"';
					break;
				case '\\': 
					nextCh = '\\';
					break;
				default:
					free(buf);
					return NULL; // invalid escape
			}
		} else
			nextCh = advance(r);

		if (len + 1 >= cap) {
			cap *= 2;
			char *temp = realloc(buf, cap);
			if (!temp) {
				free(buf);
				raise_error("ran out of memory space on the heap");
			}
			buf = temp;
		}

		buf[len++] = (char) nextCh;
	}
	char *temp = realloc(buf, len + 1);
	if (!temp) {
		free(buf);
		raise_error("ran out of memory space on the heap");
	}
	buf = temp;
	buf[len] = '\0';
	return buf;
}

int GetTrueFalseValue(key_t key) {
	if (key == KW_TRUE)
		return 1;
	else if (key == KW_FALSE)
		return 0;
	return -1; //to silence compiler warning
}

value_t *getRawToken(Reader *r) {
    value_t *val = initValue();
    if (peek(r) == EOF)
		return NULL;

    if (isalpha(peek(r))) {
		char *out = getNextWord(r);
		key_t key = getKeyType(out);
		if ((key != -1) && (GetTrueFalseValue(key) != -1)) {
			val->type = VAL_NUM;
			val->num = GetTrueFalseValue(key);
			free(out);
		} else if (key != -1) {
			val->type = VAL_KEYWORD;
			val->key = key;
			free(out);
		} else {
			val->type = VAL_NAME;
			val->str = out;
		}
    } else if (matchesOp(peek(r))) {
		char *out = getNextOp(r);
		val->type = VAL_OP;
		val->str = out;
    } else if (isDelim(peek(r))) {
		char delim = getNextDelim(r);
		if (delim == '\'') {
			val->num = getCharacterValue(r);
			val->type = VAL_NUM;
		} else if (delim == '"') {
			val->str = getStringValue(r);
			val->type = VAL_STR;
		} else {
			val->type = VAL_DELIM;
			val->ch = delim;
		}
    } else if (isdigit(peek(r))) {
		int out = getNextNum(r);
		val->type = VAL_NUM;
		val->num = out;
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

value_t *getToken(Reader *r) {
    if (!isAlive(r)) return NULL;

    skip_spaces(r);

    value_t *out = r->curr_token;
    r->curr_token = getRawToken(r);
    return out;
}

value_t *peekToken(Reader *r) {
    if (!isAlive(r))
		return NULL;
    return r->curr_token;
}

void acceptToken(Reader *r, value_type_t type, const char *expected) {
	value_t *tok = getToken(r);
	if (!tok) {
		freeValue(tok);
		if (r->curr_token)
			freeValue(r->curr_token);
		raise_error("Invalid Null token value");
	} else if (!expected) {
		freeValue(tok);
		if (r->curr_token)
			freeValue(r->curr_token);
		raise_error("Invalid expected value");
	} else if (tok->type != type) {
		freeValue(tok);
		if (r->curr_token)
			freeValue(r->curr_token);
		raise_error("Unexpected token type");
	}
	
	if (tok->type == VAL_KEYWORD) {
		if (strcmp(getKeyStr(tok->key), expected)) {
			freeValue(tok);
			raise_error("Unexpected keyword value");
		}
    } else if (isStrType(tok)) {
		if (!tok->str || strcmp(tok->str, expected)) {
			freeValue(tok);
			raise_error("Unexpected token value");
		}
    } else if (tok->type == VAL_DELIM) {
		if (tok->ch != expected[0]) {
			freeValue(tok);
			raise_error("Unexpected token value");
		}
	}
	freeValue(tok);
}


// Error Handling
void _raise_error(const char *msg, const char *func, const char *file, int line) {
    fprintf(stderr, "ERROR in %s at %s:%d - %s:%s\n", func, file, line, msg, strerror(errno));
    fflush(stderr);
    exit(EXIT_FAILURE);
}
