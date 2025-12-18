#ifndef TEST_H
#define TEST_H

void test_file(const char *input_file, const char *expected);
size_t measure_exp_strlen(const Exp *exp);
void getExpStr(char *out, const Exp *exp);

#endif //TEST_H
