#include "parser.h"
#include "utils.h"
#include "exp.h"
#include "value.h"

#include <stdlib.h>
#include <stdbool.h>

void free_exp_contents(struct exp *exp) {
	if (!exp)
		return;
	switch (exp->type) {
	case EXP_NAME:
		free(exp->name);
		exp->name = NULL;
		break;
	case EXP_ARRAY:
		free_exp(exp->array.name);
		exp->array.name = NULL;
		free_exp(exp->array.index);
		exp->array.index = NULL;
		break;
	case EXP_ASSIGN_OP:
	case EXP_BINARY_OP:
	case EXP_UNARY:
		free(exp->op.op);
		exp->op.op = NULL;
		free_exp(exp->op.left);
		exp->op.left = NULL;
		free_exp(exp->op.right);
		exp->op.right = NULL;
		break;
	case EXP_CALL:
		free_exp(exp->call.call);
		exp->call.call = NULL;
		break;
	case EXP_RIGHTARRAY:
		free_rightArray(exp->right_array.array, exp->right_array.size);
		exp->right_array.array = NULL;
	}
}

//EXP_EMPTY, EXP_NAME, EXP_ARRAY, EXP_NUM, EXP_ASSIGN_OP, EXP_BINARY_OP, EXP_UNARY, EXP_CALL, EXP_RIGHTARRAY, EXP_NESTED
void free_exp(struct exp *exp) {
    	free_exp_contents(exp);
    	free(exp);
}

void free_rightArray(struct exp *array, int size) {
	if (!array)
		return;
	for (int i = 0; i < size; i++)
		free_exp_contents(array);
	free(array);
}

void print_exp(const struct exp *exp)
{
    	if (!exp) {
		printf("NULL");
		return;
	}
	switch (exp->type) {
	case EXP_EMPTY:
		printf("EMPTY()");
		break;
	case EXP_NAME:
		printf("NAME(%s)", exp->name);
		break;
	case EXP_NUM:
		printf("NUM(%d)", exp->num);
		break;
	case EXP_ASSIGN_OP:
	case EXP_BINARY_OP:
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
		print_exp(exp->array.name);
		printf(", ");
		print_exp(exp->array.index);
		printf(")");
		break;
	case EXP_RIGHTARRAY:
		printf("RIGHTARRAY(");
		for(int i = 0; i < exp->right_array.size; i++) {
			print_exp(exp->right_array.array + i);
			if (i < exp->right_array.size - 1)
				printf(", ");
		}
		printf(")");
	}
}

void reinit_exp(struct exp *from, struct exp *to, struct reader *r) {
	switch (from->type) {
	case EXP_NAME:
		init_name(from->name, to);
		break;
	case EXP_NUM:
		init_num(from->num, to);
		break;
	case EXP_ARRAY:
		init_array(from->array.name, from->array.index, to);
		break;
	case EXP_CALL:
		init_call(from->call.key, from->call.call, to);
		break;
	case EXP_ASSIGN_OP:
		init_assignop(from->op.left, from->op.op, from->op.right, to);
		break;
	case EXP_BINARY_OP:
		init_binop(from->op.left, from->op.op, from->op.right, to);
		break;
	case EXP_UNARY:
		init_unary(from->op.left, from->op.op, from->op.right, to);
		break;
	case EXP_RIGHTARRAY:
		init_rightarray(from->right_array.array, from->right_array.size, r, to);
		break;
	default:
		return;
	}
}

// requires to_free to end in NULL
struct exp *init_exp_or_free(struct reader *r, struct exp **exps) {
	struct exp *e = init_exp();
	if (e)
        	return e;

	if (exps)
		for (size_t i = 0; exps[i] != NULL; ++i)
			free_exp(exps[i]);

	raise_syntax_error("failed to allocate exp", r);
	return NULL;
}

struct exp *init_exp_or_free_str(struct reader *r, struct exp **exps, char *str) {
	struct exp *e = init_exp();
	if (e)
        	return e;

	if (str)
		free(str);

	if (exps)
		for (size_t i = 0; exps[i] != NULL; ++i)
			free_exp(exps[i]);

	raise_syntax_error("failed to allocate exp", r);
	return NULL;
}


// initialization functions
struct exp *init_exp()
{
	struct exp  *exp = calloc(1, sizeof(*exp));
	
	if (exp == NULL)
		return NULL;

	exp->type = EXP_EMPTY;
	return exp;
}

void init_binop(struct exp  *left, char *op, struct exp  *right, struct exp *in)
{
	if (!in)
		return;
	in->type = EXP_BINARY_OP;
	in->op.left = left;
	in->op.op = op;
	in->op.right = right;
}

void init_assignop(struct exp  *left, char *op, struct exp  *right, struct exp *in) {
	init_binop(left, op, right, in);
	in->type = EXP_ASSIGN_OP;
}

void init_unary(struct exp  *left, char *op, struct exp  *right, struct exp *in)
{
	init_binop(left, op, right, in);
	in->type = EXP_UNARY;
}

void init_num(int num, struct exp *in)
{
	in->type = EXP_NUM;
	in->num = num;
}

void init_name(char *name, struct exp *in)
{
	in->type = EXP_NAME;
	in->name = name;
}

void init_call(enum key_type key, struct exp  *call, struct exp *in)
{
	in->type = EXP_CALL;
	in->call.key = key;
	in->call.call = call;
}

void init_array(struct exp  *name, struct exp  *index, struct exp *in)
{
	in->type = EXP_ARRAY;
	in->array.name = name;
	in->array.index = index;
}

void init_rightarray(struct exp* array, int size, struct reader *r, struct exp *in)
{
	in->type = EXP_RIGHTARRAY;
	if (array)
		in->right_array.array = array;
	else
		in->right_array.array = calloc(size + 1, sizeof(*in->right_array.array));
	in->right_array.size = size;
	if (!in->right_array.array) {
		free_rightArray(array, size);
		raise_syntax_error("failed to initialize exp array", r);
	}
}

// utility functions
bool exps_match(struct exp *exp1, struct exp  *exp2) {
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
	case EXP_NAME:
		return !strcmp(exp1->name, exp2->name);
	case EXP_NUM:
		return exp1->num == exp2->num;
	case EXP_UNARY:
	case EXP_ASSIGN_OP:
	case EXP_BINARY_OP:
		bool lefts_match = exps_match(exp1->op.left, exp2->op.left);
		bool ops_match = strcmp(exp1->op.op, exp2->op.op);
		bool rights_match = exps_match(exp1->op.right, exp2->op.right);
		return lefts_match && ops_match && rights_match;
	case EXP_CALL:
		bool keys_match = exp1->call.key == exp2->call.key;
		bool calls_match = exps_match(exp1->call.call, exp2->call.call);
		return keys_match && calls_match;
	case EXP_ARRAY:
		bool indexes_match = exp1->array.index == exp2->array.index;
		bool names_match = exps_match(exp1->array.name, exp2->array.name);
		return indexes_match && names_match;
	case EXP_RIGHTARRAY:
		if (exp1->right_array.size != exp2->right_array.size)
			return false;
		for (int i = 0; i < exp1->right_array.size; i++)
			if (!exps_match(exp1->right_array.array + i, exp2->right_array.array + i))
				return false;
		return true;
	default:
		raise_error("invalid exp type");
		return false;
	}
}

bool exps_are_compatable(struct exp *exp1, struct exp *exp2) {
	if (!exp1 != !exp2)
		return false;
	
	if (exp1 == exp2) //handles the two pointing to the same address and both being null
		return true;
	
	if (exp1->type == EXP_ARRAY)
		return ((exp2->type == EXP_RIGHTARRAY) || (exp2->type == EXP_NAME) || (exp2->type == EXP_ARRAY));
	if (exp1->type == EXP_NAME)
		return (exp2->type != EXP_RIGHTARRAY);


	return false;
}


char *get_name_from_exp(struct exp  *exp) {
	if (!exp)
		return NULL;
	
	switch (exp->type) {
	case EXP_ARRAY:
		return get_name_from_exp(exp->array.name);
	case EXP_UNARY:
		char *left_name = get_name_from_exp(exp->op.left);
		if (left_name)
			return left_name;
		else
			return get_name_from_exp(exp->op.right);
	case EXP_NAME:
		return exp->name;
	default:
		return NULL;
	}
}

int get_exp_array_depth(const struct exp *name) {
	if (name && (name->type == EXP_ARRAY))
		return get_exp_array_depth(name->array.name) + 1;
	return 0;
}

bool is_assignable(const struct exp *exp) {
	return (exp) && ((exp->type == EXP_NAME) || (exp->type == EXP_ARRAY));
}

bool is_atomic(const struct exp *exp) {
	if (!exp)
		return false;
	if (is_assignable(exp))
		return true;
	
	switch (exp->type) {
	case EXP_NAME:
		return true;
	case EXP_ARRAY:
		return true;
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
	case EXP_UNARY:
		return (is_atomic(exp->op.left) || is_atomic(exp->op.right));
	default:
		return false;
	}
}

bool parses_to_int(struct exp *exp) {
	if (!exp)
		return false;
	switch (exp->type) {
	case EXP_NAME:
		return true;
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
	case EXP_UNARY:
		return (parses_to_int(exp->op.left) && parses_to_int(exp->op.right));
	default:
		return false;
	}
}

bool exp_is_array(const struct exp  *exp) {
	return exp && (exp->type == EXP_ARRAY);
}

bool exp_is_unary(const struct exp  *exp) {
	return (exp_is_array(exp) || (exp->type == EXP_UNARY) || (exp->type == EXP_NAME));
}

bool exp_is_op(const struct exp *exp) {
	return exp && ((exp->type == EXP_UNARY) || (exp->type == EXP_ASSIGN_OP) || (exp->type == EXP_BINARY_OP));
}

bool exp_is_rightArray(const struct exp *exp) {
	return exp && (exp->type == EXP_RIGHTARRAY) && (exp->right_array.array != NULL);
}