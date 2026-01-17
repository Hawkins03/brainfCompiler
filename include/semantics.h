#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdbool.h>
#include "structs.h"
#include "utils.h"
#include "parser.h"
#include "stmt.h"
#include "exp.h"

#define STARTING_ENV_CAP 8

//env utility functions
void setup_env(struct env *env, struct env *parent);
void free_env(struct env *env);

bool declare_var(struct env *env, char *name, bool is_mutable, int array_depth);
struct var_data *get_var(const struct env *env, const char *name);
bool var_exists(const struct env *env, const char *name);

//checking semantics
void check_stmt_semantics(struct env *env, struct stmt *stmt);
void check_exp_semantics(struct env *env, struct exp *exp);

void check_file_semantics(char *filename);

#endif //SEMANTICS_H