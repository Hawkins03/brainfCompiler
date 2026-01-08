#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdbool.h>

typedef struct stmt stmt_t;


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

#define ALLOC_LIST_START_LEN 8

//TODO: need to have it free r automatically
#define raise_error(msg, r) \
    _raise_error((msg), __func__, __FILE__, __LINE__, r)


#define NUM_PRIOS 13
extern const char *OPS[][12];

typedef enum {
		KW_VAR = 0, KW_VAL, KW_WHILE, KW_FOR, 
		KW_IF, KW_ELSE, KW_PRINT, KW_INPUT, KW_BREAK, 
        KW_TRUE, KW_FALSE
   } key_t;

typedef enum {VAL_NAME, VAL_STR, VAL_OP, VAL_NUM, VAL_DELIM, VAL_KEYWORD} value_type_t;

typedef struct value {
    value_type_t type;
    union {
	char *str;
	int num;
	key_t key;
	char ch;
    };
} value_t;

typedef struct {
    FILE *fp;

    // storage for tokens
    int curr_char;
    value_t *curr_token;

    // for freeing structs
    stmt_t *root;
    stmt_t *curr_stmt;
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
bool isSuffixOp(value_t *tok);
int getPrio(const char *op);
bool isDelim(char delim);
bool isBinOp(const char *binop);
bool isStrType(value_t *v);
bool hasNextStmt(Reader *r);
bool atSemicolon(Reader *r);

value_t *initValue();
void freeValue(value_t *val);
int peek(Reader *r);
int advance(Reader *r);
void skip_spaces(Reader *r);
char *strdup(const char *s, Reader *r);

void printVal(value_t *tok);

char *stealTokString(value_t *tok);

int getNextNum(Reader *r);
char *getNextWord(Reader *r);
key_t getKeyType(char *keyword);
const char *getKeyStr(key_t key);
char *getNextOp(Reader *r);
int getCharacterValue(Reader *r); // i.e. for the input " 'x'; " it returns the ascii value of x.
char getNextDelim(Reader *r);
value_t *getRawToken(Reader *r);

value_t *getToken(Reader *r);
value_t *peekToken(Reader *r);
void acceptToken(Reader *r, value_type_t type, const char *expected);


void _raise_error(const char *msg, const char *func, const char *file, int line, Reader *r);
#endif //UTILS_H
