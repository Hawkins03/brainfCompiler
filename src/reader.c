#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "reader.h"
#include "utils.h"
#include "value.h"
#include "stmt.h"

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
	
	r->root = init_stmt();
	if (!r->root) {
		killReader(r);
		raise_syntax_error("failed to init root statement", r);
		return NULL;
	}

	r->root->next = r->root; // self loop to mark as sentinal
	r->curr_stmt = r->root;

	r->curr_token = initValue(r);
	if (!r->curr_token) {
		raise_syntax_error("failed to allocate initial value", r);
	}

	r->filename = strdup(filename, r);

	advance(r);
	freeValue(getValue(r));
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

	freeValue(r->curr_token);
	r->curr_token = NULL;

	fclose(r->fp);
	r->fp = NULL;
	free(r);
}


// Checker Functions
bool readerIsAlive(struct reader *r)
{
	return (!r || (r->curr_token != NULL));
}

bool hasNextStmt(struct reader *r)
{
	struct value *tok = peekValue(r);
	return (tok != NULL) && !((tok->type == VAL_DELIM) && (tok->ch == '}'));
}

bool atSemicolon(struct reader *r)
{
	struct value *tok = peekValue(r);
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
int peek(struct reader *r)
{
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

// Tokenization Functions
char *stealTokString(struct value *tok)
{
	if (!tok)
		return NULL;

	char *s = tok->str;
	tok->str = NULL;
	return s;
}

char *stealNextString(struct reader *r)
{
	char *out = stealTokString(peekValue(r));
	if (!out)
		raise_syntax_error("failed to get token string", r);
	freeValue(getValue(r));
	return out;
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

char *getNextOp(struct reader *r)
{
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
	return strdup(tmp, r);
}

int getNextNum(struct reader *r)
{
	if (!readerIsAlive(r))
		raise_syntax_error("Got EOF", r);
	long num = 0;
	while (isdigit(peek(r)))
		num = (num * 10) + (advance(r) - '0');
	
	if (num >= INT_MAX)
		raise_syntax_error("number is greater than the max int size", r);

	return num;
}

char *getNextWord(struct reader *r)
{
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

enum key_type getKeyType(char *keyword)
{
	if (!keyword)
		return -1;
	for (int i = 0; KEYWORDS[i] != NULL; i++)
		if (!strcmp(KEYWORDS[i], keyword))
			return (enum key_type) i;
	
	return -1;
}

const char *getKeyStr(enum key_type key)
{
	if ((key <= KW_BREAK) && (key >= KW_VAR))
		return KEYWORDS[key];
	return NULL;
}

int getCharacterValue(struct reader *r)
{ //i.e. 'x' it gives the int struct value of whatever x is
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
