#ifndef TEST_H
#define TEST_H

#include "stmt.h"
#include "exp.h"

int test_file(const char *input_file, const char *expected);
size_t measure_stmt_strlen(const struct stmt *stmt);
size_t measure_exp_strlen(const struct exp  *exp);
void getStmtStr(char *out, const struct stmt *stmt);
void getExpStr(char *out, const struct exp  *exp);

#endif //TEST_H
