#include "parser.h"
#include "utils.h"
#include "exp.h"

#include <stdlib.h>
#include <stdbool.h>

void free_exp(exp_t *exp) {
    if (!exp)
        return;
    exp->free_fn(exp);
}

void free_empty_exp(exp_t *exp) {
    free(exp);
}

void free_str_exp(exp_t *exp) {
    free(exp->str);
    exp->str = NULL;
    free(exp);
}

void free_op_exp(exp_t *exp) {
    free(exp->op.op);
    exp->op.op = NULL;
    free_exp(exp->op.left);
    exp->op.left = NULL;
    free_exp(exp->op.right);
    exp->op.right = NULL;
    free(exp);
}

void free_call_exp(exp_t *exp) {
    free_exp(exp->call.call);
	exp->call.call = NULL;
    free(exp);
}

void free_array_exp(exp_t *exp) {
    free_exp(exp->arr.name);
    exp->arr.name = NULL;
    free_exp(exp->arr.index);
    exp->arr.index = NULL;
    free(exp);
}

void free_nested_exp(exp_t *exp) {
    free_exp(exp->nested);
	exp->nested = NULL;
    free(exp);
}


void print_exp(const exp_t *exp) {
    if (!exp) {
		printf("NULL");
		return;
	}
}

void print_empty_exp(const exp_t *exp) {
    if (exp->type == EXP_EMPTY)
        printf("EMPTY()");
}

void print_str_exp(const exp_t *exp) {
    printf("NAME(%s)", exp->str);
}

void print_num_exp(const exp_t *exp) {
    printf("NUM(%d)", exp->num);
}

void print_op_exp(const exp_t *exp) {
    printf("OP( ");
    print_exp(exp->op.left);
    printf(", %s, ", exp->op.op);
    print_exp(exp->op.right);
    printf(")");
}

void print_unary_exp(const exp_t *exp) {
    printf("UNARY(");
    print_exp(exp->op.left);
    printf(", %s, \n", exp->op.op);
    print_exp(exp->op.right);
    printf(")");
}

void print_call_exp(const exp_t *exp) {
    printf("CALL(%s,", getKeyStr(exp->call.key));
    print_exp(exp->call.call);
    printf(")");
}

void print_array_exp(const exp_t *exp) {
    printf("ARR(");
    print_exp(exp->arr.name);
    printf(", ");
    print_exp(exp->arr.index);
    printf(")");
}

void print_nested_exp(const exp_t *exp) {
    printf("NESTED(");
    print_exp(exp->nested);
    printf(")");
}


// initialization functions
exp_t *init_exp() {
    exp_t *exp = calloc(1, sizeof(*exp));
	
    if (exp == NULL)
		return NULL;

    exp->type = EXP_EMPTY;
    
    exp->free_fn = free_empty_exp;
    exp->print_fn = print_empty_exp;
    return exp;
}

exp_t *init_op(exp_t *left, char *op, exp_t *right, Reader *r) {
    exp_t *exp = init_exp();
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

exp_t *init_unary(exp_t *left, char *op, exp_t *right, Reader *r) {
    exp_t *exp = init_op(left, op, right, r);
	exp->type = EXP_UNARY;

    exp->free_fn = free_op_exp;
    exp->print_fn = print_unary_exp;
    return exp;
}

exp_t *init_num(int num, Reader *r) {
    exp_t *exp = init_exp();
	if (!exp)
		raise_syntax_error("failed to initialize exp", r);

    exp->type = EXP_NUM;
    exp->num= num;

    exp->free_fn = free_empty_exp;
    exp->print_fn = print_num_exp;
    return exp;
}

exp_t *init_str(char *str, Reader *r) {
	exp_t *exp = init_exp();
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

exp_t *init_call(key_t key, exp_t *call, Reader *r) {
	exp_t *exp = init_exp();
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

exp_t *init_array(exp_t *name, exp_t *index, Reader *r) {
    exp_t *exp = init_exp(); //TODO: ensure index is an int-type, and name is a str-type
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

exp_t *init_nested(exp_t *op, Reader *r) {
	exp_t *exp = init_exp();
	if (!exp) {
		free_exp(op);
		killReader(r);
		raise_syntax_error("failed to initialize exp", r);
	}

	exp->type = EXP_NESTED;
	exp->nested = op;

    exp->free_fn = free_nested_exp;
    exp->print_fn = print_nested_exp;
	return exp;
}


// utility functions
bool compare_exps(exp_t *exp1, exp_t *exp2) {
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
            bool lefts_match = compare_exps(exp1->op.left, exp2->op.left);
            bool ops_match = strcmp(exp1->op.op, exp2->op.op);
            bool rights_match = compare_exps(exp1->op.right, exp2->op.right);
            return lefts_match && ops_match && rights_match;
		case EXP_CALL:
            bool keys_match = exp1->call.key == exp2->call.key;
            bool calls_match = compare_exps(exp1->call.call, exp2->call.call);
            return keys_match && calls_match;
		case EXP_ARRAY:
            bool indexes_match = exp1->arr.index == exp2->arr.index;
            bool names_match = compare_exps(exp1->arr.name, exp2->arr.name);
            return indexes_match && names_match;
		case EXP_NESTED:
			return compare_exps(exp1->nested, exp2->nested);
		default:
			raise_error("invalid exp type");
	}
    return false;
}

char *get_name_from_exp(exp_t *exp) {
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

bool exp_is_array(exp_t *exp) {
	return exp && (exp->type == EXP_ARRAY);
}

bool exp_is_unary(exp_t *exp) {
	return (exp_is_array(exp) || (exp->type == EXP_UNARY) || (exp->type == EXP_STR));
}
