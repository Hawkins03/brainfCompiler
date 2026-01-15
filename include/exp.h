#ifndef EXP_H
#define EXP_H

#include <stdbool.h>
#include <string.h>
#include "utils.h"
#include "reader.h"


void free_exp(struct exp  *exp);
void free_rightArray(struct exp *array, int size);
void print_exp(const struct exp  *exp);

void swap_exps(struct exp *from, struct exp *to);
struct exp *initExpOrKill(struct reader *r);
void init_rightarray(struct exp *op, int size, struct reader *r, struct exp *in);

bool exps_match(struct exp  *exp1, struct exp  *exp2);
bool exps_are_compatable(struct exp *exp1, struct exp *exp2);
char *get_name_from_exp(struct exp  *exp);
int get_exp_array_depth(const struct exp *name);
bool is_assignable(const struct exp *exp);
bool is_atomic(const struct exp *exp);
bool parses_to_int(struct exp *exp);
bool exp_is_array(const struct exp *exp);
bool exp_is_unary(const struct exp *exp);
bool exp_is_nested(const struct exp *exp);

#endif //EXP_H