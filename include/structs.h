
#ifndef STRUCT_H
#define STRUCT_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    KW_VAR = 0, KW_VAL, KW_WHILE, KW_FOR, 
    KW_IF, KW_ELSE, KW_PRINT, KW_INPUT, KW_BREAK, 
    KW_TRUE, KW_FALSE
} key_t;

typedef enum {
    VAL_EMPTY, VAL_NAME, VAL_STR, VAL_OP, VAL_NUM, VAL_DELIM, VAL_KEYWORD
} value_type_t;

typedef enum {
    EXP_EMPTY, EXP_STR, EXP_NUM, EXP_OP, EXP_UNARY, EXP_CALL, EXP_ARRAY, EXP_NESTED
} exp_type_t;

typedef enum {
    STMT_EMPTY, STMT_VAR, STMT_VAL, STMT_LOOP, STMT_IF, STMT_EXPR
} stmt_type_t;

typedef struct value {
    value_type_t type;
    union {
        char *str;
        int num;
        key_t key;
        char ch;
    };
    void (*print_fn)(const struct value *value);
} value_t;


typedef struct exp {
    exp_type_t type;
    union {//e.as (e.as.name for example).
        char *str;
        int num;
        struct exp *nested; // in this case it's for arrays.
        struct { struct exp *name, *index;} arr;
        struct { key_t key; struct exp *call; } call;
        struct { struct exp *left, *right; char *op; } op;
    };
    void (*print_fn)(const struct exp *exp);
    void (*free_fn)(struct exp *exp);
} exp_t;

typedef struct stmt {
    stmt_type_t type;
    union {
        struct { exp_t *name; exp_t *value; bool is_mutable; } var;
        struct { exp_t *cond; struct stmt *body; } loop;
        struct { exp_t *cond; struct stmt *thenStmt; struct stmt *elseStmt;} ifStmt;
        exp_t* exp;
    };
    void (*print_fn)(const struct stmt *stmt);
    void (*free_fn)(struct stmt *stmt);
    struct stmt *next;
} stmt_t;


typedef struct reader {
    FILE *fp;

    // storage for tokens
    int curr_char;
    value_t *curr_token;

    // for freeing structs
    stmt_t *root;
    stmt_t *curr_stmt;
} Reader;


#endif //STRUCT_H