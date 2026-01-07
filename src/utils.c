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
#include <execinfo.h>

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
    return (r->error == ERR_NONE);
}

Reader *readInFile(const char *filename) {
    if (filename == NULL)
		return NULL;

    Reader *r = calloc(1, sizeof(*r));
    if (!r)
		return NULL;
    r->fp = fopen(filename, "r");
    if (!r->fp) {
		free(r);
		return NULL;
    } else if (feof(r->fp)) {
		free(r);
		return NULL;
    }
	r->error = ERR_NONE;
    r->curr_token = NULL;
    r->curr = fgetc(r->fp);
    getToken(r); //preloading token (aka first crank);
    return r;
}

void killReader(Reader *r) {
    if (!r) {
		fprintf(stderr, "Error, r already freed");
		exit(EXIT_FAILURE);
	}


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

	if (!r->fp) {
		r->error = ERR_EOF;
		r->error_msg = strdup("End of file");
	} else if (feof(r->fp)) {
		r->error = ERR_EOF;
		r->error_msg = strdup("End of file");
	}
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
    if (result == NULL)
		return NULL;
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

bool atSemicolon(Reader *r) {
	value_t *tok = peekToken(r);
	return (tok != NULL) && ((tok->type == VAL_DELIM) && (tok->ch == ';'));
}


// value_t Functions
value_t *initValue() {
    value_t *val =  calloc(1, sizeof(*val));
    if (val == NULL)
		return NULL;
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
    }
}

void freeValue(value_t *val) {
	if (val == NULL) {
		fprintf(stderr, "value is null");
		exit(EXIT_FAILURE);
	} else if ((isStrType(val)) && (val->str != NULL))
		free(val->str);
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
    if (peek(r) == EOF) {
		raise_error("unexpected: reached end of file while reading in op", ERR_UNEXPECTED_TOKEN, r);
		return NULL;
	}
    if (!matchesOp(peek(r)))
        return NULL;
    char *out = calloc(MAX_OP_LEN + 1, sizeof(*out));
	if (!out) {
		raise_error("failed to allocate space on the heap", ERR_OOM, r);
		return NULL;
	}
    for (int i = 0; ((i < MAX_OP_LEN) && (peek(r) != EOF)); i++) {
    	out[i] = (char) peek(r);
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
			raise_error("failed to reallocate space on the heap", ERR_OOM, r);
			return NULL;
		}
		out = temp;
	}
    return out;
}

int getNextNum(Reader *r) {
    if (!isAlive(r)) {
		raise_error("Error, getting num when Reader died", ERR_UNEXPECTED_TOKEN, r);
		return 0;
	}
    int num = 0;
    while (isdigit(peek(r)))
		num = (num * 10) + (advance(r) - '0');

    return num;
}

char *getNextWord(Reader *r) {
    if (!isAlive(r)) {
		raise_error("Error, getting word when Reader died", ERR_UNEXPECTED_TOKEN, r);
		return NULL;
	}

    if (!isalpha(peek(r))) {
		raise_error("Invallid first letter in word (first letter must be in the range 'a-zA-Z')", ERR_UNEXPECTED_TOKEN, r);
		return NULL;
	}
    
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
	if ((key <= KW_BREAK) && (key >= KW_VAR))
		return KEYWORDS[key];
	return NULL;
}

int getCharacterValue(Reader *r) { //i.e. 'x' it gives the int value_t of whatever x is
	int out = advance(r);
	if (out == '\'')
		return 0;

	if (advance(r) != '\'') {
		raise_error("missing ending single quote", ERR_UNEXPECTED_TOKEN, r);
		return 0;
	}
	return out;
}

char *getStringValue(Reader *r) {
	if (!isAlive(r) || !peek(r) || (peek(r) == EOF))
		return NULL;
	size_t cap = 16;
	size_t len = 0;
	char *buf = calloc(cap, sizeof(*buf));
	if (!buf) { 
		raise_error("ran out of memory space on the heap", ERR_OOM, r);
		return NULL;
	}
	while (true) {
		int nextCh = peek(r);

		if (nextCh == EOF) {
				free(buf);
				raise_error("got EOF before end of string", ERR_UNEXPECTED_TOKEN, r);
				return NULL;
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
					raise_error("invalid escape in string", ERR_UNEXPECTED_TOKEN, r);
					return NULL;
			}
		} else
			nextCh = advance(r);

		if (len + 1 >= cap) {
			cap *= 2;
			char *temp = realloc(buf, cap);
			if (!temp) {
				free(buf);
				raise_error("ran out of memory space on the heap", ERR_OOM, r);
				return NULL;
			}
			buf = temp;
		}

		buf[len++] = (char) nextCh;
	}
	char *temp = realloc(buf, len + 1);
	if (!temp) {
		free(buf);
		raise_error("ran out of memory space on the heap", ERR_OOM, r);
		return NULL;
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
	if (!val) {
		raise_error("failed to initialize value", ERR_OOM, r);
		return NULL;
	}
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
		raise_error("Error, unexpected character", ERR_UNEXPECTED_TOKEN, r);
		return NULL;
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
		raise_error("Invalid Null token value", ERR_UNEXPECTED_TOKEN, r);
		return;
	} else if (!expected) {
		freeValue(tok);
		if (r->curr_token)
			freeValue(r->curr_token);
		raise_error("Invalid expected value", ERR_UNEXPECTED_TOKEN, r);
		return;
	} else if (tok->type != type) {
		freeValue(tok);
		if (r->curr_token)
			freeValue(r->curr_token);
		raise_error("Unexpected token type", ERR_UNEXPECTED_TOKEN, r);
		return;
	}
	
	if (tok->type == VAL_KEYWORD) {
		if (strcmp(getKeyStr(tok->key), expected)) {
			freeValue(tok);
			raise_error("Unexpected keyword value", ERR_UNEXPECTED_TOKEN, r);
		}
    } else if (isStrType(tok)) {
		if (!tok->str || strcmp(tok->str, expected)) {
			freeValue(tok);
			raise_error("Missing token string", ERR_UNEXPECTED_TOKEN, r);
		}
    } else if (tok->type == VAL_DELIM) {
		if (tok->ch != expected[0]) {
			freeValue(tok);
			raise_error("Unexpected token value", ERR_UNEXPECTED_TOKEN, r);
			return;
		}
	}
	freeValue(tok);
}


// Error Handling
void _raise_error(const char *msg, ErrorType err_type, const char *func, const char *file, int line, Reader *r) {
	if (r) {
		r->error = err_type;
		char *temp = NULL;
		int needed = snprintf(NULL, 0, "ERROR (%d) in %s at %s:%d - %s\n", err_type, func, file, line, msg);
		if (needed < 0) {
			r->error_msg = NULL;
			return;
		}
		temp = malloc((size_t) needed + 1); /* allocate exact size (assume int fits) */
		if (!temp) {
			r->error_msg = NULL;
			return;
		}
		snprintf(temp, (size_t) needed + 1, "ERROR (%d) in %s at %s:%d - %s\n", err_type, func, file, line, msg);
		r->error_msg = strdup(temp); /* duplicate into reader-owned memory */
		free(temp);
	}
}