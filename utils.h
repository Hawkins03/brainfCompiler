#ifndef UTILS_H
#define UTILS_H

#define MAX_WORD_LEN 32
#define MAX_NUM_LEN 10
#define DELIMS "=;()[]{}"
#define OPS "+-*/%"
#define BINOP_STARTS "!=<>"
#define BINOP_ENDS "="

#define BINOPS (const char*[]) {"==", "!=", "<", "<=", ">", ">="}
#define BINOPS_COUNT 6
#define KEYWORDS (const char*[]) {"var", "const", "while", "for", "if", "else"}
#define KEYWORDS_COUNT 6

#define raise_error(msg) \
    _raise_error((msg), __func__, __FILE__, __LINE__)

typedef enum {VAL_EMPTY, VAL_NAME, VAL_OP, VAL_BINOP, VAL_NUM, VAL_DELIM, VAL_KEYWORD} ValueType;
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
bool isWordChar(char ch);
bool isOp(char op);
bool isDelim(char delim);
bool isBinOp(const char *binop);
bool isKeyword(const char *keyword);
bool matchesBinop(char ch);
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
char getNextOp(Reader *r);
char getNextDelim(Reader *r);
char *getNextBinOp(char first, Reader *r);
Value *getRawToken(Reader *r);
Value *getToken(Reader *r);
Value *peekToken(Reader *r);
void acceptToken(Value *tok, const char *expected);
void killReader(Reader *r);
bool isAlive(Reader *r);

//error printing:
void _raise_error(const char *msg, const char *file, const char *func, int line);
#endif //UTILS_H
