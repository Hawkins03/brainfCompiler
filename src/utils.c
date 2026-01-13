#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"
#include "parser.h"
#include "semantics.h"
#include "utils.h"
#include "stmt.h"
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
bool readerIsAlive(Reader *r) { //TODO: update to 
    return (!r || !r->fp || !feof(r->fp) || (r->curr_char == EOF));
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
	
	r->root = init_stmt();
	if (!r->root) {
		fclose(r->fp);
		free(r);
		raise_syntax_error("failed to init root statement", r);
		return NULL;
	}

	r->root->next = r->root; // self loop to mark as sentinal
	r->curr_stmt = r->root;

	advance(r);
	getToken(r);
	return r;
}

void killReader(Reader *r) {
    if (!r)
		raise_error("Error, r already freed");

	free_stmt(r->root);
	r->root = NULL;

	if (r->curr_token)
		free_value(r->curr_token);
    r->curr_token = NULL;

    fclose(r->fp);
    r->fp = NULL;
    free(r);
}

int peek(Reader *r) {
    if (!readerIsAlive(r))
		return EOF;
    return r->curr_char;
}

int advance(Reader *r) {
    if (!readerIsAlive(r))
		return EOF;

	int out = r->curr_char;
	if (r->fp && !feof(r->fp))
		r->curr_char = fgetc(r->fp);
	return out;
}

void skip_spaces(Reader *r) {
	while ((peek(r) == '\n') || isspace(peek(r)))
		advance(r);
}

char* strdup(const char* s, Reader *r) {
    size_t len = strlen(s);
    char* result = calloc(len + 1, sizeof(*result));
    if (result == NULL)
		raise_syntax_error("failed to allocate space for a string", r);
    strcpy(result, s);
    return result;
}


// Checker Functions
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

bool isValidKey(key_t key) {
	return ((key >= KW_VAR) && (key <= KW_FALSE));
}

bool isDelim(const int delim) {
    return ((delim != EOF) && (strchr(DELIMS, delim) != NULL));
}

bool isStrType(value_t *v) {
    return v && ((v->type == VAL_NAME) || (v->type == VAL_OP));
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
    return calloc(1, sizeof(value_t));
}

void print_value(value_t *tok, Reader *r) {
    if (!tok)
		printf("NULL()\n");
    switch (tok->type) {
		case VAL_EMPTY:
			printf("EMPTY()\n");
			break;
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
		default:
			raise_syntax_error("invalid value type", r);
    }
}

void free_value(value_t *val) {
	if ((isStrType(val)) && (val->str != NULL))
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
	int ch = peek(r);
	if (!readerIsAlive(r))
		raise_syntax_error("Got EOF", r);
    if (isDelim(ch))
		return (char) advance(r);
	return 0;
}

char *getNextOp(Reader *r) {
	if (!readerIsAlive(r))
		raise_syntax_error("got EOF", r);
    if (!matchesOp(peek(r)))
        raise_syntax_error("expected next character to be an operator", r);
    char out[MAX_OP_LEN + 1] = {0};

    for (int i = 0; i < MAX_OP_LEN; i++) {
		if (!readerIsAlive(r) || !matchesOp(peek(r))) {
			out[i] = '\0';
			break;
		}
		out[i] = advance(r);
    }
    return strdup(out, r);
}

int getNextNum(Reader *r) {
    if (!readerIsAlive(r))
		raise_syntax_error("Got EOF", r);
    long num = 0;
    while (isdigit(peek(r)))
		num = (num * 10) + (advance(r) - '0');
	if (num >= INT_MAX)
		raise_syntax_error("number is greater than the max int size", r);

    return num;
}

char *getNextWord(Reader *r) {
    if (!isalpha(peek(r)))
		raise_syntax_error("Invallid word (first letter must be in the range 'a-zA-Z')", r);
    
	char chars[MAX_WORD_LEN + 1] = {0};
    int len = 0;

    for (int i = 0; i < MAX_WORD_LEN; i++) {
		if (!readerIsAlive(r) && !isalnum(peek(r)) && (peek(r) != '_'))
			break;
		if (isalnum(peek(r)) || (peek(r) == '_'))
			chars[len++] = (char) advance(r);
    }

    char *word = calloc(len + 1, sizeof(*word));
	if (!word)
		raise_syntax_error("failed to allocate space in the heap", r);
    strncpy(word, chars, len + 1);
    word[len] = '\0';
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

const char *getKeyStr(key_t key) {
	if ((key <= KW_BREAK) && (key >= KW_VAR))
		return KEYWORDS[key];
	return NULL;
}

int getCharacterValue(Reader *r) { //i.e. 'x' it gives the int value_t of whatever x is
	int out = advance(r);
	if (out == '\'')
		return 0;

	if (advance(r) != '\'')
		raise_syntax_error("missing ending single quote", r);

	return out;
}

char *getStringValue(Reader *r) {
	if (!readerIsAlive(r))
		return NULL;
	size_t cap = 16;
	size_t len = 0;
	char *buf = calloc(cap, sizeof(*buf));
	if (!buf)
		raise_syntax_error("ran out of memory space on the heap", r);

	while (readerIsAlive(r) && (peek(r) != '\"')) {

		if (peek(r) == '\\') {
			advance(r); //consume '\'
			
			switch (advance(r)) {
				case 'n':  
					buf[len++] = '\n';
					break;
				case 't':  
					buf[len++] = '\t';
					break;
				case 'r':  
					buf[len++] = '\r';
					break;
				case '"':  
					buf[len++] = '"';
					break;
				case '\\': 
					buf[len++] = '\\';
					break;
				default:
					free(buf);
					raise_syntax_error("invalid escape character", r);
			}
		} else
			buf[len++] = advance(r);

		if (len + 1 >= cap) {
			cap *= 2;
			char *temp = realloc(buf, cap);
			if (!temp) {
				free(buf);
				raise_syntax_error("ran out of memory space on the heap", r);
			}
			buf = temp;
		}
	}

	if (advance(r) != '"') {
		free(buf);
		raise_syntax_error("Expected ending quote after string", r);
	}
	char *temp = realloc(buf, len + 1);
	if (!temp) {
		free(buf);
		raise_syntax_error("ran out of memory space on the heap", r);
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
	value_t *val = initValue(r);
	if (!val) {
		raise_syntax_error("failed to initialize value", r);
		return NULL;
	}
    if (peek(r) == EOF)
		return NULL;

    if (isalpha(peek(r))) {
		char *out = getNextWord(r);
		key_t key = getKeyType(out);
		if (isValidKey(key)) {
			if (GetTrueFalseValue(key) != -1) {
				val->type = VAL_NUM;
				val->num = GetTrueFalseValue(key);
				free(out);
			} else {
				val->type = VAL_KEYWORD;
				val->key = key;
				free(out);
			}
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
		switch (delim) {
			case '\'':
				val->num = getCharacterValue(r);
				val->type = VAL_NUM;
				break;
			case '"':
				val->str = getStringValue(r);
				val->type = VAL_STR;
				break;
			default:
				val->type = VAL_DELIM;
				val->ch = delim;
				break;
		}
    } else if (isdigit(peek(r))) {
		int out = getNextNum(r);
		val->type = VAL_NUM;
		val->num = out;
	} else if (!readerIsAlive(r))
		free(val);
    else {
		free(val);
		raise_syntax_error("Error, unexpected character", r);
    }
    return val;
}

value_t *getToken(Reader *r) {
    if (!readerIsAlive(r))
		return NULL;

    skip_spaces(r);

    value_t *out = r->curr_token;
    r->curr_token = getRawToken(r);
    return out;
}

value_t *peekToken(Reader *r) {
    if (!readerIsAlive(r))
		return NULL;
    return r->curr_token;
}

void acceptToken(Reader *r, value_type_t type, const char *expected) {
	value_t *tok = getToken(r);
	if (!tok) {
		free_value(tok);
		raise_syntax_error("Invalid Null token value", r);
	} else if (!expected) {
		free_value(tok);
		raise_syntax_error("Invalid expected value", r);
	} else if (tok->type != type) {
		free_value(tok);
		raise_syntax_error("Unexpected token type", r);
	}
	
	if (tok->type == VAL_KEYWORD) {
		if (strcmp(getKeyStr(tok->key), expected)) {
			free_value(tok);
			raise_syntax_error("Unexpected keyword value", r);
		}
    } else if (isStrType(tok)) {
		if (!tok->str || strcmp(tok->str, expected)) {
			free_value(tok);
			raise_syntax_error("Missing token string", r);
		}
    } else if (tok->type == VAL_DELIM) {
		if (tok->ch != expected[0]) {
			free_value(tok);
			raise_syntax_error("Unexpected token value", r);
		}
	}
	free_value(tok);
}


// Error Handling
void _raise_error(const char *msg, const char *func, const char *file, int line) {
	fprintf(stderr, "ERROR in %s at %s:%d - %s\n", func, file, line, msg);
	exit(EXIT_FAILURE);
}

void _raise_syntax_error(const char *msg, const char *func, const char *file, int line, Reader *r) {
	if (r)
		killReader(r);
	_raise_error(msg, func, file, line);
}

void _raise_semantic_error(const char *msg, const char *func, const char *file, int line, env_t *env) {
	if (env)
		free_env(env);
	_raise_error(msg, func, file, line);
}