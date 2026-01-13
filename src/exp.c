#include "parser.h"
#include "utils.h"
#include "exp.h"
#include "value.h"

#include <stdlib.h>
#include <stdbool.h>

void free_exp(struct exp  *exp)
{
    if (!exp)
        return;
    exp->free_fn(exp);
}

void free_empty_exp(struct exp  *exp)
{
    free(exp);
}

void free_str_exp(struct exp  *exp)
{
    free(exp->str);
    exp->str = NULL;
    free(exp);
}

void free_op_exp(struct exp  *exp)
{
    free(exp->op.op);
    exp->op.op = NULL;
    free_exp(exp->op.left);
    exp->op.left = NULL;
    free_exp(exp->op.right);
    exp->op.right = NULL;
    free(exp);
}

void free_call_exp(struct exp  *exp)
{
    free_exp(exp->call.call);
	exp->call.call = NULL;
    free(exp);
}

void free_array_exp(struct exp  *exp)
{
    free_exp(exp->arr.name);
    exp->arr.name = NULL;
    free_exp(exp->arr.index);
    exp->arr.index = NULL;
    free(exp);
}

void free_nested_exp(struct exp  *exp)
{
    free_exp(exp->nested);
	exp->nested = NULL;
    free(exp);
}


void print_exp(const struct exp  *exp)
{
    if (!exp) {
		printf("NULL");
		return;
	}
}

void print_empty_exp(const struct exp  *exp)
{
    if (exp->type == EXP_EMPTY)
        printf("EMPTY()");
}

void print_str_exp(const struct exp  *exp)
{
    printf("NAME(%s)", exp->str);
}

void print_num_exp(const struct exp  *exp)
{
    printf("NUM(%d)", exp->num);
}

void print_op_exp(const struct exp  *exp)
{
    printf("OP( ");
    print_exp(exp->op.left);
    printf(", %s, ", exp->op.op);
    print_exp(exp->op.right);
    printf(")");
}

void print_unary_exp(const struct exp  *exp)
{
    printf("UNARY(");
    print_exp(exp->op.left);
    printf(", %s, \n", exp->op.op);
    print_exp(exp->op.right);
    printf(")");
}

void print_call_exp(const struct exp  *exp)
{
    printf("CALL(%s,", getKeyStr(exp->call.key));
    print_exp(exp->call.call);
    printf(")");
}

void print_array_exp(const struct exp  *exp)
{
    printf("ARR(");
    print_exp(exp->arr.name);
    printf(", ");
    print_exp(exp->arr.index);
    printf(")");
}

void print_nested_exp(const struct exp  *exp)
{
    printf("NESTED(");
    print_exp(exp->nested);
    printf(")");
}


// initialization functions
struct exp  *init_exp()
{
    struct exp  *exp = calloc(1, sizeof(*exp));
	
    if (exp == NULL)
		return NULL;

    exp->type = EXP_EMPTY;
    
    exp->free_fn = free_empty_exp;
    exp->print_fn = print_empty_exp;
    return exp;
}

struct exp  *init_op(struct exp  *left, char *op, struct exp  *right, struct reader *r)
{
    struct exp  *exp = init_exp();
	if (!exp) {
		free_exp(left);
		free(op);
		free_exp(right);
		raise_syntax_error("failed to initialize exp", r);
	}
   
	exp->type = EXP_OP;
    exp->op.left = left;
    exp->op.op = op;
    exp->op.right = right;

    exp->free_fn = free_op_exp;
    exp->print_fn = print_op_exp;
    return exp;
}

struct exp  *init_unary(struct exp  *left, char *op, struct exp  *right, struct reader *r)
{
    struct exp  *exp = init_op(left, op, right, r);
	exp->type = EXP_UNARY;

    exp->free_fn = free_op_exp;
    exp->print_fn = print_unary_exp;
    return exp;
}

struct exp  *init_num(int num, struct reader *r)
{
    struct exp  *exp = init_exp();
	if (!exp)
		raise_syntax_error("failed to initialize exp", r);

    exp->type = EXP_NUM;
    exp->num= num;

    exp->free_fn = free_empty_exp;
    exp->print_fn = print_num_exp;
    return exp;
}

struct exp  *init_str(char *str, struct reader *r)
{
	struct exp  *exp = init_exp();
	if (!exp) {
		free(str);
		raise_syntax_error("failed to initialize exp", r);
	}

    exp->type = EXP_STR;
    exp->str = str;

    exp->free_fn = free_str_exp;
    exp->print_fn = print_str_exp;
    return exp;
}

struct exp  *init_call(enum key_type key, struct exp  *call, struct reader *r)
{
	struct exp  *exp = init_exp();
	if (!exp) {
		free_exp(call);
		raise_syntax_error("failed to initialize exp", r);
	}

	exp->type = EXP_CALL;
	exp->call.key = key;
	exp->call.call = call;

    exp->free_fn = free_call_exp;
    exp->print_fn = print_call_exp;
	return exp;
}

struct exp  *init_array(struct exp  *name, struct exp  *index, struct reader *r)
{
    struct exp  *exp = init_exp(); //TODO: ensure index is an int-type, and name is a str-type
	if (!exp) {
		free_exp(name);
		free_exp(index);
		raise_syntax_error("failed to initialize exp", r);
	}

    exp->type = EXP_ARRAY;
    exp->arr.name = name;
    exp->arr.index = index;

    exp->free_fn = free_array_exp;
    exp->print_fn = print_array_exp;
    return exp;
}

struct exp  *init_nested(struct exp  *op, struct reader *r)
{
	struct exp  *exp = init_exp();
	if (!exp) {
		free_exp(op);
		killreader_t(r);
		raise_syntax_error("failed to initialize exp", r);
	}

	exp->type = EXP_NESTED;
	exp->nested = op;

    exp->free_fn = free_nested_exp;
    exp->print_fn = print_nested_exp;
	return exp;
}


// utility functions
bool exps_match(struct exp  *exp1, struct exp  *exp2)
{
    if (!exp1 != !exp2)
        return false;
    else if (exp1 == exp2) //handles the two pointing to the same address and both being null
        return true;
    else if ((exp1->type != exp2->type)) {
        return false;
    }

	switch (exp1->type) {
    case EXP_EMPTY:
        return true;
    case EXP_STR:
        return !strcmp(exp1->str, exp2->str);
    case EXP_NUM:
        return exp1->num == exp2->num;
    case EXP_UNARY:
    case EXP_OP:
        bool lefts_match = exps_match(exp1->op.left, exp2->op.left);
        bool ops_match = strcmp(exp1->op.op, exp2->op.op);
        bool rights_match = exps_match(exp1->op.right, exp2->op.right);
        return lefts_match && ops_match && rights_match;
    case EXP_CALL:
        bool keys_match = exp1->call.key == exp2->call.key;
        bool calls_match = exps_match(exp1->call.call, exp2->call.call);
        return keys_match && calls_match;
    case EXP_ARRAY:
        bool indexes_match = exp1->arr.index == exp2->arr.index;
        bool names_match = exps_match(exp1->arr.name, exp2->arr.name);
        return indexes_match && names_match;
    case EXP_NESTED:
        return exps_match(exp1->nested, exp2->nested);
    default:
        raise_error("invalid exp type");
        return false;
	}
}

char *get_name_from_exp(struct exp  *exp)
{
	if (!exp)
		return NULL;
	
	switch (exp->type) {
    case EXP_ARRAY:
        return get_name_from_exp(exp->arr.name);
    case EXP_UNARY:
        char *left_name = get_name_from_exp(exp->op.left);
        if (left_name)
            return left_name;
        else
            return get_name_from_exp(exp->op.right);
    case EXP_STR:
        return exp->str;
    default:
        return NULL;
	}
}

bool exp_is_array(struct exp  *exp)
{
	return exp && (exp->type == EXP_ARRAY);
}

bool exp_is_unary(struct exp  *exp)
{
	return (exp_is_array(exp) || (exp->type == EXP_UNARY) || (exp->type == EXP_STR));
}
