#ifndef EXP_H
#define EXP_H

#include <stdbool.h>
#include <string.h>
#include "utils.h"

typedef enum {EXP_EMPTY, EXP_STR, EXP_NUM, EXP_OP, EXP_UNARY, EXP_CALL, EXP_ARRAY, EXP_NESTED} exp_type_t;

typedef struct exp {
    exp_type_t type;
    union {//e.as (e.as.name for example).
	char *str;
	int num;
    struct exp *nested; // in this case it's for arrays.
    struct { struct exp *name, *index;} arr;
	struct { key_t key; struct exp *call; } call;
	struct { struct exp *left, *right; char *op; } op;
    };
    void (*print_fn)(const struct exp *exp);
    void (*free_fn)(struct exp *exp);
} exp_t;


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
bool is_array(exp_t *exp);
bool is_unary_exp(exp_t *exp);

#endif //EXP_H