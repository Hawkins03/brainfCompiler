#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdbool.h>
#include "structs.h"

typedef struct env env_t;
typedef struct stmt stmt_t;

#define ALLOC_LIST_START_LEN 8

#define raise_error(msg) \
    _raise_error((msg), __func__, __FILE__, __LINE__)

#define raise_syntax_error(msg, r) \
    _raise_syntax_error((msg), __func__, __FILE__, __LINE__, r)

#define raise_semantic_error(msg, env) \
    _raise_semantic_error((msg), __func__, __FILE__, __LINE__, env)


//reader struct:

char *strdup(const char *s, Reader *r);

void _raise_error(const char *msg, const char *func, const char *file, int line);
void _raise_syntax_error(const char *msg, const char *func, const char *file, int line, Reader *r);
void _raise_semantic_error(const char *msg, const char *func, const char *file, int line, env_t *env);
#endif //UTILS_H
