/** @file reader.c
 *  @brief Functions for reading from a file
 * 
 *  This contains the utility functions for
 *  the reader struct, a number of static inline
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

#include "reader.h"
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

static bool read_next_line(struct reader *r) {
	if (!r || !r->fp || feof(r->fp))
		return false;
	
	r->line_buf[0] = '\0';
	fgetpos(r->fp, &r->line_start_pos);

	if (!fgets(r->line_buf, r->line_cap, r->fp))
		return false;

	r->line_pos = 0;
	r->line_num++;
	return true;
} 


static inline int peek(struct reader *r) {
	if (!r)
		return EOF;
	int out = r->line_buf[r->line_pos];
	if (out == '\0')
		return EOF;
	return out; 
}
static inline int advance(struct reader *r) {
	int out = peek(r);
	if (!r || !r->fp || feof(r->fp))
		return out;
	
	r->line_pos++;

	if (out == '\n')
		read_next_line(r);
	return out;
}

static inline void skip_spaces(struct reader *r) {
	while ((peek(r) == '\n') || isspace(peek(r)))
		advance(r);
}

// struct reader util functions
struct reader *readInFile(const char *filename) {
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

	r->line_num = 1;
	r->line_pos = 0;
	r->line_cap = DEFAULT_LINE_CAP;
	r->line_buf = calloc(r->line_cap, sizeof((*r->line_buf)));

	if (!r->line_buf)
		raise_syntax_error(ERR_NO_MEM, r);

	read_next_line(r);

	r->ch = peek(r);
	nextValue(r);
	return r;
}

void killReader(struct reader *r) {
	if (!r)
		raise_error(ERR_REFREE);

	if (r->line_buf) {
		free(r->line_buf);
		r->line_buf = NULL;
	}

	if (r->filename) {
		free(r->filename);
		r->filename = NULL;
	}

	if ((r->val.str) && ((r->val.type == VAL_STR) || r->val.type == VAL_NAME))
		free(r->val.str);

	free_stmt(r->root);
	r->root = NULL;

	fclose(r->fp);
	r->fp = NULL;
	free(r);
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


static inline void getSingleLineComment(struct reader *r) {
	while (r && (advance(r) != '\n'));
}

static inline void getMultiLineComment(struct reader *r) {
	char prev = advance(r);
	char curr = advance(r);

	while (r && (prev != '*') && (curr != '/')) {
		prev = curr;
		curr = advance(r);
	}
}

//fetching value contents
static inline enum operator getNextOp(struct reader *r) {
	char tmp[MAX_OP_LEN + 1] = {0};
	int i = 0;
	do {
		tmp[i] = peek(r);
		if ((i >= MAX_OP_LEN) || (getOpEnum(tmp) == OP_UNKNOWN)) {
			tmp[i] = '\0';
			break;
		}
		advance(r);
		i++;
	} while (1);

	return getOpEnum(tmp);
}

static inline int getNextNum(struct reader *r) {
	long num = 0;
	while (isdigit(peek(r)))
		num = (num * 10) + (advance(r) - '0');
	
	if (num >= INT_MAX)
		raise_syntax_error(ERR_BIG_NUM, r);

	return num;
}

static inline char *getNextWord(struct reader *r) {
	int cap = MAX_WORD_LEN;
	char *str = calloc(cap + 1, sizeof(*str));
	if (!str)
		raise_syntax_error(ERR_NO_MEM, r);

	int len = 0;
	while (r && (isalnum(peek(r)) || (peek(r) == '_'))) {
		str[len++] = (char) advance(r);
		reset_strlen_if_needed(&str, len, &cap, r);
	}
	
	set_strlen(&str, len+1, r);
	str[len] = '\0';
	return str;
}

static inline int getCharacterValue(struct reader *r) {
	int ch = advance(r);
	if (ch == '\'')
		return 0;

	if (advance(r) != '\'')
		raise_syntax_error(ERR_UNMATCHED_QUOTE, r);
	//ERR_MISS_DELIM

	return ch;
}

static inline char catchEscapeChar(struct reader *r) {
	switch (advance(r)) {
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

static inline char *getNextString(struct reader *r) {
	int cap = DEFAULT_CAP_SIZE * 2;
	int len = 0;
	char *buf = calloc(cap, sizeof(*buf));
	if (!buf)
		raise_syntax_error(ERR_NO_MEM, r);

	while (r && (peek(r) != '\"')) {
		if (peek(r) == '\\') {
			advance(r);

			buf[len] = catchEscapeChar(r);
			if (buf[len++] == '\0') {
				free(buf);
				raise_syntax_error(ERR_NO_MEM, r);
			}
		} else {
			buf[len++] = advance(r);
		}
		reset_strlen_if_needed(&buf, len, &cap, r);
	}

	if (advance(r) != '"') {
		free(buf);
		raise_syntax_error(ERR_UNMATCHED_QUOTE, r);
	}
	set_strlen(&buf, len + 1, r);
	buf[len] = '\0';
	return buf;
}

static inline char getNextDelim(struct reader *r) {
	char ch = advance(r);
	if (ch == '/') {
		switch (peek(r)) {
		case '/':
			getSingleLineComment(r);
			return advance(r);
		case '*':
			getMultiLineComment(r);
			return advance(r);
		}
	}
	return ch;
}


// init value functions
static inline void initKeywordValue(enum key_type  key, struct reader *r) {
	if (isTrueFalseKey(key)) {
		r->val.type = VAL_NUM;
		r->val.num = (key == KW_TRUE);
		return;
	}
	r->val.type = VAL_KEYWORD;
	r->val.num = key;
}

static inline void initNameValue(char *str, struct reader *r) {
	enum key_type key = getKeyType(str);
	if (isKeyword(key)) {
		free(str);
		initKeywordValue(key, r);
		return;
	}

	r->val.type = VAL_NAME;
	r->val.str = str;
}

static inline void initOpValue(enum operator op, struct reader *r) {
	r->val.type = VAL_OP;
	r->val.num = op;   
}

static inline void initDelimValue(char delim, struct reader *r) {
	switch (delim) {
	case '\'':
		r->val.num = getCharacterValue(r);
		r->val.type = VAL_NUM;
		return;
	case '"':
		r->val.str = getNextString(r);
		r->val.type = VAL_STR;
		return;
	}

	r->val.type = VAL_DELIM;
	r->val.ch = delim;
}

static inline void initNumValue(int num, struct reader *r) {
	r->val.type = VAL_NUM;
	r->val.num = num;
}

//fetching functions
void nextValue(struct reader *r) {
	skip_spaces(r);

	char ch = peek(r);
	if (ch == EOF || !r)
		return;

	r->val.start_pos = r->line_pos;

	if (isalpha(ch))
		initNameValue(getNextWord(r), r);
	else if (matchesOp(ch))
		initOpValue(getNextOp(r), r);
	else if (isDelim(ch))
		initDelimValue(advance(r), r);
	else if (isdigit(peek(r)))
		initNumValue(getNextNum(r), r);
	else
		raise_syntax_error(ERR_UNEXP_CHAR, r);
}

inline struct value Value(struct reader *r) {
	return r->val;
}

void acceptValue(struct reader *r, enum value_type type, const char *expected) {
	struct value tok = r->val;
	if (!expected)
		raise_syntax_error(ERR_INTERNAL, r);
	if (tok.type != type)
		raise_syntax_error(ERR_INV_VAL, r);
	
	if ((tok.type == VAL_KEYWORD) && strcmp(getKeyStr(tok.num), expected))
		raise_syntax_error(ERR_INV_VAL, r);
    	else if ((tok.type == VAL_DELIM) && (tok.ch != expected[0]))
		raise_syntax_error(ERR_INV_VAL, r);
	nextValue(r);
}


char *stealNextString(struct reader *r) {
	struct value v = r->val;
	if (v.type != VAL_STR)
		raise_syntax_error(ERR_INV_VAL, r);

	char *out = v.str;
	nextValue(r);
	return out;
}

char *stealNextName(struct reader *r) {
	struct value v = r->val;
	if (v.type != VAL_NAME)
		raise_syntax_error(ERR_INV_VAL, r);

	char *out = v.str;
	nextValue(r);
	return out;
}

enum operator stealNextOp(struct reader *r) {
	struct value v = r->val;
	if ((v.type != VAL_OP) || (v.num == OP_UNKNOWN))
		return OP_UNKNOWN;
	enum operator op = v.num;
	nextValue(r);
	return op;
}
