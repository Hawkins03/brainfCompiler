#include "semantics.h"
#include "parser.h"
#include "structs.h"
#include <string.h>
#include <stdlib.h>

// env utility functions
void setup_env(struct env *env, struct env *parent) {
	env->parent = parent;
	env->root = parent->root;
	env->cap = DEFAULT_CAP_SIZE;
	env->vars = calloc(env->cap, sizeof(*env->vars));
	if (!env->vars)
		raise_semantic_error("failed to allocate space for vars", env);
}

static inline void increase_env_len(struct env *env) {
	if (env->len + 1 < env->cap)
		return;
	
	env->cap *= 2;
	struct var_data *tmp = realloc(env->vars, env->cap);
	if (!tmp)
		raise_semantic_error("failed to allocate space for vars", env);
	env->vars = tmp;

}

void free_env(struct env *env) {
	if(!env)
		return;
	if (env->parent) {
		free_env(env->parent);
		env->parent = NULL;
	} else {
		free_stmt(env->root);
		env->root = NULL;
	}
	free(env->vars);
	env->vars = NULL;
	env->cap = 0;
	env->len = 0;
}

//var utility functions
void declare_var(struct env *env, char *name, bool is_mutable, int array_depth) {
	if (!env->vars || var_exists(env, name))
		raise_semantic_error("variable name already defined", env);

	struct var_data *var_loc = env->vars + (env->len)++;
	var_loc->name = name;
	increase_env_len(env);
	var_loc->is_mutable = is_mutable;
	var_loc->array_depth = array_depth;
}

struct var_data *get_var(const struct env *env, const char *name) {
	if (!env)
		return NULL;
	for (size_t i = 0; i < env->len; i++) {
		struct var_data *var = env->vars + i;
		if ((var) && (!strcmp(var->name, name)))
			return var;
		}
	return get_var(env->parent, name);
}

bool var_exists(const struct env *env, const char *name) {
  	return (get_var(env, name) != NULL);
}

static int get_var_depth(struct env *env, const char *name) {
	if (!name)
		return 0;
	const struct var_data *var_data = get_var(env, name);
	if (var_data)
		return var_data->array_depth;
	raise_semantic_error("failed to find array name", env);
	return 0;
}

static bool is_array_var(struct env *env, const char *name) {
	return get_var_depth(env, name) > 0;
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

static int get_exp_depth(struct env *env, const struct exp *exp) {
	if (!exp)
		return 0;
	switch (exp->type) {
		case EXP_ARRAY_REF:
			return get_var_depth(env, get_exp_name(exp)) - get_exp_depth(env, exp->array_ref->name);
		case EXP_ARRAY_LIT:
			int max = 0;
			for (int i = 0; i < exp->array_lit->size; i++) {
				int depth = get_exp_depth(env, exp->array_lit->array + i);
				if (depth > max)
					max = depth;
			}
			return max + 1;
		case EXP_UNARY:
			return get_exp_depth(env, exp->unary->operand);
		case EXP_ASSIGN_OP:
		case EXP_BINARY_OP:
			int left_depth = get_exp_depth(env, exp->op->left);
			int right_depth = get_exp_depth(env, exp->op->right);
			if (left_depth > right_depth)
				return left_depth;
			else
				return right_depth;
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

static inline bool exp_is_arrayLit(const struct exp *exp) {
	return exp && (exp->type == EXP_ARRAY_LIT) && (exp->array_lit->array != NULL);
}

static bool is_array(struct env *env, const struct exp *exp) {
	if (exp && exp->type == EXP_ARRAY_LIT)
		return true;
	return is_array_var(env, get_exp_name(exp));
}

static inline bool setting_two_arrays(struct env *env, const struct exp *exp) {
	if ((exp->type != EXP_ASSIGN_OP) && (exp->type != EXP_BINARY_OP))
		return false;
	return is_array(env, exp->op->left) && is_array(env, exp->op->right) && (exp->op->op == OP_ASSIGN);
}

static inline void raise_error_if_invalid_depth(struct env *env, struct exp *exp, int depth) {
	if (get_exp_depth(env, exp) > depth)
		raise_semantic_error("invalid array dimension", env);
}

static inline void raise_error_if_immutable(struct env *env, const struct exp *exp) {
	struct var_data *vd = get_var(env, get_exp_name(exp));
	if (vd && !vd->is_mutable)
		raise_semantic_error("attempting to set an immutable variable", env);
}

static inline bool op_must_be_assignable(enum operator op) {
	return ((op == OP_INCREMENT) || (op == OP_DECREMENT));
}

static inline bool is_incrementable(struct env *env, const struct exp *exp) {
	struct var_data *vd = get_var(env, get_exp_name(exp));
	return vd->is_mutable && (vd->array_depth == 0);
}

/** check_stmt_semantics:
 * RULES:
 * 1. All exps must be valid.
 * 2. control flow rules:
 *   2.1: all conds must be int type.
 * 3. declaration rules:
 *   3.1 a variable must be declared before it is used (eg: "x=3; var x;" is invalid)
 *   3.2 no redeclaring variables in the same scope
 *   3.3 symbols declared with val are immutable
 *   3.4 see assignment rules
*/
void check_stmt_semantics(struct env *env, struct stmt *stmt) {
	if(!stmt)
		return;

	if (!env->root && stmt)
		env->root = stmt;

	switch (stmt->type) {
	case STMT_VAR:
		bool is_mutable = stmt->var->is_mutable;
		int array_depth = get_exp_depth(env, stmt->var->name);
		char *name = get_exp_name(stmt->var->name);

		bool lhs_is_array = stmt->var->name->type == EXP_ARRAY_REF;
		bool rhs_is_array = is_array(env, stmt->var->value);

		if (lhs_is_array && (!rhs_is_array && !stmt->var->value))
			raise_semantic_error("arrays can only be defined as arrays", env);
		else if (!lhs_is_array && rhs_is_array)
			raise_semantic_error("only arrays can be defined to be arrays", env);

		declare_var(env, name, is_mutable, array_depth);

		raise_error_if_invalid_depth(env, stmt->var->value, array_depth);

		check_exp_semantics(env, stmt->var->value);
		check_exp_semantics(env, stmt->var->name);
		break;
	case STMT_EXPR:
		check_exp_semantics(env, stmt->exp);
		break;
	case STMT_IF:
		if (is_array(env, stmt->ifStmt->cond))
			raise_semantic_error("conditions can't be an array", env);
		check_exp_semantics(env, stmt->ifStmt->cond);

		struct env then_env;
		setup_env(&then_env, env);
		check_stmt_semantics(&then_env, stmt->ifStmt->thenStmt);
		free_env(&then_env);

		if (stmt->ifStmt->elseStmt) {
			struct env else_env;
			setup_env(&else_env, env);
			check_stmt_semantics(&else_env, stmt->ifStmt->elseStmt);
			free_env(&else_env);

			//todo: merge them.
		}
		break;
	case STMT_LOOP:
		if (is_array(env, stmt->loop->cond))
			raise_error("conditions can't be an array");
		check_exp_semantics(env, stmt->loop->cond);
		struct env loop_env;
		setup_env(&loop_env, env);
		check_stmt_semantics(&loop_env, stmt->loop->body);
		free_env(&loop_env);
		break;
	default:
		raise_semantic_error("invalid statement in semantics", env);
		break;
	}
	check_stmt_semantics(env, stmt->next);
}

 /** check_exp_semantics
  * RULES:
  * 1. symbol and binding rules:
  *   1.1: a variable must be declared before it is used (eg: "x=3; var x;" is invalid)
  *   1.2: no redeclaring variables in the same scope
  *   1.3: symbols declared with val are immutable
  *   1.4: to reffer to a name as an array, it must be declared as an array.
  * 2. assignment rules:
  *   2.1: left hand side must be assigable and mutable.
  *   2.2: ints get assigned to ints, arrays get assigned to arrays
  *   2.3: for name <op>= value, name and value must be ints
  * 3. unary rules:
  *   3.1: for ++, --, and - unary operators, they must eventually refer directly to assignable values.
  * 4. binary op rules:
  *   4.1: all assignments must be ints.
  * 6: print and input take and return int values only.
  */
void check_exp_semantics(struct env *env, struct exp *exp) {
	if (!env)
		return;

	switch (exp->type) {
	case EXP_NAME:
		struct var_data *var = get_var(env, exp->name);
		if (!var)
			raise_semantic_error("variable has not been defined", env);
		if (var->array_depth > 0)
			raise_semantic_error("array is being used as an integer", env);
		break;
	
	case EXP_ASSIGN_OP:
		struct exp *a_left = exp->op->left;
		struct exp *a_right = exp->op->right;
		raise_error_if_immutable(env, a_left);
		if (setting_two_arrays(env, exp)) {
			raise_error_if_invalid_depth(env, a_right, get_exp_depth(env, a_left));
			check_exp_semantics(env, a_left);
			check_exp_semantics(env, a_right);
			return;
		}
		if (is_array(env, a_left) || is_array(env, a_right))
			raise_error("operations can't apply to arrays");
		if (!exp_is_unary(a_left))
			raise_semantic_error("expected a single variable being set.", env);
		check_exp_semantics(env, a_left);
		check_exp_semantics(env, a_right);
		break;
	case EXP_UNARY:
		if (is_array(env, exp->unary->operand))
			raise_semantic_error("unary expressions can't apply to arrays", env);
		if (op_must_be_assignable(exp->unary->op) && !is_incrementable(env, exp->unary->operand))
			raise_semantic_error("++ and -- only work on integer variables", env);
		
		check_exp_semantics(env, exp->unary->operand);
		break;
	case EXP_BINARY_OP:
		struct exp *b_left = exp->op->left;
		struct exp *b_right = exp->op->right;
		if (is_array(env, b_left) || is_array(env, b_right))
			raise_semantic_error("operations can't apply to arrays", env);
		check_exp_semantics(env, b_left);
		check_exp_semantics(env, b_right);
		break;
	case EXP_ARRAY_REF:
		if (is_array(env, exp->array_ref->name))
			raise_semantic_error("can only reffer to arrays as arrays", env);
		check_exp_semantics(env, exp->array_ref->name);
		check_exp_semantics(env, exp->array_ref->index);
		break;
	case EXP_ARRAY_LIT:
		for (int i = 0; i < exp->array_lit->size; i++)
			check_exp_semantics(env, exp->array_lit->array + i);
		break;
	case EXP_CALL:
		if (exp->call->key == KW_PRINT)  { 
			if(is_array(env, exp->call->arg))
				raise_semantic_error("print can only print integer values", env);
			check_exp_semantics(env, exp->call->arg);
		}
		break;
	default:
		raise_semantic_error("invalid exp in semantics", env);
		break;
	}
}

void check_file_semantics(char *filename) {
	struct stmt *root = parse_file(filename);

	struct env env;
	setup_env(&env, NULL);
	env.root = root;
	check_stmt_semantics(&env, root);
	free_env(&env);
}