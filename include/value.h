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

value_t *initValue();
void print_value(const value_t *val);
void free_value(value_t *val);

bool isWordChar(const int ch);
int get_prio(const char *op);
bool isOp(const char *op);
bool isRightAssoc(const int prio);
bool matchesOp(const int op);
bool isUnaryOp(const char *op);
bool isSuffixOp(value_t *tok);
bool isValidKey(const key_t key);
bool isDelim(const int delim);
bool isStrType(value_t *v);


char *stealTokString(value_t *tok);

#endif //VALUE_H