#ifndef READER_H
#define READER_H

#include "structs.h"

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

struct reader *readInFile(const char *filename);
void killReader(struct reader *r);

bool readerIsAlive(struct reader *r);
bool hasNextStmt(struct reader *r);
bool atSemicolon(struct reader *r);
bool parserCanProceed(struct reader *r);
bool isValidNameChar(const char ch, struct reader *r);

int peek(struct reader *r);
int advance(struct reader *r);
void skip_spaces(struct reader *r);

char *stealNextString(struct reader *r);
enum operator stealNextOp(struct reader *r);
char getNextDelim(struct reader *r);

void printOpStr(enum operator op);
void getOpStr(enum operator op, char *out);
int getPrio(const enum operator op);
bool isAssignOp(const enum operator op);
bool isBinaryOp(enum operator op);
bool matchesOp(const int op);
bool is_suffix_unary(enum operator op);
bool is_prefix_unary(enum operator op);


enum operator getNextOp(struct reader *r);
int getNextNum(struct reader *r);
char *getNextWord(struct reader *r);
enum key_type getKeyType(char *keyword);
const char *getKeyStr(enum key_type key);
int getCharacterValue(struct reader *r); // i.e. for the input " 'x'; " it returns the ascii value of x.
char *getNextString(struct reader *r);

#endif //READER_H