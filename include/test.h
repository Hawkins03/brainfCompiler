#ifndef TEST_H
#define TEST_H

#include "stmt.h"
#include "exp.h"

int test_file(const char *input_file, const char *expected);
size_t measure_stmt_strlen(const stmt_t *stmt);
size_t measure_exp_strlen(const exp_t *exp);
void getStmtStr(char *out, const stmt_t *stmt);
void getExpStr(char *out, const exp_t *exp);

#endif //TEST_H
