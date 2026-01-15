#ifndef STMT_H
#define STMT_H

#include "exp.h"
#include "utils.h"
#include "structs.h"
#include <stdbool.h>

struct stmt *initStmtOrKill(struct reader *r);


void free_stmt(struct stmt *stmt);
void print_stmt(const struct stmt *stmt);

bool stmts_match(const struct stmt *stmt1, const struct stmt *stmt2);
bool isValidInitStmt(const struct stmt *stmt);

#endif //STMT_H