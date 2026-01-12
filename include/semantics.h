#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdbool.h>
#include "utils.h"
#include "parser.h"

#define STARTING_ENV_CAP 8

typedef struct env env_t;

typedef struct var_data {
    char *name;
    exp_t *value;
    bool is_mutable;
    bool is_array;
} var_data_t;

struct env {
    var_data_t *vars;
    size_t count;
    size_t capacity;
    struct env *parent;
};

env_t *init_env(env_t *parent);
void free_env(env_t *env);
void define_var(env_t *env, char *name, exp_t *value, bool is_mutable, bool is_array);
void set_var(env_t *env, char *name, exp_t *value);
var_data_t *get_var(const env_t *env, const char *name);
bool var_exists(const env_t *env, const char *name);

env_t *check_semantics(env_t *env, stmt_t *stmt, exp_t *exp);

#endif //SEMANTICS_H