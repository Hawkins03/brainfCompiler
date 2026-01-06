#ifndef MS_H
#define MS_H

#include <stdbool.h>
#include "utils.h"

struct MS_Exp;

typedef enum {EXP_EMPTY, EXP_STR, EXP_NUM, EXP_OP, EXP_UNARY, EXP_CALL, EXP_ARRAY, EXP_INITLIST, EXP_INDEX} ExpType;

typedef struct Exp_t {
    ExpType type;
    union {//e.as (e.as.name for example).
	char *str;
	int num;
    struct Exp_t *initlist;
    struct { struct Exp_t *index, *next; } index;
    struct { struct Exp_t *arr_name, *index;} arr;
	struct { KeyType key; struct Exp_t *call; } call;
	struct { struct Exp_t *left, *right; char *op; } op;
    };
} Exp;

typedef enum {STMT_EMPTY, STMT_VAR, STMT_VAL, STMT_LOOP, STMT_IF, STMT_EXPR, STMT_CALL} StmtType;

typedef struct Stmt_t {
    StmtType type;
    union {
	struct { Exp *cond; struct Stmt_t *body; } loop;
	struct { Exp *cond; struct Stmt_t *thenStmt; struct Stmt_t *elseStmt;} ifStmt;
	Exp *exp;
    };
    struct Stmt_t *next;
} Stmt;

void free_exp(Exp *exp);
void free_stmt(Stmt *stmt);

void print_full_exp(const Exp *atom);
void print_exp(const Exp *exp);
void print_stmt(const Stmt *stmt);

Exp *init_exp();
Exp *init_op(Exp *left, char *op, Exp *right);
Exp *init_unary(Exp *left, char *op, Exp *right);
Exp *init_num(int num);
Exp *init_str(char *str);
Exp *init_call(KeyType key, Exp *call);

Stmt *init_stmt();
Stmt *init_loop(Exp *cond, Stmt *body, Stmt *next);
Stmt *init_ifStmt(Exp *cond, Stmt *thenStmt, Stmt *elseStmt, Stmt *next);
Stmt *init_expStmt(Exp *exp, Stmt *next);

Exp *parse_call(Reader *r);

Exp *parse_unary(Reader *r);
Exp *parse_char(Reader *r);
Exp *parse_parenthesis(Reader *r);
Exp *parse_atom(Reader *r);
Exp *parse_exp(int minPrio, Reader *r);
Stmt *parse_keyword(Reader *r);
Stmt *parse_stmt(Reader *r);
Stmt *parse_file(const char *filename);


#endif //MS_H