#include <stdbool.h>
#include "utils.h"
#ifndef MS_H
#define MS_H

struct MS_Exp;


typedef enum {EXP_EMPTY, EXP_STR, EXP_NUM, EXP_OP, EXP_BINOP} ExpType;

typedef struct Exp_t {
    ExpType type;
    union {//e.as (e.as.name for example).
	char *str;
	int num;
	struct { struct Exp_t *left, *right; char *binop; } binop;
	struct { struct Exp_t *left, *right; char op; } op;
    };
} Exp;

typedef enum {STMT_VAR, STMT_LET, STMT_SET, STMT_LOOP, STMT_IF, STMT_EXPR} StmtType;

typedef struct Stmt_t {
    StmtType type;
    union {
	struct { char *name; Exp *init; } var;
	struct { Exp *cond; struct Stmt_t *body; } loop;
	struct { Exp *cond; struct Stmt_t *thenStmt; struct Stmt_t *elseStmt;} ifStmt;
	struct { Exp *exp; } expStmt;
    };
    struct Stmt_t *next;
} Stmt;

void free_exp(Exp *exp);
void print_full_exp(Exp *atom);
void print_exp(Exp *exp);
Exp *init_exp();
Exp *parse_exp(int minPrio, Reader *r);
//Stmt *parse_stmt(Stmt *prev, Reader *r);
Exp *parse_file(const char *filename);


#endif //MS_H
