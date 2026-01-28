/** @file semantics.c
 *  @brief Functions for checking the semantics of the stmt linked list
 * 
 *  This contains the utility functions for
 *  the env struct and the functions to check the semantics
 *  as well as a lot of inlined functions for readability.
 *
 *  @author Hawkins Peterson (hawkins03)
 *  @bug No known bugs.
 */


#include "semantics.h"
#include "parser.h"
#include "structs.h"
#include <string.h>
#include <stdlib.h>

// env utility functions
void setup_env(struct env *env, struct env *parent) {
	env->parent = parent;
	env->root = parent ? parent->root : NULL;
	env->filename = parent ? parent->filename : NULL;
	env->cap = DEFAULT_CAP_SIZE;
	env->vars = calloc(env->cap, sizeof(*env->vars));
	env->len = 0;
	if (!env->vars) {
		if (parent)
			free_stmt(parent->root);
		raise_error(ERR_NO_MEM);
	}
}

static inline void increase_env_len(struct env *env) {
	if (env->len + 1 < env->cap)
		return;
	
	env->cap *= 2;
	struct var_data *tmp = realloc(env->vars, env->cap);
	if (!tmp) {
		free_env(env);
		raise_error(ERR_NO_MEM);
	}
	env->vars = tmp;

}

static void free_child_env(struct env *env) {
	if(!env)
		return;
	if (env->vars) {
		free(env->vars);
		env->vars = NULL;
		env->cap = 0;
		env->len = 0;
	}
}

void free_env(struct env *env) {
	if(!env)
		return;
	if (env->parent) {
		free_env(env->parent);
		env->parent = NULL;
	} else {
		if (env->filename) {
			free(env->filename);
			env->filename = NULL;
		}
		free_stmt(env->root);
		env->root = NULL;
	}
	free(env->vars);
	env->vars = NULL;
	env->cap = 0;
	env->len = 0;
}

//var utility functions
bool define_var(struct env *env, char *name, bool is_mutable, int array_depth) {
	if (!env->vars  || (get_var(env, name) != NULL))
		return false;
	//ERR_REDEF
	struct var_data *var_loc = env->vars + (env->len)++;
	var_loc->name = name;
	increase_env_len(env);
	var_loc->is_mutable = is_mutable;
	var_loc->array_depth = array_depth;
	return true;
}

struct var_data *get_var(const struct env *env, const char *name) {
	if (!env || !name)
		return NULL;
	for (size_t i = 0; i < env->len; i++) {
		struct var_data *var = env->vars + i;
		if ((var) && var->name && (!strcmp(var->name, name)))
			return var;
		}
	return get_var(env->parent, name);
}

static int get_var_depth(struct env *env, const char *name) {
	if (!name)
		return 0;
	const struct var_data *var_data = get_var(env, name);
	if (var_data)
		return var_data->array_depth;
	return 0;
}

// exp utility functions
static char *get_exp_name(const struct exp *exp) {
	if (!exp)
		return NULL;
	
	switch (exp->type) {
	case EXP_ARRAY_REF:
		return get_exp_name(exp->array_ref->name);
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
		char *left_name = get_exp_name(exp->op->left);
		if (left_name)
			return left_name;
		else
			return get_exp_name(exp->op->right);
	case EXP_UNARY:
		return get_exp_name(exp->unary->operand);
	case EXP_NAME:
		return exp->name;
	default:
		return NULL;
	}
}

static inline int get_exp_name_depth(const struct exp *exp) {
	if (exp->type == EXP_ARRAY_REF)
		return get_exp_name_depth(exp->array_ref->name) + 1;
	return 0;
}

static int get_exp_depth(struct env *env, const struct exp *exp) {
	if (!exp)
		return 0;
	switch (exp->type) {
		case EXP_ARRAY_REF:
			int total_depth = get_var_depth(env, get_exp_name(exp));
			int name_depth = get_exp_name_depth(exp->array_ref->name) + 1;
			return total_depth - name_depth;
		case EXP_ARRAY_LIT:
			int max = 0;
			for (int i = 0; i < exp->array_lit->size; i++) {
				int depth = get_exp_depth(env, exp->array_lit->array + i);
				if (depth > max)
					max = depth;
			}
			return max + 1;
		case EXP_UNARY:
			return 0;
		case EXP_ASSIGN_OP:
		case EXP_BINARY_OP:
			return 0;
		case EXP_NAME:
			return get_var_depth(env, exp->name);
		default:
			return 0;
	}
}

static inline bool exp_is_unary(const struct exp  *exp) {
	return exp && ((exp->type == EXP_ARRAY_REF) || (exp->type == EXP_UNARY) || (exp->type == EXP_NAME));
}

static inline bool exp_is_op(const struct exp *exp) {
	return exp && ((exp->type == EXP_UNARY) || (exp->type == EXP_ASSIGN_OP) || (exp->type == EXP_BINARY_OP));
}

static inline bool exp_is_mutable(struct env *env, const struct exp *exp) {
	struct var_data *var = get_var(env, get_exp_name(exp));
	if (!var)
		return false;
		//raise_exp_semantic_error(ERR_NO_VAR, exp, env);
	return var->is_mutable;
}

static inline bool exp_is_arrayLit(const struct exp *exp) {
	return exp && (exp->type == EXP_ARRAY_LIT) && (exp->array_lit->array != NULL);
}

static bool is_array(struct env *env, const struct exp *exp) {
	return get_exp_depth(env, exp) > 0;
}

static inline bool setting_two_arrays(struct env *env, const struct exp *exp) {
	if ((exp->type != EXP_ASSIGN_OP) && (exp->type != EXP_BINARY_OP))
		return false;
	return is_array(env, exp->op->left) && is_array(env, exp->op->right) && (exp->op->op == OP_ASSIGN);
}

static inline void raise_error_if_invalid_depth(struct env *env, struct exp *exp, int depth) {
	
}

static inline void raise_error_if_immutable(struct env *env, const struct exp *exp) {
	struct var_data *vd = get_var(env, get_exp_name(exp));
	if (vd && !vd->is_mutable)
		raise_exp_semantic_error(ERR_IMMUT, exp, env);
}

static inline bool op_must_be_assignable(enum operator op) {
	return ((op == OP_INCREMENT) || (op == OP_DECREMENT));
}

static inline bool is_incrementable(struct env *env, const struct exp *exp) {
	struct var_data *vd = get_var(env, get_exp_name(exp));
	return vd->is_mutable && (vd->array_depth == 0);
}

// checker functions
void check_exp_semantics(struct env *env, struct exp *exp) {
	if (!env || !exp)
		return;

	switch (exp->type) {
	case EXP_NAME:
		struct var_data *var = get_var(env, exp->name);
		if (!var)
			raise_exp_semantic_error(ERR_NO_VAR, exp, env);
		break;
	
	case EXP_ASSIGN_OP:
		struct exp *a_left = exp->op->left;
		struct exp *a_right = exp->op->right;
		raise_error_if_immutable(env, a_left);
		
		if (get_exp_depth(env, a_right) != get_exp_depth(env, a_left))
			raise_exp_semantic_error(ERR_INV_ARR, exp, env);

		check_exp_semantics(env, a_left);
		check_exp_semantics(env, a_right);
		break;
	case EXP_UNARY:
		if (is_array(env, exp->unary->operand))
			raise_exp_semantic_error(ERR_INV_ARR, exp->unary->operand, env);
		
		check_exp_semantics(env, exp->unary->operand);
		break;
	case EXP_BINARY_OP:
		struct exp *b_left = exp->op->left;
		struct exp *b_right = exp->op->right;
		
		if (is_array(env, b_left))
			raise_exp_semantic_error(ERR_INV_ARR, b_left, env);
		else if (is_array(env, b_right))
			raise_exp_semantic_error(ERR_INV_ARR, b_right, env);
		
		check_exp_semantics(env, b_left);
		check_exp_semantics(env, b_right);
		break;
	case EXP_ARRAY_REF:
		if (!is_array(env, exp->array_ref->name))
			raise_exp_semantic_error(ERR_INV_ARR, exp, env);

		check_exp_semantics(env, exp->array_ref->name);
		check_exp_semantics(env, exp->array_ref->index);
		break;
	case EXP_ARRAY_LIT:
		for (int i = 0; i < exp->array_lit->size; i++)
			check_exp_semantics(env, exp->array_lit->array + i);
		break;
	case EXP_CALL:
		if (exp->call->key == KW_PRINT)  { 
			//if(is_array(env, exp->call->arg))
			//	raise_exp_semantic_error(ERR_INV_ARR, exp, env);
			// I'm not sure if this should be an error. For now I'm going to say no, but
			// TODO: confirm this is an error
			check_exp_semantics(env, exp->call->arg);
		}
		break;
	case EXP_NUM:
		break;
	default:
		raise_exp_semantic_error(ERR_INV_EXP, exp, env);
		break;
	}
}

void check_stmt_semantics(struct env *env, struct stmt *stmt) {
	if(!stmt)
		return;

	switch (stmt->type) {
	case STMT_VAR:
		bool is_mutable = stmt->var->is_mutable;
		int depth = get_exp_name_depth(stmt->var->name);
		char *name = get_exp_name(stmt->var->name);

		if (get_exp_depth(env, stmt->var->value) > depth)
			raise_exp_semantic_error(ERR_INV_ARR, stmt->var->value, env);

		check_exp_semantics(env, stmt->var->value);

		if (!define_var(env, name, is_mutable, depth))
			raise_stmt_semantic_error(ERR_REDEF, stmt, env);

		break;
	case STMT_EXPR:
		check_exp_semantics(env, stmt->exp);
		break;
	case STMT_IF:
		if (is_array(env, stmt->ifStmt->cond))
			raise_exp_semantic_error(ERR_INV_ARR, stmt->ifStmt->cond, env);
		check_exp_semantics(env, stmt->ifStmt->cond);

		struct env then_env;
		setup_env(&then_env, env);
		check_stmt_semantics(&then_env, stmt->ifStmt->thenStmt);
		free_child_env(&then_env);

		if (stmt->ifStmt->elseStmt) {
			struct env else_env;
			setup_env(&else_env, env);
			check_stmt_semantics(&else_env, stmt->ifStmt->elseStmt);
			free_child_env(&else_env);
		}
		break;
	case STMT_LOOP:
		if (is_array(env, stmt->loop->cond))
			raise_exp_semantic_error(ERR_INV_ARR, stmt->loop->cond, env);
		check_exp_semantics(env, stmt->loop->cond);
		struct env loop_env;
		setup_env(&loop_env, env);
		check_stmt_semantics(&loop_env, stmt->loop->body);
		free_child_env(&loop_env);
		break;
	default:
		raise_stmt_semantic_error(ERR_INV_STMT, stmt, env);
		break;
	}
	if (stmt->next)
		check_stmt_semantics(env, stmt->next);
}

void check_file_semantics(char *filename) {
	struct stmt *root = parse_file(filename);

	struct env env;
	setup_env(&env, NULL);
	env.root = root;
	env.filename = strdup(filename);
	check_stmt_semantics(&env, root);
	free_env(&env);
}