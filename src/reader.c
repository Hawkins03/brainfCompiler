#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "reader.h"
#include "utils.h"
#include "value.h"
#include "stmt.h"


static enum operator BINARY_OPS[][5] = {  		// lowest priority to highest priority:
	{OP_LOGICAL_OR, OP_UNKNOWN},     		// 0: logical or
	{OP_LOGICAL_AND, OP_UNKNOWN},                   // 1: logical and
	{OP_BITWISE_OR, OP_UNKNOWN},                    // 2: bitwiase or
	{OP_BITWISE_XOR, OP_UNKNOWN},                   // 3: bitwise xor
	{OP_BITWISE_AND, OP_UNKNOWN},                   // 4: bitwise and
	{OP_EQ, OP_NE, OP_UNKNOWN},             	// 5: equivalence operators
	{OP_LT, OP_GT, OP_LE, OP_GE, OP_UNKNOWN},   	// 6: relational operators
	{OP_LEFT_SHIFT, OP_RIGHT_SHIFT, OP_UNKNOWN},	// 7: bitwise shifts
	{OP_PLUS, OP_MINUS, OP_UNKNOWN},               	// 8: addition / subtraction
	{OP_MULTIPLY, OP_DIVIDE, OP_MODULO, OP_UNKNOWN} // 9: multiplication, division, modulo
};

struct reader *readInFile(const char *filename)
{
	if (filename == NULL)
		return NULL;

	struct reader *r = calloc(1, sizeof(*r));
	if (!r)
		return NULL;

	r->fp = fopen(filename, "r");

	if ((!r->fp) || (feof(r->fp))) {
		free(r);
		return NULL;
	}
	
	r->root = init_stmt(r);
	r->root->next = r->root; // self loop to mark as sentinal 

	r->filename = strdup(filename, r);

	advance(r);
	nextValue(r);
	return r;
}

void killReader(struct reader *r) 
{
	if (!r)
		raise_error("r already freed");

	if (r->filename) {
		free(r->filename);
		r->filename = NULL;
	}

	free_stmt(r->root);
	r->root = NULL;

	fclose(r->fp);
	r->fp = NULL;
	free(r);
}


// Checker Functions
bool readerIsAlive(struct reader *r)
{
	return r != NULL;
}

bool hasNextStmt(struct reader *r)
{
	struct value *tok = getValue(r);
	return (tok != NULL) && !((tok->type == VAL_DELIM) && (tok->ch == '}'));
}

bool atSemicolon(struct reader *r)
{
	struct value *tok = getValue(r);
	return (tok != NULL) && ((tok->type == VAL_DELIM) && (tok->ch == ';'));
}

bool parserCanProceed(struct reader *r)
{
	return readerIsAlive(r) && hasNextStmt(r) && !atSemicolon(r);
}

bool isValidNameChar(const char ch, struct reader *r) {
	return readerIsAlive(r) && (isalnum(ch) || (ch == '_'));
}


// getting single characters
int peek(struct reader *r) {
	if (!readerIsAlive(r))
		return EOF;
	return r->curr_char;
}

int advance(struct reader *r)
{
	if (!readerIsAlive(r))
		return EOF;

	int out = r->curr_char;
	if (r->fp && !feof(r->fp))
		r->curr_char = fgetc(r->fp);
	return out;
}

void skip_spaces(struct reader *r)
{
	while ((peek(r) == '\n') || isspace(peek(r)))
		advance(r);
}

char *stealNextString(struct reader *r) {
	struct value *v = getValue(r);
	if (!v || (v->type != VAL_NAME))
		return NULL;

	char *out = v->str;
	nextValue(r);
	return out;
}

enum operator stealNextOp(struct reader *r) {
	struct value *v = getValue(r);
	if (!v || (v->type != VAL_OP) || (v->num == OP_UNKNOWN))
		return OP_UNKNOWN;
	enum operator op = v->num;
	nextValue(r);
	return op;
}

char getNextDelim(struct reader *r)
{
	int ch = peek(r);
	if (!readerIsAlive(r))
		raise_syntax_error("Got EOF", r);
	if (isDelim(ch))
		return (char) advance(r);
	return 0;
}

static enum operator getOpEnum(const char *op) {
	for (int i = 0; i < OP_UNKNOWN; i++)
		if (!strcmp(OP_STRINGS[i], op))
			return (enum operator) i;
	return OP_UNKNOWN;
}

void printOpStr(enum operator op) {
	if ((op >= 0) && (op < OP_UNKNOWN))
		printf("%s", OP_STRINGS[op]);
}

void getOpStr(enum operator op, char *out) {
	if ((op >= 0) && (op < OP_UNKNOWN) && out)
		sprintf(out + strlen(out), "%s", OP_STRINGS[op]);
}

int getPrio(const enum operator op) {
	if (op == OP_UNKNOWN)
		return -1;
	for (int y = 0; y < NUM_PRIOS; y++)
		for (int x = 0; BINARY_OPS[y][x] != OP_UNKNOWN; x++)
			if (op == BINARY_OPS[y][x])	
				return y;
	return -1;
}

bool isAssignOp(const enum operator op) {
	for (int i = 0; ASSIGN_OPS[i] != OP_UNKNOWN; i++)
		if (op == ASSIGN_OPS[i])
			return true;
	return false;
}

bool isBinaryOp(enum operator op) {
	return getPrio(op) >= 0;
}

bool matchesOp(const int op) {
	return (op != EOF) && strchr(OP_START, op);
}

bool is_suffix_unary(enum operator op) {
    	if (!op)
		return false;

	for (int i = 0; (SUFFIX_OPS[i] != OP_UNKNOWN); i++)
		if (op == SUFFIX_OPS[i])
			return true;

    	return false;
}

bool is_prefix_unary(enum operator op) {
	if (!op)
		return false;

	for (int i = 0; (PREFIX_OPS[i] != OP_UNKNOWN); i++) {
		if (PREFIX_OPS[i] == op)
			return true;
	}

    	return false;
}

static inline bool isOp(char *op) {
	return getOpEnum(op) != OP_UNKNOWN;
}

enum operator getNextOp(struct reader *r) {
	if (!readerIsAlive(r))
		raise_syntax_error("got EOF", r);
	if (!matchesOp(peek(r)))
		raise_syntax_error("expected next character to be an operator", r);
	
	char tmp[MAX_OP_LEN + 1] = {0};
	for (int i = 0; i < MAX_OP_LEN; i++) {
		tmp[i] = peek(r);
		if (!readerIsAlive(r) || !isOp(tmp)) {
			tmp[i] = '\0';
			break;
		}
		advance(r);
	}
	enum operator op = getOpEnum(tmp);
	return op;
}

int getNextNum(struct reader *r) {
	if (!readerIsAlive(r))
		raise_syntax_error("Got EOF", r);
	long num = 0;
	while (isdigit(peek(r)))
		num = (num * 10) + (advance(r) - '0');
	
	if (num >= INT_MAX)
		raise_syntax_error("number is greater than the max int size", r);

	return num;
}

char *getNextWord(struct reader *r) {
	if (!isalpha(peek(r)))
		raise_syntax_error("Invallid word (first letter must be in the range 'a-zA-Z')", r);
	
	int cap = MAX_WORD_LEN;
	char *str = calloc(cap + 1, sizeof(*str));
	if (!str)
		raise_syntax_error("failed to allocate space in the heap", r);

	int len = 0;
	while (isValidNameChar(peek(r), r)) {
		str[len++] = (char) advance(r);
		reset_strlen_if_needed(&str, len, &cap, r);
	}
	
	set_strlen(&str, len+1, r);
	str[len] = '\0';
	return str;
}

enum key_type getKeyType(char *keyword) {
	if (!keyword)
		return -1;
	for (int i = 0; KEYWORDS[i] != NULL; i++)
		if (!strcmp(KEYWORDS[i], keyword))
			return (enum key_type) i;
	
	return -1;
}

const char *getKeyStr(enum key_type key) {
	if ((key <= KW_BREAK) && (key >= KW_VAR))
		return KEYWORDS[key];
	return NULL;
}

int getCharacterValue(struct reader *r) {
	int out = advance(r);
	if (out == '\'')
		return 0;

	if (advance(r) != '\'')
		raise_syntax_error("missing ending single quote", r);

	return out;
}

char catchEscapeChar(struct reader *r)
{
	switch (advance(r)) {
	case 'n':  
		return '\n';
	case 't':  
		return '\t';
		break;
	case 'r':  
		return '\r';
	case '"':  
		return '"';
		break;
	case '\\': 
		return '\\';
		break;
	default:
		return '\0';
	}
}

char *getNextString(struct reader *r)
{
	if (!readerIsAlive(r))
		return NULL;

	int cap = 16;
	int len = 0;
	char *buf = calloc(cap, sizeof(*buf));
	if (!buf)
		raise_syntax_error("ran out of memory space on the heap", r);

	while (readerIsAlive(r) && (peek(r) != '\"')) {
		if (peek(r) == '\\') {
			advance(r); //consume '\'

			buf[len] = catchEscapeChar(r);
			if (buf[len++] == '\0') {
				free(buf);
				raise_syntax_error("invalid escape character", r);
			}
		} else {
			buf[len++] = advance(r);
		}
		reset_strlen_if_needed(&buf, len, &cap, r);
	}

	if (advance(r) != '"') {
		free(buf);
		raise_syntax_error("Expected ending quote after string", r);
	}
	set_strlen(&buf, len + 1, r);
	buf[len] = '\0';
	return buf;
}
