#ifndef EXP_H
#define EXP_H

#include <stdbool.h>
#include <string.h>
#include "utils.h"
#include "reader.h"


struct exp  *init_exp();
struct exp  *init_op(struct exp  *left, char *op, struct exp  *right, struct reader *r);
struct exp  *init_unary(struct exp  *left, char *op, struct exp  *right, struct reader *r);
struct exp  *init_num(int num, struct reader *r);
struct exp  *init_str(char *str, struct reader *r);
struct exp  *init_call(enum key_type key, struct exp  *call, struct reader *r);
struct exp  *init_array(struct exp  *name, struct exp  *index, struct reader *r);
struct exp  *init_nested(struct exp  *op, struct reader *r);

void free_exp(struct exp  *exp);
void print_exp(const struct exp  *exp);

bool exps_match(struct exp  *exp1, struct exp  *exp2);
char *get_name_from_exp(struct exp  *exp);
bool exp_is_array(struct exp  *exp);
bool exp_is_unary(struct exp  *exp);

#endif //EXP_H