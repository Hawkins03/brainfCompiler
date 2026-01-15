#ifndef STMT_H
#define STMT_H

#include "exp.h"
#include "utils.h"
#include "structs.h"
#include <stdbool.h>

struct stmt *init_stmt(struct reader *r);
void init_varStmt(struct reader *r, struct stmt *stmt, bool is_maliable);
void init_ifStmt(struct reader *r, struct stmt *stmt);
void init_loopStmt(struct reader *r, struct stmt *stmt);
void init_expStmt(struct stmt *stmt, struct exp *exp);

void free_stmt(struct stmt *stmt);
void print_stmt(const struct stmt *stmt);

bool stmts_match(const struct stmt *stmt1, const struct stmt *stmt2);
bool isValidInitStmt(const struct stmt *stmt);

#endif //STMT_H