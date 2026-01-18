/** @file test.h
 *  @brief the file providing utility
 * 
 * basically just string mallocing and error printing.
 * might rename to errors and add a strings file, but for now it's just this.
 * might also move reader into it's own file again and remove out values, but I like the current setup
 * 
 * @author Hawkins Peterson (Hawkins03)
 * @bug no bugs
 */


#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdbool.h>
#include "structs.h"

#define ALLOC_LIST_START_LEN 8

#define raise_error(err) \
	_raise_error(err, __func__, __FILE__, __LINE__)

#define raise_syntax_error(err, r) \
	_raise_syntax_error(err, __func__, __FILE__, __LINE__, r)

#define raise_exp_semantic_error(err, exp, env) \
	_raise_exp_semantic_error(err, exp, __func__, __FILE__, __LINE__, env)

#define raise_stmt_semantic_error(err, stmt, env) \
	_raise_stmt_semantic_error(err, stmt, __func__, __FILE__, __LINE__, env)

//reader struct:
void set_strlen(char **str, const int len, struct reader *r);
void reset_strlen_if_needed(char **str, const int len, int *cap, struct reader *r);

void _raise_error(enum err_type err, const char *func, const char *file, int line);
void _raise_syntax_error(enum err_type err, const char *func, const char *file, int line, struct reader *r);
void _raise_exp_semantic_error(enum err_type err, const struct exp *exp, const char *func, const char *file, int line, struct env *env);
void _raise_stmt_semantic_error(enum err_type err, const struct stmt *stmt, const char *func, const char *file, int line, struct env *env);

#endif //UTILS_H
