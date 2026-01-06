#include <stdio.h>

#ifndef UTILS_H
#define UTILS_H

#define MAX_WORD_LEN 32
#define MAX_NUM_LEN 10
#define MAX_OP_LEN 3
#define DELIMS ";()[]{}'\""
#define OP_START "=+-*/%!~<>&^|,"
#define RIGHT_ASSOC_PRIO 0
#define UNARY_OP_PRIO 12

//TODO: add ! handling
#define KEYWORDS (const char*[]) {"var", "val", "while", "for", "if", "else", "print", "input", "break", "true", "false", NULL}
#define KEYWORDS_COUNT 11

#define SUFFIX_OPS (const char *[]) {"++", "--"}
#define SUFFIX_OPS_LEN 2

//TODO: need to have it free r automatically
#define raise_error(msg) \
    _raise_error((msg), __func__, __FILE__, __LINE__)


#define NUM_PRIOS 13
extern const char *OPS[][12];

typedef enum {
		KW_INVALID = -1, //TODO: determine if needed
		KW_VAR = 0, KW_VAL, KW_WHILE, KW_FOR, 
		KW_IF, KW_ELSE, KW_PRINT, KW_INPUT, KW_BREAK, 
        KW_TRUE, KW_FALSE
   } KeyType;
typedef enum {VAL_EMPTY, VAL_NAME, VAL_STR, VAL_OP, VAL_NUM, VAL_DELIM, VAL_KEYWORD} ValueType;
typedef struct Value {
    ValueType type;
    union {
	char *str;
	int num;
	KeyType key;
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
bool isAlive(Reader *r);
Reader *readInFile(const char *filename);
void killReader(Reader *r);

bool isWordChar(const char ch);
bool isOp(const char *op);
bool matchesOp(const char op);
bool isRightAssoc(int prio);
bool isUnaryOp(const char *op);
bool isSuffixOp(Value *tok);
int getPrio(const char *op);
bool isDelim(char delim);
bool isBinOp(const char *binop);
bool isStrType(Value *v);
bool hasNextStmt(Reader *r);

Value *initValue();
void freeValue(Value *val);
int peek(Reader *r);
int advance(Reader *r);
void skip_spaces(Reader *r);
char *strdup(const char *s);

void printVal(Value *tok);

char *stealTokString(Value *tok);

int getNextNum(Reader *r);
char *getNextWord(Reader *r);
KeyType getKeyType(char *keyword);
const char *getKeyStr(KeyType key);
char *getNextOp(Reader *r);
int getCharacterValue(Reader *r); // i.e. for the input " 'x'; " it returns the ascii value of x.
char getNextDelim(Reader *r);
Value *getRawToken(Reader *r);

Value *getToken(Reader *r);
Value *peekToken(Reader *r);
void acceptToken(Reader *r, ValueType type, const char *expected);


void _raise_error(const char *msg, const char *file, const char *func, int line);
#endif //UTILS_H
