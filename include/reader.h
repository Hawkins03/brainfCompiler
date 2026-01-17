#ifndef READER_H
#define READER_H

#include <stdbool.h>
#include "structs.h"

#define DEFAULT_LINE_CAP 256
#define MAX_LINE_LEN 4096
#define MAX_WORD_LEN 32
#define MAX_NUM_LEN 10
#define MAX_OP_LEN 3
#define DELIMS ";()[]{}'\","
#define OP_START "=+-*/%!~<>&^|,"
#define ASSIGN_OP_PRIO 0

#define KEYWORDS (const char*[]) {"var", "val", "while", "for", "if", "else", "print", "input", "break", "true", "false", NULL}
#define KEYWORDS_COUNT 11
#define NUM_PRIOS 10

#define OP_STRINGS (const char *[]) 	{"+", "-", 		\
					"*", "/", "%",		\
					"~", "|", "^", "&", 	\
					"<<", ">>", 		\
					"<", "<=", ">", ">=", 	\
					"==", "!=", 		\
					"!", "&&", "||", 	\
					"=", "+=", "-=", 	\
					"*=", "/=", "%=", 	\
					"<<=", ">>=",		\
					"&=", "^=", "|=", 	\
					"++", "--"}		\

#define PREFIX_OPS (enum operator []) {OP_LOGICAL_NOT, OP_BITWISE_NOT, OP_INCREMENT, OP_DECREMENT, OP_MINUS, OP_UNKNOWN}
#define SUFFIX_OPS (enum operator []) {OP_INCREMENT, OP_DECREMENT, OP_UNKNOWN}
#define ASSIGN_OPS (enum operator []) { OP_ASSIGN, OP_PLUS_ASSIGN, OP_MINUS_ASSIGN, \
    				OP_MULTIPLY_ASSIGN, OP_DIVIDE_ASSIGN, OP_MODULO_ASSIGN, \
    				OP_LEFT_SHIFT_ASSIGN, OP_RIGHT_SHIFT_ASSIGN, \
    				OP_BITWISE_AND_ASSIGN, OP_BITWISE_XOR_ASSIGN, OP_BITWISE_OR_ASSIGN, \
				OP_UNKNOWN}

// reading from the file object
struct reader *readInFile(const char *filename);
void killReader(struct reader *r);

bool readerIsAlive(struct reader *r);
bool hasNextStmt(struct reader *r);
bool atSemicolon(struct reader *r);
bool parserCanProceed(struct reader *r);

// turning enums back into strings for printing
const char *getOpStr(enum operator op);
const char *getKeyStr(enum key_type key);

//checking op values
int getPrio(const enum operator op);
bool isAssignOp(const enum operator op);
bool isBinaryOp(const enum operator op);
bool is_suffix_unary(enum operator op);
bool is_prefix_unary(enum operator op);

void nextValue(struct reader *r);
struct value peekValue(struct reader *r);
void acceptValue(struct reader *r, enum value_type type, const char *expected);

char *stealNextString(struct reader *r);
char *stealNextName(struct reader *r);
enum operator stealNextOp(struct reader *r);

#endif //READER_H