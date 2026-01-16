#ifndef EXP_H
#define EXP_H

#include <stdbool.h>
#include <string.h>
#include "utils.h"
#include "reader.h"

#define DEFAULT_CAP_SIZE 8

void free_exp(struct exp  *exp);
void print_exp(const struct exp  *exp);

struct exp *init_exp(struct reader *r);
void init_binary(struct reader *r, struct exp *exp, enum exp_type tp);
void init_exp_unary(struct reader *r, struct exp *exp, bool is_prefix);
void init_exp_array_ref(struct reader *r, struct exp *exp);
void init_exp_array_lit(struct reader *r, struct exp *exp, int size);
void init_exp_call(struct reader *r, struct exp *exp, enum key_type key);

void swap_exps(struct exp *from, struct exp *to);
bool exps_match(struct exp  *exp1, struct exp  *exp2);
bool exps_are_compatable(struct exp *exp1, struct exp *exp2);
char *get_name_from_exp(struct exp  *exp);
int get_exp_array_depth(const struct exp *name);
bool is_assignable(const struct exp *exp);
bool is_atomic(const struct exp *exp);
bool parses_to_int(struct exp *exp);
bool exp_is_unary(const struct exp *exp);
bool exp_is_arrayLit(const struct exp *exp);

#endif //EXP_H