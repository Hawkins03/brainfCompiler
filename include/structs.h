
#ifndef STRUCT_H
#define STRUCT_H

#include <stdio.h>
#include <stdbool.h>

enum key_type {
    KW_VAR = 0, KW_VAL, KW_WHILE, KW_FOR, 
    KW_IF, KW_ELSE, KW_PRINT, KW_INPUT, KW_BREAK, 
    KW_TRUE, KW_FALSE
};

enum value_type{
    VAL_EMPTY, VAL_NAME, VAL_STR, VAL_OP, VAL_NUM, VAL_DELIM, VAL_KEYWORD
};

enum exp_type {
    EXP_EMPTY, EXP_STR, EXP_NUM, EXP_OP, EXP_UNARY, EXP_CALL, EXP_ARRAY, EXP_NESTED
};

enum stmt_type {
    STMT_EMPTY, STMT_VAR, STMT_VAL, STMT_LOOP, STMT_IF, STMT_EXPR
};

struct value {
    enum value_type type;
    union {
        char *str;
        int num;
        enum key_type key;
        char ch;
    };
    void (*print_fn)(const struct value *value);
};


struct exp {
    enum exp_type type;
    union {//e.as (e.as.name for example).
        char *str;
        int num;
        struct exp *nested; // in this case it's for arrays.
        struct { struct exp *name, *index;} arr;
        struct { enum key_type key; struct exp *call; } call;
        struct { struct exp *left, *right; char *op; } op;
    };
    void (*print_fn)(const struct exp *exp);
    void (*free_fn)(struct exp *exp);
};

struct stmt {
    enum stmt_type type;
    union {
        struct { struct exp *name; struct exp *value; bool is_mutable; } var;
        struct { struct exp  *cond; struct stmt *body; } loop;
        struct { struct exp  *cond; struct stmt *thenStmt; struct stmt *elseStmt;} ifStmt;
        struct exp * exp;
    };
    void (*print_fn)(const struct stmt *stmt);
    void (*free_fn)(struct stmt *stmt);
    struct stmt *next;
};


struct reader {
    FILE *fp;
    char *filename;

    // storage for tokens
    int curr_char;
    struct value *curr_token;

    // for freeing structs
    struct stmt *root;
    struct stmt *curr_stmt;
};

struct env;

struct var_data {
    char *name;
    struct exp  *value;
    bool is_mutable;
    bool is_array;
};

struct env {
    struct var_data *vars;
    size_t count;
    size_t capacity;
    struct env *parent;
};


#endif //STRUCT_H