#ifndef VALUE_H
#define VALUE_H

#include <stdbool.h>
#include "structs.h"

#define MAX_WORD_LEN 32
#define MAX_NUM_LEN 10
#define MAX_OP_LEN 3
#define DELIMS ";()[]{}'\","
#define OP_START "=+-*/%!~<>&^|,"
#define ASSIGN_OP_PRIO 0

#define KEYWORDS (const char*[]) {"var", "val", "while", "for", "if", "else", "print", "input", "break", "true", "false", NULL}
#define KEYWORDS_COUNT 11

#define PREFIX_OPS (const char *[]) {"!", "~", "++", "--", "-", NULL}
#define SUFFIX_OPS (const char *[]) {"++", "--", NULL}
#define ASSIGN_OPS (const char *[]) {"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|=", NULL}
#define NUM_PRIOS 10


struct value *initValue();
void freeValue(struct value *val);

int getPrio(const char *op);
bool isAssignOp(const char *op);
bool isOp(const char *op);
bool matchesOp(const int op);
bool isOpType(const struct value *v);
bool isSuffixVal(const struct value *v);
bool isPrefixVal(struct value *v);
bool isBinaryOpVal(struct value *v);
bool isAssignOpVal(struct value *v);
bool isValidOp(struct value *v, int minPrio);

bool isWordChar(const int ch);
bool isKeyword(const enum key_type key);
bool isElseKey(const struct value *val);
bool isTrueFalseKey(const enum key_type  key);
bool isDelim(const int delim);
bool isDelimType(const struct value *v);
bool isDelimChar(const struct value *v, char match);
bool isStrType(struct value *v);

bool isValidOp(struct value *v, const int minPrio);

struct value *getRawValue(struct reader *r);
struct value *getValue(struct reader *r);
struct value *peekValue(struct reader *r);
void acceptValue(struct reader *r, enum value_type type, const char *expected);

#endif //VALUE_H