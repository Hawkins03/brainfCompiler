#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdbool.h>
#include "utils.h"
#include "parser.h"
#include "stmt.h"
#include "exp.h"

#define STARTING_ENV_CAP 8


struct env *init_env(struct env *parent);
void free_env(struct env *env);
void define_var(struct env *env, char *name, struct exp  *value, bool is_mutable, bool is_array);
void set_var(struct env *env, char *name, struct exp  *value);
struct var_data *get_var(const struct env *env, const char *name);
bool var_exists(const struct env *env, const char *name);

void check_stmt_semantics(struct env *env, struct stmt *stmt);
void check_exp_semantics(struct env *env, struct exp *exp);

#endif //SEMANTICS_H