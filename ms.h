#include <stdbool.h>
#include "utils.h"
#ifndef MS_H
#define MS_H

struct MS_Exp;


typedef enum {EXP_EMPTY, EXP_NAME, EXP_ATOM, EXP_OP, EXP_BINOP} ExpType;

typedef struct Exp_t {
    ExpType type;
    union {//e.as (e.as.name for example).
	struct { char *value; } name;
	struct { int  value; } atom;
	struct { struct Exp_t *left, *right; char *op; } op; 
    };
} Exp;

typedef enum {STMT_VARDEC, STMT_LOOP, STMT_IF, STMT_EXPR} StmtType;

typedef struct Stmt_t {
    StmtType type;
    union {
	struct { char *name; Exp *init; } vardec;
	struct { Exp *cond; struct Stmt_t *body; } loop;
	struct { Exp *cond; struct Stmt_t *thenStmt; struct Stmt_t *elseStmt;} ifStmt;
	struct { Exp *exp; } expStmt;
    };
    struct Stmt_t *next;
} Stmt;

extern const char *MS_SYMBOLS;
extern const char *MS_OPSYM;
extern const char *MS_OPS[];
extern const char *MS_BINOP[];
extern const char *MS_KEYWORDS[];
extern const int NUM_OPS;
extern const int NUM_BINOPS;
extern const int NUM_KEYWORDS;

void free_exp(Exp *exp);
void print_full_exp(Exp *atom);
void print_exp(Exp *exp);
bool isOp(char *op);
bool isBinOp(char *op);
Exp *init_exp();
Exp *parse_exp(int minPrio, Reader *r);
//Stmt *parse_stmt(Stmt *prev, Reader *r);
Exp *parse_file(char *filename);


#endif //MS_H
