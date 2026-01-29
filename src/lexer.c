/** @file lexer_ctx.c
 *  @brief Functions for reading from a file
 * 
 *  This contains the utility functions for
 *  the lexer_ctx struct, a number of static inline
 *  functions for readability's sake, as well
 *  as some value utility functions
 *
 *  @author Hawkins Peterson (hawkins03)
 *  @bug No known bugs.
 */


#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "structs.h"
#include "utils.h"
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

static bool read_next_line(struct lexer_ctx *lex) {
	if (!lex || !lex->fp || feof(lex->fp)) {
		lex->ch = EOF;
		return false;
	}
	
	lex->line_buf[0] = '\0';
	fgetpos(lex->fp, &lex->line_start_pos);

	if (!fgets(lex->line_buf, lex->line_cap, lex->fp)) {
		lex->ch = EOF;
		return false;
	}

	lex->line_pos = 0;
	lex->line_num++;
	lex->ch = lex->line_buf[lex->line_pos];
	return true;
} 


static inline int peek(struct lexer_ctx *lex) {
	if (!lex || !lex->line_buf || (lex->line_pos > strlen(lex->line_buf)))
		return EOF;
	int out = lex->line_buf[lex->line_pos];
	if (out == '\0')
		return EOF;
	return out; 
}
static inline int advance(struct lexer_ctx *lex) {
	int out = peek(lex);
	if (!lex || (out == EOF) || !lex->line_buf) {
		lex->ch = EOF;
		return out;
	}
	bool atEOL = lex->line_pos >= strlen(lex->line_buf) - 1;
	if (!atEOL) {
		lex->ch = lex->line_buf[++lex->line_pos];
		return out;
	}
	
	if (atEOL && !feof(lex->fp)) {
		read_next_line(lex);
		return out;
	}
		
	// at the end of the file.
	lex->ch = EOF;
	lex->line_pos = 0;
	free(lex->line_buf);
	lex->line_buf = NULL;

	return out;
}

static inline void skip_spaces(struct lexer_ctx *lex) {
	while (lex && (isspace(lex->ch)) && (lex->ch != '\0'))
		advance(lex);
}

// struct lexer_ctx util functions
struct lexer_ctx *readInFile(const char *filename) {
	if (filename == NULL)
		return NULL;

	struct lexer_ctx *lex = calloc(1, sizeof(*lex));
	if (!lex)
		return NULL;

	lex->fp = fopen(filename, "r");

	if ((!lex->fp) || (feof(lex->fp))) {
		free(lex);
		return NULL;
	}
	
	lex->root = init_stmt(lex);
	lex->root->next = lex->root; // self loop to mark as sentinal 
	lex->filename = strdup(filename);

	lex->line_num = 0;
	lex->line_pos = 0;
	lex->line_cap = DEFAULT_LINE_CAP;
	lex->line_buf = calloc(lex->line_cap, sizeof((*lex->line_buf)));

	if (!lex->line_buf)
		raise_syntax_error(ERR_NO_MEM, lex);

	read_next_line(lex);

	lex->ch = peek(lex);
	nextValue(lex);
	return lex;
}

void killReader(struct lexer_ctx *lex) {
	if (!lex)
		raise_error(ERR_REFREE);

	if (lex->line_buf) {
		free(lex->line_buf);
		lex->line_buf = NULL;
	}

	if (lex->filename) {
		free(lex->filename);
		lex->filename = NULL;
	}

	if ((lex->val.str) && ((lex->val.type == VAL_STR) || lex->val.type == VAL_NAME))
		free(lex->val.str);

	free_stmt(lex->root);
	lex->root = NULL;

	fclose(lex->fp);
	lex->fp = NULL;
	free(lex);
}

//op checker functions
const char *getOpStr(enum operator op) {
	if ((op >= 0) && (op < OP_UNKNOWN))
		return OP_STRINGS[op];
	return NULL;
}

const char *getKeyStr(enum key_type key) {
	if ((key <= KW_BREAK) && (key >= KW_VAR))
		return KEYWORDS[key];
	return NULL;
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

bool isBinaryOp(const enum operator op) {
	return getPrio(op) >= 0;
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

//inline checker functions
static inline bool matchesOp(const int op) {
	return (op != EOF) && strchr(OP_START, op);
}

static inline bool isKeyword(const enum key_type  key)
{
	return ((key >= KW_VAR) && (key <= KW_FALSE));
}

static inline bool isTrueFalseKey(const enum key_type  key)
{
    	return ((key == KW_TRUE) || (key == KW_FALSE));
}

static inline bool isDelim(const int delim) {
   	return ((delim != EOF) && (strchr(DELIMS, delim) != NULL));
}
static inline enum operator getOpEnum(const char *op) {
	for (int i = 0; i < OP_UNKNOWN; i++)
		if (!strcmp(OP_STRINGS[i], op))
			return (enum operator) i;
	return OP_UNKNOWN;
}
static inline enum key_type getKeyType(char *keyword) {
	if (!keyword)
		return -1;
	for (int i = 0; KEYWORDS[i] != NULL; i++)
		if (!strcmp(KEYWORDS[i], keyword))
			return (enum key_type) i;
	
	return -1;
}


static inline void getSingleLineComment(struct lexer_ctx *lex) {
	while (lex && (advance(lex) != '\n'));
}

static inline void getMultiLineComment(struct lexer_ctx *lex) {
	char prev = advance(lex);
	char curr = advance(lex);

	while (lex && (prev != '*') && (curr != '/')) {
		prev = curr;
		curr = advance(lex);
	}
}

//fetching value contents
static inline enum operator getNextOp(struct lexer_ctx *lex) {
	char tmp[MAX_OP_LEN + 1] = {0};
	int i = 0;
	do {
		tmp[i] = peek(lex);
		if ((i >= MAX_OP_LEN) || (getOpEnum(tmp) == OP_UNKNOWN)) {
			tmp[i] = '\0';
			break;
		}
		advance(lex);
		i++;
	} while (1);

	return getOpEnum(tmp);
}

static inline int getNextNum(struct lexer_ctx *lex) {
	long num = 0;
	while (isdigit(peek(lex)))
		num = (num * 10) + (advance(lex) - '0');
	
	if (num >= INT_MAX)
		raise_syntax_error(ERR_BIG_NUM, lex);

	return num;
}

static inline char *getNextWord(struct lexer_ctx *lex) {
	int cap = MAX_WORD_LEN;
	char *str = calloc(cap + 1, sizeof(*str));
	if (!str)
		raise_syntax_error(ERR_NO_MEM, lex);

	int len = 0;
	while (lex && (isalnum(peek(lex)) || (peek(lex) == '_'))) {
		str[len++] = (char) advance(lex);
		reset_strlen_if_needed(&str, len, &cap, lex);
	}
	
	set_strlen(&str, len+1, lex);
	str[len] = '\0';
	return str;
}

static inline int getCharacterValue(struct lexer_ctx *lex) {
	int ch = advance(lex);
	if (ch == '\'')
		return 0;

	if (advance(lex) != '\'')
		raise_syntax_error(ERR_UNMATCHED_QUOTE, lex);
	//ERR_MISS_DELIM

	return ch;
}

static inline char catchEscapeChar(struct lexer_ctx *lex) {
	switch (advance(lex)) {
	case 'n':  
		return '\n';
	case 't':  
		return '\t';
	case 'r':  
		return '\r';
	case '"':  
		return '"';
	case '\\': 
		return '\\';
	default:
		return '\0';
	}
}

static inline char *getNextString(struct lexer_ctx *lex) {
	int cap = DEFAULT_LINE_CAP * 2;
	int len = 0;
	char *buf = calloc(cap, sizeof(*buf));
	if (!buf)
		raise_syntax_error(ERR_NO_MEM, lex);

	while (lex && (lex->ch != '\"') && (lex->ch != EOF)) {
		if (lex->ch == '\\') {
			advance(lex);

			buf[len] = catchEscapeChar(lex);
			if (buf[len++] == '\0') {
				free(buf);
				raise_syntax_error(ERR_INV_ESC, lex);
			}
		} else {
			buf[len++] = advance(lex);
		}
		reset_strlen_if_needed(&buf, len, &cap, lex);
	}

	if (advance(lex) != '"') {
		free(buf);
		raise_syntax_error(ERR_UNMATCHED_QUOTE, lex);
	}
	set_strlen(&buf, len + 1, lex);
	buf[len] = '\0';
	return buf;
}

static inline char getNextDelim(struct lexer_ctx *lex) {
	char ch = advance(lex);
	if (ch == '/') {
		switch (peek(lex)) {
		case '/':
			getSingleLineComment(lex);
			return advance(lex);
		case '*':
			getMultiLineComment(lex);
			return advance(lex);
		}
	}
	return ch;
}


// init value functions
static inline void initKeywordValue(enum key_type  key, struct lexer_ctx *lex) {
	if (isTrueFalseKey(key)) {
		lex->val.type = VAL_NUM;
		lex->val.num = (key == KW_TRUE);
		return;
	}
	lex->val.type = VAL_KEYWORD;
	lex->val.num = key;
}

static inline void initNameValue(char *str, struct lexer_ctx *lex) {
	enum key_type key = getKeyType(str);
	if (isKeyword(key)) {
		free(str);
		initKeywordValue(key, lex);
		return;
	}

	lex->val.type = VAL_NAME;
	lex->val.str = str;
}

static inline void initOpValue(enum operator op, struct lexer_ctx *lex) {
	lex->val.type = VAL_OP;
	lex->val.num = op;   
}

static inline void initDelimValue(char delim, struct lexer_ctx *lex) {
	switch (delim) {
	case '\'':
		lex->val.num = getCharacterValue(lex);
		lex->val.type = VAL_NUM;
		return;
	case '"':
		lex->val.str = getNextString(lex);
		lex->val.type = VAL_STR;
		return;
	}

	lex->val.type = VAL_DELIM;
	lex->val.ch = delim;
}

static inline void initNumValue(int num, struct lexer_ctx *lex) {
	lex->val.type = VAL_NUM;
	lex->val.num = num;
}

//fetching functions
void nextValue(struct lexer_ctx *lex) {
	skip_spaces(lex);

	if (!lex || lex->ch == EOF || lex->ch == '\0')
		return;

	lex->val.start_pos = lex->line_pos;

	if (isalpha(lex->ch))
		initNameValue(getNextWord(lex), lex);
	else if (matchesOp(lex->ch))
		initOpValue(getNextOp(lex), lex);
	else if (isDelim(lex->ch))
		initDelimValue(advance(lex), lex);
	else if (isdigit(lex->ch))
		initNumValue(getNextNum(lex), lex);
	else
		raise_syntax_error(ERR_UNEXP_CHAR, lex);
}

inline struct value Value(struct lexer_ctx *lex) {
	return lex->val;
}

void acceptValue(struct lexer_ctx *lex, enum value_type type, const char *expected) {
	struct value tok = lex->val;
	if (!expected)
		raise_syntax_error(ERR_INTERNAL, lex);
	if (tok.type != type)
		raise_syntax_error(ERR_INV_VAL, lex);
	
	if ((tok.type == VAL_KEYWORD) && strcmp(getKeyStr(tok.num), expected))
		raise_syntax_error(ERR_INV_VAL, lex);
    	else if ((tok.type == VAL_DELIM) && (tok.ch != expected[0]))
		raise_syntax_error(ERR_INV_VAL, lex);
	nextValue(lex);
}


char *stealNextString(struct lexer_ctx *lex) {
	struct value v = lex->val;
	if (v.type != VAL_STR)
		raise_syntax_error(ERR_INV_VAL, lex);

	char *out = v.str;
	lex->val.str = NULL;
	lex->val.type = VAL_EMPTY;
	nextValue(lex);
	return out;
}

char *stealNextName(struct lexer_ctx *lex) {
	struct value v = lex->val;
	if (v.type != VAL_NAME)
		raise_syntax_error(ERR_INV_VAL, lex);

	char *out = v.str;
	lex->val.str = NULL;
	lex->val.type = VAL_EMPTY;
	nextValue(lex);
	return out;
}

enum operator stealNextOp(struct lexer_ctx *lex) {
	struct value v = lex->val;
	if ((v.type != VAL_OP) || (v.num == OP_UNKNOWN))
		return OP_UNKNOWN;
	enum operator op = v.num;
	lex->val.type = VAL_EMPTY;
	nextValue(lex);
	return op;
}
