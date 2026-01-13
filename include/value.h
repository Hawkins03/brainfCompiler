#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>
#include "structs.h"

#define MAX_WORD_LEN 32
#define MAX_NUM_LEN 10
#define MAX_OP_LEN 3
#define DELIMS ";()[]{}'\""
#define OP_START "=+-*/%!~<>&^|,"
#define UNARY_OP_PRIO 12
#define ADD_OP_PRIO 10
#define SET_OP_PRIO 0

#define KEYWORDS (const char*[]) {"var", "val", "while", "for", "if", "else", "print", "input", "break", "true", "false", NULL}
#define KEYWORDS_COUNT 11

#define SUFFIX_OPS (const char *[]) {"++", "--"}
#define SUFFIX_OPS_LEN 2

#define NUM_PRIOS 13
extern const char *OPS[][12];

struct value *initValue();
void freeValue(struct value *val);

bool isWordChar(const int ch);
int getPrio(const char *op);
bool isOp(const char *op);
bool isRightAssoc(const int prio);
bool matchesOp(const int op);
bool isUnaryOp(const char *op);
bool isSuffixOp(struct value *tok);
bool isKeyword(const enum key_type key);
bool isDelim(const int delim);
bool isStrType(struct value *v);
bool isValidOp(const struct value *op, const int minPrio);

struct value *getRawValue(struct reader *r);
struct value *getValue(struct reader *r);
struct value *peekValue(struct reader *r);
void acceptValue(struct reader *r, enum value_type type, const char *expected);


#endif //VALUE_H