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


typedef enum {VAL_EMPTY, VAL_NAME, VAL_OP, VAL_BINOP, VAL_NUM, VAL_DELIM, VAL_KEYWORDS} ValueType;
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
    char curr;
    Value *curr_token;
    long chars_left;
} Reader;

//reader struct:
Reader *readInFile(char *filename);
bool isWordChar(char ch);
bool isOp(char op);
bool isDelim(char delim);
bool isBinOp(char *binop);
bool isKeyword(char *keyword);
bool matchesBinop(char ch);
Value *initValue();
void freeValue(Value *val);
void freeValueNoString(Value *val);
char peek(Reader *r);
char advance(Reader *r);
void accept(Reader *r, char ch2, char *message);
void skip_spaces(Reader *r);
int getNextNum(Reader *r);
char *getNextWord(Reader *r);
char getNextOp(Reader *r);
char getNextDelim(Reader *r);
char *getNextBinOp(char first, Reader *r);
Value *getRawToken(Reader *r);
Value *getToken(Reader *r);
Value *peekToken(Reader *r);
void acceptToken(Reader *r, ValueType type, char *expected, char *file, int line);
void acceptNumToken(Reader *r, int expected, char *file, int line);
void killReader(Reader *r);
bool isAlive(Reader *r);

//error printing:
void raise_error(char *error_message);
void raise_error_and_free(char *error_message, Reader *r);
void raise_error_at_loc(char *file, int line);
#endif //UTILS_H
