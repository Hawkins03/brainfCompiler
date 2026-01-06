#include "semantics.h"
#include <string.h>
#include <stdlib.h>



env_t *init_env(env_t *parent) {
  env_t *env = calloc(1, sizeof(*env));
  if (!env)
    raise_error("failed to allocate space on the heap");
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
  if (var_exists(env, name)) {
    //TODO: do an actual buffer into the raise error with the name
    raise_error("variable name already defined");
  }
  var_data_t *var_loc = env->vars + (env->count)++;
  var_loc->name = name;
  var_loc->value = value;
  var_loc->is_mutable = is_mutable;
  var_loc->is_array = is_array;
}

void set_var(env_t *env, char *name, exp_t *value) {
  var_data_t *var = get_var(env, name);
  if (!var->is_mutable && var->value != NULL)
    raise_error("user is attempting to reassign an immutable variable");
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



 /*
  * Checks the semantics of the given statement list.
  * 1. no reassigning a value,
  * 2. no redefining a variable (i.e. no "var x;" if x already exists),
  * 3. no referencing undefined variables.
  * 4. a variable is assigned in all paths (if and loops included) before it's used.
  * 5. no operations on a list (operations on list values are fine, but not directly on a list)
  * 5. no obvious division by zero.
  * 6. Expressions will be expanded, such that they can't be internally nested.
  * 7. if a variable is assigned in only one branch of an if statement, it is considered unassigned after the if statement.
  * 8. Integers don't go out of bounds (32 bit signed).
  */
env_t *check_semantics(env_t *env, stmt_t *stmt) {
  
}