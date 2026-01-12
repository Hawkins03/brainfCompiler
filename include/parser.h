#ifndef MS_H
#define MS_H

#include <stdbool.h>
#include "utils.h"

struct MS_Exp;

typedef enum {EXP_EMPTY, EXP_STR, EXP_NUM, EXP_OP, EXP_UNARY, EXP_CALL, EXP_ARRAY, EXP_INITLIST} exp_type_t;

typedef struct exp {
    exp_type_t type;
    union {//e.as (e.as.name for example).
	char *str;
	int num;
    struct exp *initlist;
    struct { struct exp *name, *index;} arr;
	struct { key_t key; struct exp *call; } call;
	struct { struct exp *left, *right; char *op; } op;
    };
} exp_t;

typedef enum {STMT_EMPTY, STMT_VAR, STMT_VAL, STMT_LOOP, STMT_IF, STMT_EXPR} stmt_type_t;

typedef struct stmt {
    stmt_type_t type;
    union {
    struct { exp_t *name; exp_t *value; } var;
	struct { exp_t *cond; struct stmt *body; } loop;
	struct { exp_t *cond; struct stmt *thenStmt; struct stmt *elseStmt;} ifStmt;
	exp_t* exp;
    };
    struct stmt *next;
} stmt_t;

void free_exp(exp_t *exp);
void free_stmt(stmt_t *root, stmt_t *stmt);

void print_exp(const exp_t *exp);
void print_stmt(const stmt_t *stmt);

char *get_name_from_exp(exp_t *exp);
bool is_array(exp_t *exp);
bool is_unary_exp(exp_t *exp);

exp_t *init_exp();
exp_t *init_op(exp_t *left, char *op, exp_t *right, Reader *r);
exp_t *init_unary(exp_t *left, char *op, exp_t *right, Reader *r);
exp_t *init_num(int num, Reader *r);
exp_t *init_str(char *str, Reader *r);
exp_t *init_call(key_t key, exp_t *call, Reader *r);
exp_t *init_array(exp_t *name, exp_t *index, Reader *r);
exp_t *init_initList(exp_t *op, Reader *r);

stmt_t *init_stmt();
stmt_t *init_var(exp_t *name, exp_t *value, Reader *r);
stmt_t *init_loop(exp_t *cond, stmt_t *body, Reader *r);
stmt_t *init_ifStmt(exp_t *cond, stmt_t *thenStmt, Reader *r);
stmt_t *init_expStmt(exp_t *exp, Reader *r);

exp_t *parse_atom(Reader *r);
exp_t *parse_exp(int minPrio, Reader *r);
void parse_stmt(Reader *r);
stmt_t *parse_file(const char *filename);


#endif //MS_H