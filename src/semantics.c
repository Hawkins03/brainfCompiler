#include "semantics.h"
#include "parser.h"
#include "structs.h"
#include "value.h"
#include "exp.h"
#include "stmt.h"
#include <string.h>
#include <stdlib.h>



env_t *init_env(env_t *parent) {
  env_t *env = calloc(1, sizeof(*env));
  if (!env)
    return NULL;
  env->parent = parent;
  return env;
}

void free_env(env_t *env) {
  if (!env)
    return;
  free_env(env->parent);
  free(env);
}

void define_var(env_t *env, char *name, exp_t *value, bool is_mutable, bool is_array) {
  if (var_exists(env, name))
    raise_semantic_error("variable name already defined", env);

  var_data_t *var_loc = env->vars + (env->count)++;
  var_loc->name = name;
  var_loc->value = value;
  var_loc->is_mutable = is_mutable;
  var_loc->is_array = is_array;
}

void set_var(env_t *env, char *name, exp_t *value) {
  var_data_t *var = get_var(env, name);
  if (!var->is_mutable && var->value != NULL)
    raise_semantic_error("user is attempting to reassign an immutable variable", env);
  var->value = value;
}
var_data_t *get_var(const env_t *env, const char *name) {
  if (!env)
    return NULL;
  for (size_t i = 0; i < env->count; i++) {
    var_data_t *var = env->vars + i;
    if ((var) && (!strcmp(var->name, name)))
      return var;
  }
  return get_var(env->parent, name);
}

bool var_exists(const env_t *env, const char *name) {
  return (get_var(env, name) != NULL);
}

bool is_array_var(const env_t *env, const char *name) {
  var_data_t *var = get_var(env, name);
  return ((var != NULL) && var->is_array);
}


//TODO: create a check_semantics that returns void, and remove a lot of funcs from the .h file.
 /*
  * Checks the semantics of the given statement list.
  * 1. no reassigning a non mutable variable, (i.e. val x can't be reassigned after you define it.
  * 2. no redefining a variable (i.e. no "var x;" if x already exists),
  * 3. no referencing variables if they haven't been defined yet (i.e. no x = y; var y = 0;)
  * 5. no combining lists and integers (i.e. x[i] = x is invalid)
  */
env_t *check_semantics(env_t *env, stmt_t *stmt, exp_t *exp) {
  if (stmt) {
    switch (stmt->type) {
      case STMT_VAR:
      case STMT_VAL:
        //TODO: if val = NULL, define it as uninitiated
        bool is_mutable = (stmt->type == STMT_VAR);
        bool is_array_tp = exp_is_array(stmt->var.name);
        if (!(is_array_tp ^ (stmt->var.value->type != EXP_NESTED))) // ^ = xor, so basically means no array and no 
          raise_semantic_error("you can't set an array to an integer", env);
        char *name = get_name_from_exp(stmt->var.name);
        
        define_var(env, name, stmt->var.value, is_mutable, is_array_tp);
        check_semantics(env, NULL, stmt->var.value);
        check_semantics(env, NULL, stmt->var.name);
        break;
      case STMT_EXPR:
        check_semantics(env, NULL, stmt->exp);
        break;
      case STMT_IF:
        check_semantics(env, NULL, stmt->ifStmt.cond);
        check_semantics(env, stmt->ifStmt.thenStmt, NULL);
        check_semantics(env, stmt->ifStmt.elseStmt, NULL);
        break;
      case STMT_LOOP:
        check_semantics(env, NULL, stmt->loop.cond);
        check_semantics(env, stmt->loop.body, NULL);
        break;
      default:
        raise_semantic_error("invalid statement in semantics", env);
        break;
    }
    check_semantics(env, stmt->next, NULL);
  } else if (exp) {
    //EXP_EMPTY, EXP_STR, EXP_NUM, EXP_OP, EXP_UNARY, EXP_CALL, EXP_ARRAY, EXP_INITLIST, EXP_INDEX
    switch (exp->type) {
      case EXP_STR:
        var_data_t *var = get_var(env, exp->str);
        if (!var)
          raise_semantic_error("variable has not been defined", env);
        if (var->is_array)
          raise_semantic_error("array is being used as an integer", env);
        break;
      case EXP_UNARY:
      case EXP_OP:
        if (get_prio(exp->op.op) == SET_OP_PRIO) {
          if (!exp_is_unary(exp->op.left))
            raise_semantic_error("expected a single variable being set.", env);
          char *name = get_name_from_exp(exp->op.left);
          if (!((exp->op.right->type == EXP_NESTED) ^ ((is_array_var(env, name) && (exp->op.left->type != EXP_ARRAY))))) // if it's an array, set it to an initlist.
            raise_semantic_error("array must be set to an array (i.e. {asdf})", env);
        }

        check_semantics(env, stmt, exp->op.left);
        check_semantics(env, stmt, exp->op.left);
        break;
      case EXP_ARRAY:
        check_semantics(env, stmt, exp->arr.name);
        break;
      case EXP_NESTED:
        check_semantics(env, stmt, exp->nested);
        break;
      default:
        raise_semantic_error("invalid exp in semantics", env);
        break;
    }
  }
  return NULL;
}