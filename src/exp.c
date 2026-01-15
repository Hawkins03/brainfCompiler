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
	case EXP_EMPTY:
	case EXP_NUM:
		break;
	case EXP_NAME:
		if (!exp->name)
			return;
		free(exp->name);
		exp->name = NULL;
		break;
	case EXP_ARRAY_REF:
		if (!exp->array_ref)
			return;
		free_exp(exp->array_ref->name);
		exp->array_ref->name = NULL;
		free_exp(exp->array_ref->index);
		exp->array_ref->index = NULL;
		free(exp->array_ref);
		exp->array_ref = NULL;
		break;
	case EXP_ARRAY_LIT:
		if (!exp->array_lit)
			return;
		free_array_lit(exp->array_lit->array, exp->array_lit->size);
		exp->array_lit->array = NULL;
		free(exp->array_lit);
		exp->array_lit = NULL;
		break;
	case EXP_ASSIGN_OP:
	case EXP_BINARY_OP:
		if (!exp->op)
			return;
		if (exp->op->op) {
			free(exp->op->op);
			exp->op->op = NULL;
		}
		free_exp(exp->op->left);
		exp->op->left = NULL;
		free_exp(exp->op->right);
		exp->op->right = NULL;
		free(exp->op);
		exp->op = NULL;
		break;
	case EXP_UNARY:
		if (!exp->unary)
			return;
		if (exp->unary->op) {
			free(exp->unary->op);
			exp->unary->op = NULL;
		}
		free_exp(exp->unary->operand);
		exp->unary->operand = NULL;
		free(exp->unary);
		exp->unary = NULL;
		break;
	case EXP_CALL:
		if (!exp->call)
			return;
		free_exp(exp->call->arg);
		exp->call->arg = NULL;
		free(exp->call);
		exp->call = NULL;
		break;
	}
}

//EXP_EMPTY, EXP_NAME, EXP_ARRAY, EXP_NUM, EXP_ASSIGN_OP, EXP_BINARY_OP, EXP_UNARY, EXP_CALL, EXP_RIGHTARRAY, EXP_NESTED
void free_exp(struct exp *exp) {
    	free_exp_contents(exp);
    	free(exp);
}

void free_array_lit(struct exp *array, int size) {
    if (!array)
        return;
    for (int i = 0; i < size; i++)
        free_exp_contents(&array[i]);  // Pass address, not iterate
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
		print_exp(exp->op->left);
		printf(", %s, ", exp->op->op);
		print_exp(exp->op->right);
		printf(")");
		break;
	case EXP_UNARY:
		printf("UNARY(");
		if (exp->unary->is_prefix) {
			printf("%s, ", exp->unary->op);
			print_exp(exp->unary->operand);
			printf(")");
		} else {
			print_exp(exp->unary->operand);
			printf(", %s)\n", exp->unary->op);
		}
		break;
	case EXP_CALL:
		printf("CALL(%s,", getKeyStr(exp->call->key));
		print_exp(exp->call->arg);
		printf(")");
		break;
	case EXP_ARRAY_REF:
		printf("ARR(");
		print_exp(exp->array_ref->name);
		printf(", ");
		print_exp(exp->array_ref->index);
		printf(")");
		break;
	case EXP_ARRAY_LIT:
		printf("ARR_LIT(");
		for(int i = 0; i < exp->array_lit->size; i++) {
			print_exp(exp->array_lit->array + i);
			if (i < exp->array_lit->size - 1)
				printf(", ");
		}
		printf(")");
	}
}

// initialization functions
struct exp *init_exp(struct reader *r) {
	struct exp  *exp = calloc(1, sizeof(*exp));
	
	if (!exp)
		raise_syntax_error("failed to allocate exp", r);

	exp->type = EXP_EMPTY;
	return exp;
}

void init_binary(struct reader *r, struct exp *exp, enum exp_type tp) {
	exp->type = tp;
	exp->op = calloc(1, sizeof(*(exp->op)));
	
	if (!exp->op)
		raise_syntax_error("failed to allocate op", r);
}

void init_exp_unary(struct reader *r, struct exp *exp, bool is_prefix) {
	exp->type = EXP_UNARY;
	exp->unary = calloc(1, sizeof(*(exp->unary)));
	if (!exp->unary)
		raise_syntax_error("failed to allocate unary op", r);
	exp->unary->is_prefix = is_prefix;
}

void init_exp_array_ref(struct reader *r, struct exp *exp) {
	exp->type = EXP_ARRAY_REF;
	exp->array_ref = calloc(1, sizeof(*exp->array_ref));
	
	if (!exp->array_ref)
		raise_syntax_error("failed to allocate array refrence struct", r);
}

void init_exp_array_lit(struct reader *r, struct exp *exp, int size) {
	exp->type = EXP_ARRAY_LIT;
	exp->array_lit = calloc(1, sizeof(*exp->array_lit));
	
	if (!exp->array_lit)
		raise_syntax_error("failed to allocate array literal", r);

	exp->array_lit->array = calloc(size + 1, sizeof(*(exp->array_lit->array)));
	exp->array_lit->size = size;

	if (!exp->array_lit->array) {
		free(exp->array_lit);
		exp->array_lit = NULL;
		raise_syntax_error("failed to allocate array block", r);
	}
	
}

void init_exp_call(struct reader *r, struct exp *exp, enum key_type key) {
	exp->type = EXP_CALL;
	exp->call = calloc(1, sizeof(*exp->call));
	
	if (!exp->call)
		raise_syntax_error("failed to allocate array refrence struct", r);
	exp->call->key = key;
}

void swap_exps(struct exp *from, struct exp *to) {
	struct exp tmp = *from;
	*from = *to;
	*to = tmp;
}

// utility functions
static bool _match_op_exps(const struct exp *exp1, const struct exp *exp2) {
    return exps_match(exp1->op->left, exp2->op->left) &&
           !strcmp(exp1->op->op, exp2->op->op) &&
           exps_match(exp1->op->right, exp2->op->right);
}

bool exps_match(struct exp *exp1, struct exp *exp2) {
    if (!exp1 != !exp2)
        return false;
    else if (exp1 == exp2)
        return true;
    else if (exp1->type != exp2->type)
        return false;

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
        return _match_op_exps(exp1, exp2);
    case EXP_CALL:
        return exp1->call->key == exp2->call->key &&
               exps_match(exp1->call->arg, exp2->call->arg);
    case EXP_ARRAY_REF:
        return exps_match(exp1->array_ref->name, exp2->array_ref->name) &&
               exps_match(exp1->array_ref->index, exp2->array_ref->index);
    case EXP_ARRAY_LIT:
        if (exp1->array_lit->size != exp2->array_lit->size)
            return false;
        for (int i = 0; i < exp1->array_lit->size; i++)
            if (!exps_match(&exp1->array_lit->array[i], 
                            &exp2->array_lit->array[i]))
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
	
	if (exp1->type == EXP_ARRAY_REF)
		return ((exp2->type == EXP_ARRAY_LIT) || (exp2->type == EXP_NAME) || (exp2->type == EXP_ARRAY_REF));
	if (exp1->type == EXP_NAME)
		return (exp2->type != EXP_ARRAY_LIT);

	return false;
}


char *get_name_from_exp(struct exp  *exp) {
	if (!exp)
		return NULL;
	
	switch (exp->type) {
	case EXP_ARRAY_REF:
		return get_name_from_exp(exp->array_ref->name);
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
		char *left_name = get_name_from_exp(exp->op->left);
		if (left_name)
			return left_name;
		else
			return get_name_from_exp(exp->op->right);
	case EXP_UNARY:
		return get_name_from_exp(exp->unary->operand);
	case EXP_NAME:
		return exp->name;
	default:
		return NULL;
	}
}

int get_exp_array_depth(const struct exp *name) {
	if (name && (name->type == EXP_ARRAY_REF))
		return get_exp_array_depth(name->array_ref->name) + 1;
	return 0;
}

bool is_assignable(const struct exp *exp) {
	return (exp) && ((exp->type == EXP_NAME) || (exp->type == EXP_ARRAY_REF));
}

bool is_atomic(const struct exp *exp) {
	if (!exp)
		return false;
	if (is_assignable(exp))
		return true;
	
	switch (exp->type) {
	case EXP_NAME:
		return true;
	case EXP_ARRAY_REF:
		return true;
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
	case EXP_UNARY:
		return (is_atomic(exp->op->left) || is_atomic(exp->op->right));
	default:
		return false;
	}
}

bool parses_to_int(struct exp *exp) {
	if (!exp)
		return false;
	switch (exp->type) {
	case EXP_NAME:
	case EXP_ARRAY_REF:
		return true;
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
	case EXP_UNARY:
		return (parses_to_int(exp->op->left) && parses_to_int(exp->op->right));
	default:
		return false;
	}
}

bool exp_is_unary(const struct exp  *exp) {
	return exp && ((exp->type == EXP_ARRAY_REF) || (exp->type == EXP_UNARY) || (exp->type == EXP_NAME));
}

bool exp_is_op(const struct exp *exp) {
	return exp && ((exp->type == EXP_UNARY) || (exp->type == EXP_ASSIGN_OP) || (exp->type == EXP_BINARY_OP));
}

bool exp_is_arrayLit(const struct exp *exp) {
	return exp && (exp->type == EXP_ARRAY_LIT) && (exp->array_lit->array != NULL);
}