#include "parser.h"
#include "utils.h"
#include "exp.h"
#include "value.h"

#include <stdlib.h>
#include <stdbool.h>
void free_exp(struct exp *exp) {
    	if (exp == NULL)
		return;
    	switch (exp->type) {
	case EXP_STR:
		free(exp->str);
		exp->str = NULL;
		break;
	case EXP_NUM:
		break;
	case EXP_UNARY:
	case EXP_OP:
		free(exp->op.op);
		exp->op.op = NULL;
		free_exp(exp->op.left);
		exp->op.left = NULL;
		free_exp(exp->op.right);
		exp->op.right = NULL;
		break;
	case EXP_EMPTY:
		break;
	case EXP_CALL:
		free_exp(exp->call.call);
		exp->call.call = NULL;
		break;
	case EXP_ARRAY:
		free_exp(exp->arr.name);
		exp->arr.name = NULL;
		free_exp(exp->arr.index);
		exp->arr.index = NULL;
		break;
	case EXP_NESTED:
		free_exp(exp->nested);
		exp->nested = NULL;
		break;
    	}
    	free(exp);
}

void print_exp(const struct exp *exp)
{
    	if (!exp) {
		printf("NULL");
		return;
	}
	switch (exp->type) {
	case EXP_STR:
		printf("NAME(%s)", exp->str);
		break;
	case EXP_NUM:
		printf("NUM(%d)", exp->num);
		break;
	case EXP_OP:
		printf("OP( ");
		print_exp(exp->op.left);
		printf(", %s, ", exp->op.op);
		print_exp(exp->op.right);
		printf(")");
		break;
	case EXP_UNARY:
		printf("UNARY(");
		print_exp(exp->op.left);
		printf(", %s, \n", exp->op.op);
		print_exp(exp->op.right);
		printf(")");
		break;
	case EXP_CALL:
		printf("CALL(%s,", getKeyStr(exp->call.key));
		print_exp(exp->call.call);
		printf(")");
		break;
	case EXP_ARRAY:
		printf("ARR(");
		print_exp(exp->arr.name);
		printf(", ");
		print_exp(exp->arr.index);
		printf(")");
		break;
	case EXP_NESTED:
		printf("NESTED(");
		print_exp(exp->nested);
		printf(")");
		break;
	case EXP_EMPTY:
		printf("EMPTY()");
		break;
    	}   
}

// initialization functions
struct exp  *init_exp()
{
	struct exp  *exp = calloc(1, sizeof(*exp));
	
	if (exp == NULL)
		return NULL;

	exp->type = EXP_EMPTY;
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
	return exp;
}

struct exp  *init_unary(struct exp  *left, char *op, struct exp  *right, struct reader *r)
{
	struct exp  *exp = init_op(left, op, right, r);
	exp->type = EXP_UNARY;
	return exp;
}

struct exp  *init_num(int num, struct reader *r)
{
	struct exp  *exp = init_exp();
	if (!exp)
		raise_syntax_error("failed to initialize exp", r);

	exp->type = EXP_NUM;
	exp->num = num;
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
	return exp;
}

struct exp  *init_nested(struct exp  *op, struct reader *r)
{
	struct exp  *exp = init_exp();
	if (!exp) {
		free_exp(op);
		killReader(r);
		raise_syntax_error("failed to initialize exp", r);
	}

	exp->type = EXP_NESTED;
	exp->nested = op;
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
