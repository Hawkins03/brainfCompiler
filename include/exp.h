#ifndef EXP_H
#define EXP_H

#include <stdbool.h>
#include <string.h>
#include "utils.h"
#include "reader.h"


exp_t *init_exp();
exp_t *init_op(exp_t *left, char *op, exp_t *right, Reader *r);
exp_t *init_unary(exp_t *left, char *op, exp_t *right, Reader *r);
exp_t *init_num(int num, Reader *r);
exp_t *init_str(char *str, Reader *r);
exp_t *init_call(key_t key, exp_t *call, Reader *r);
exp_t *init_array(exp_t *name, exp_t *index, Reader *r);
exp_t *init_nested(exp_t *op, Reader *r);

void free_exp(exp_t *exp);
void print_exp(const exp_t *exp);

bool compare_exps(exp_t *exp1, exp_t *exp2);
char *get_name_from_exp(exp_t *exp);
bool exp_is_array(exp_t *exp);
bool exp_is_unary(exp_t *exp);

#endif //EXP_H