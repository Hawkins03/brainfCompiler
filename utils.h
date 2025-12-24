#include <stdio.h>

#ifndef UTILS_H
#define UTILS_H

#define MAX_WORD_LEN 32
#define MAX_NUM_LEN 10
#define MAX_OP_LEN 3
#define DELIMS ";()[]{}'"
#define OP_START "=+-*/%!~<>&^|"
#define RIGHT_ASSOC_PRIO 0
#define UNARY_OP_PRIO 11

//TODO: add ! handling
#define KEYWORDS (const char*[]) {"var", "val", "while", "for", "if", "else", "print", "input", "break", "end"}
#define KEYWORDS_COUNT 8

#define SUFFIX_OPS (const char *[]) {"++", "--"}
#define SUFFIX_OPS_LEN 2

#define raise_error(msg) \
    _raise_error((msg), __func__, __FILE__, __LINE__)


#define NUM_PRIOS 12
extern const char *OPS[][12];


typedef enum {VAL_EMPTY, VAL_STR, VAL_OP, VAL_NUM, VAL_DELIM, VAL_KEYWORD} ValueType;
typedef struct Value {
    ValueType type;
    union {
	char *str;
	int num;
	char ch;
    };
} Value;

typedef struct {
    FILE *fp;
    int curr;
    Value *curr_token;
    bool alive;
} Reader;

//reader struct:
Reader *readInFile(const char *filename);
bool isWordChar(const char ch);
bool isOp(const char *op);
bool matchesOp(const char op);
bool isRightAssoc(int prio);
bool isUnaryOp(const char *op);
bool isSuffixOp(Value *tok);
int getPrio(const char *op);
bool isDelim(char delim);
bool isBinOp(const char *binop);
bool isKeyword(const char *keyword);
bool isStrType(Value *v);
Value *initValue();
void freeValue(Value *val);
void freeValueNoString(Value *val);
int peek(Reader *r);
int advance(Reader *r);
void skip_spaces(Reader *r);
char *stealTokString(Value *tok);
int getNextNum(Reader *r);
char *getNextWord(Reader *r);
char *getNextOp(Reader *r);
char getNextDelim(Reader *r);
char *getNextBinOp(char first, Reader *r);
Value *getRawToken(Reader *r);
Value *getToken(Reader *r);
Value *peekToken(Reader *r);
void acceptToken(Value *tok, ValueType type, const char *expected);
void printVal(Value *tok);
void killReader(Reader *r);
bool isAlive(Reader *r);

//error printing:
void _raise_error(const char *msg, const char *file, const char *func, int line);
#endif //UTILS_H
