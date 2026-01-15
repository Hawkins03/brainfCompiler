
#ifndef STRUCT_H
#define STRUCT_H

#include <stdio.h>
#include <stdbool.h>

enum key_type {
	KW_VAR, KW_VAL, KW_WHILE, KW_FOR, 
	KW_IF, KW_ELSE, KW_PRINT, KW_INPUT, KW_BREAK, 
	KW_TRUE, KW_FALSE
};

enum value_type {
	VAL_EMPTY, VAL_NAME, VAL_STR, VAL_OP, VAL_NUM, VAL_DELIM, VAL_KEYWORD
};

enum name_type {
	NAME_NAME, NAME_ARRAY
};

enum exp_type {
	EXP_EMPTY, EXP_NAME, EXP_ARRAY_REF, EXP_ARRAY_LIT, EXP_NUM, EXP_ASSIGN_OP, EXP_BINARY_OP, EXP_UNARY, EXP_CALL
};

enum stmt_type {
	STMT_EMPTY, STMT_VAR, STMT_LOOP, STMT_IF, STMT_EXPR
};

//TODO: only malloc one value, and pass stuff through it. (i.e. peek only, getValue just changes its contents)
struct value {
	enum value_type type;
	union {
		char *str;
		int num;
		enum key_type key;
		char ch;
	};
};

//TODO: change op into a enum.
// exp_binary, exp_unary, exp_array_ref, exp_array_lit, exp_call
struct exp_binary { struct exp *left; struct exp *right; char *op; };
struct exp_unary { struct exp *operand; char *op; bool is_prefix; };
struct exp_array_ref { struct exp *name, *index; };
struct exp_array_lit { struct exp *array; int size; };
struct exp_call { enum key_type key; struct exp *arg; };
struct exp {
	enum exp_type type;
	union {
		int num;
		char *name;
		struct exp_binary *op;
		struct exp_unary *unary;
		struct exp_array_ref *array_ref;
		struct exp_array_lit *array_lit;
		struct exp_call *call;
	};
};

struct stmt_var { struct exp *name; struct exp *value; bool is_mutable; };
struct stmt_if { struct exp *cond; struct stmt *thenStmt; struct stmt *elseStmt; };
struct stmt_loop { struct exp *cond; struct stmt *body; };

struct stmt {
	enum stmt_type type;
	union {
		struct stmt_var *var;
		struct stmt_loop *loop;
		struct stmt_if *ifStmt;
		struct exp * exp;
	};
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

struct var_data { //TODO: make bitarray for the bools
	char *name;
	bool is_mutable;
	int array_depth;
	bool is_defined;
};

struct env {
	struct var_data *vars;
	struct stmt *root;
	size_t count;
	size_t capacity;
	struct env *parent;
};


#endif //STRUCT_H