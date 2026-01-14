#include "semantics.h"
#include "parser.h"
#include "structs.h"
#include "value.h"
#include "exp.h"
#include "stmt.h"
#include <string.h>
#include <stdlib.h>



struct env *init_env(struct env *parent) {
	struct env *env = calloc(1, sizeof(*env));
	if (!env)
		return NULL;
	env->parent = parent;
	return env;
}

void free_env(struct env *env) {
	if (!env)
		return;
	free_env(env->parent);
	free(env);
}

void declare_var(struct env *env, char *name, bool is_defined, bool is_mutable, int array_depth) {
	if (var_exists(env, name))
		raise_semantic_error("variable name already defined", env);

	struct var_data *var_loc = env->vars + (env->count)++;
	var_loc->name = name;
	var_loc->is_defined = is_defined;
	var_loc->is_mutable = is_mutable;
	var_loc->array_depth = array_depth;
}

void define_var(struct env *env, char *name) {
	struct var_data *var = get_var(env, name);
	if (!var->is_mutable && var->is_defined)
		raise_semantic_error("user is attempting to reassign an immutable variable", env);
	var->is_defined = true;
}

struct var_data *get_var(const struct env *env, const char *name) {
	if (!env)
		return NULL;
	for (size_t i = 0; i < env->count; i++) {
		struct var_data *var = env->vars + i;
		if ((var) && (!strcmp(var->name, name)))
			return var;
		}
	return get_var(env->parent, name);
}

bool var_exists(const struct env *env, const char *name) {
  	return (get_var(env, name) != NULL);
}

int get_var_depth(struct env *env, const char *name) {
	const struct var_data *var_data = get_var(env, name);
	if (var_data)
		return var_data->array_depth;
	raise_semantic_error("failed to find array name", env);
	return 0;
}

bool is_array_var(struct env *env, const char *name) {
	return get_var_depth(env, name) > 0;
}



/** RULES:
 * 1. All exps must be valid.
 * 2. control flow rules:
 *   2.1: all conds must be int type.
 *   2.2: break is only legal in while and for loops
 * 3. declaration rules: - see exp rules
*/
void check_stmt_semantics(struct env *env, struct stmt *stmt) {
	if (!env->root && stmt)
		env->root = stmt;

	switch (stmt->type) {
	case STMT_VAR:
	case STMT_VAL:
		bool is_defined = (stmt->var.value != NULL);
		bool is_mutable = (stmt->type == STMT_VAR);
		int array_depth = get_exp_array_depth(stmt->var.name);
		//TODO: check value so that the array depths match.
		if (!(array_depth ^ (stmt->var.value->type != EXP_RIGHTARRAY))) // ^ = xor, so basically means no array and no 
			raise_semantic_error("you can't set an array to an integer", env);
		
		//TODO: rework so that each level of array gets its own level, and the minimum level is treated as not an array.
		//TODO: implement an array depth variable (i.e. x[y][z] has an array depth of 2. x on it's own has an array depth of 0)
		char *name = get_name_from_exp(stmt->var.name);
		
		declare_var(env, name, is_defined, is_mutable, array_depth);
		check_exp_semantics(env, stmt->var.value);
		check_exp_semantics(env, stmt->var.name);
		break;
	case STMT_EXPR:
		check_exp_semantics(env, stmt->exp);
		break;
	case STMT_IF:
		check_exp_semantics(env, stmt->ifStmt.cond);
		check_stmt_semantics(env, stmt->ifStmt.thenStmt);
		check_stmt_semantics(env, stmt->ifStmt.elseStmt);
		break;
	case STMT_LOOP:
		check_exp_semantics(env, stmt->loop.cond);
		check_stmt_semantics(env, stmt->loop.body);
		break;
	default:
		raise_semantic_error("invalid statement in semantics", env);
		break;
	}
	check_stmt_semantics(env, stmt->next);
}

 /*
  * Checks the semantics of the given exp.
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
  *   3.1: for ++, --, and - unary operators, they must refer to names or other unary values.
  * 4. binary op rules:
  *   4.1: all assignments must be ints.
  * 6: print and input take and return int values only.
  * 7: commas are only used when defining arrays.
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
		if (!exp_is_unary(exp->op.left))
			raise_semantic_error("expected a single variable being set.", env);
		char *name = get_name_from_exp(exp->op.left);
		//TODO: set to its own checker function
		if (!((exp->op.right->type == EXP_RIGHTARRAY) ^ ((is_array_var(env, name) && (exp->op.left->type != EXP_ARRAY))))) // if it's an array, set it to an initlist.
			raise_semantic_error("array must be set to an array (i.e. {asdf})", env);
		check_exp_semantics(env, exp->op.left);
		check_exp_semantics(env, exp->op.right);
		break;
	case EXP_UNARY:
	case EXP_BINARY_OP:
		check_exp_semantics(env, exp->op.left);
		check_exp_semantics(env, exp->op.right);
		break;
	case EXP_ARRAY:
		check_exp_semantics(env, exp->array.name);
		check_exp_semantics(env, exp->array.index);
		break;
	case EXP_RIGHTARRAY:
		for (int i = 0; i < exp->right_array.size; i++)
			check_exp_semantics(env, exp->right_array.array + i);
		break;
	default:
		raise_semantic_error("invalid exp in semantics", env);
		break;
	}
}