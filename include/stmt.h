#ifndef STMT_H
#define STMT_H

#include "exp.h"
#include "utils.h"
#include "structs.h"
#include <stdbool.h>

struct stmt *init_stmt_or_free(struct reader *r, struct exp **exps, struct stmt **stmts);
struct stmt *init_stmt();
void init_var(struct exp  *name, struct exp  *value, struct stmt *in);
void init_loop(struct exp  *cond, struct stmt *body, struct stmt *in);
void init_ifStmt(struct exp  *cond, struct stmt *thenStmt, struct stmt *in);
void init_expStmt(struct exp  *exp, struct stmt *in);

void free_stmt(struct stmt *stmt);
void print_stmt(const struct stmt *stmt);

bool stmts_match(const struct stmt *stmt1, const struct stmt *stmt2);
bool isValidInitStmt(const struct stmt *stmt);

#endif //STMT_H