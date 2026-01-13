#ifndef STMT_H
#define STMT_H

#include "exp.h"
#include "utils.h"
#include <stdbool.h>

stmt_t *init_stmt();
stmt_t *init_var(exp_t *name, exp_t *value, Reader *r);
stmt_t *init_loop(exp_t *cond, stmt_t *body, Reader *r);
stmt_t *init_ifStmt(exp_t *cond, stmt_t *thenStmt, Reader *r);
stmt_t *init_expStmt(exp_t *exp, Reader *r);

void free_stmt(stmt_t *stmt);
void print_stmt(const stmt_t *stmt);

bool compare_stmts(const stmt_t *stmt1, const stmt_t *stmt2);

#endif //STMT_H