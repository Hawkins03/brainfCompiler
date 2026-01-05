#ifndef TEST_H
#define TEST_H

int test_file(const char *input_file, const char *expected);
size_t measure_stmt_strlen(const Stmt *stmt);
size_t measure_exp_strlen(const Exp *exp);
void getStmtStr(char *out, const Stmt *stmt);
void getExpStr(char *out, const Exp *exp);

#endif //TEST_H
