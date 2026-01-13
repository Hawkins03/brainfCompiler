#ifndef STMT_H
#define STMT_H

#include "exp.h"
#include "utils.h"
#include <stdbool.h>

struct stmt *init_stmt();
struct stmt *init_var(struct exp  *name, struct exp  *value, struct reader *r);
struct stmt *init_loop(struct exp  *cond, struct stmt *body, struct reader *r);
struct stmt *init_ifStmt(struct exp  *cond, struct stmt *thenStmt, struct reader *r);
struct stmt *init_expStmt(struct exp  *exp, struct reader *r);

void free_stmt(struct stmt *stmt);
void print_stmt(const struct stmt *stmt);

bool stmts_match(const struct stmt *stmt1, const struct stmt *stmt2);

#endif //STMT_H