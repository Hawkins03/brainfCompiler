
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
	EXP_EMPTY, EXP_NAME, EXP_ARRAY, EXP_NUM, EXP_ASSIGN_OP, EXP_BINARY_OP, EXP_UNARY, EXP_CALL, EXP_RIGHTARRAY
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
};

struct exp {
	enum exp_type type;
	union {
		int num;
		char *name;
		struct { struct exp *name, *index;} array;
		struct { struct exp *array; int size; } right_array;
		struct { enum key_type key; struct exp *call; } call;
		struct { struct exp *left, *right; char *op; } op;
	};
};

struct stmt {
	enum stmt_type type;
	union {
		struct { struct exp *name; struct exp *value; bool is_mutable; } var;
		struct { struct exp *cond; struct stmt *body; } loop;
		struct { struct exp *cond; struct stmt *thenStmt; struct stmt *elseStmt;} ifStmt;
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