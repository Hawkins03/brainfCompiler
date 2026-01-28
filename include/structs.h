
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

enum err_type {
	ERR_OK = 0,
	
	// File/IO errors
	ERR_NO_FILE,        // Failed to open file
	ERR_EOF,            // Unexpected end of file
	ERR_NO_ARGS, 	    // expected an argument
	
	// Memory errors
	ERR_NO_MEM,         // Memory allocation failed
	ERR_REFREE,         // Attempted to free already freed memory
	
	// Lexer/tokenization errors
	ERR_UNEXP_CHAR,     // Unexpected character
	ERR_INV_ESC,        // Invalid escape sequence
	ERR_UNMATCHED_BRACKET,  // More specific than ERR_MISS_DELIM
	ERR_UNMATCHED_PAREN,
	ERR_UNMATCHED_BRACE,
	ERR_UNMATCHED_QUOTE, 
	ERR_BIG_NUM,        // Number exceeds maximum value
	ERR_TOO_LONG,       // String exceeds maximum length
	
	// Parser errors
	ERR_INV_TYPE,       // Invalid or unexpected type
	ERR_INV_VAL,        // Invalid value
	ERR_INV_OP,         // Invalid operator
	ERR_INV_EXP,        // Invalid expression
	ERR_INV_STMT,       // Invalid statement
	ERR_BAD_ELSE,       // Malformed else clause
	
	// Semantic errors
	ERR_REDEF,          // Variable redefinition
	ERR_NO_VAR,         // Undefined variable
	ERR_IMMUT,          // Assignment to immutable variable
	ERR_INV_ARR,        // Invalid array operation
	
	// Internal/logic errors
	ERR_INF_REC,        // Infinite recursion detected
	ERR_INTERNAL,       // Internal compiler error
};

enum operator {
	OP_PLUS, OP_MINUS,
	OP_MULTIPLY, OP_DIVIDE, OP_MODULO, //5
	OP_BITWISE_NOT, OP_BITWISE_OR, OP_BITWISE_XOR, OP_BITWISE_AND,//9
	OP_LEFT_SHIFT, OP_RIGHT_SHIFT,//11
	OP_LT, OP_LE, OP_GT, OP_GE,//15
	OP_EQ, OP_NE,//17
	OP_LOGICAL_NOT, OP_LOGICAL_AND,  OP_LOGICAL_OR,//20
	OP_ASSIGN, OP_PLUS_ASSIGN, OP_MINUS_ASSIGN,//23
	OP_MULTIPLY_ASSIGN, OP_DIVIDE_ASSIGN, OP_MODULO_ASSIGN,//26
	OP_LEFT_SHIFT_ASSIGN, OP_RIGHT_SHIFT_ASSIGN,//28
	OP_BITWISE_AND_ASSIGN, OP_BITWISE_XOR_ASSIGN, OP_BITWISE_OR_ASSIGN,//31
	OP_INCREMENT, OP_DECREMENT,//33
	OP_UNKNOWN//34
};

//TODO: only malloc one value, and pass stuff through it. (i.e. peek only, getValue just changes its contents)
struct value {
	enum value_type type;
	int start_pos;
	union {
		char *str;
		int num;
		char ch;
	};
};

//TODO: change op into a enum.
// exp_binary, exp_unary, exp_array_ref, exp_array_lit, exp_call
struct exp_binary { struct exp *left; struct exp *right; enum operator op; };
struct exp_unary { struct exp *operand; enum operator op; bool is_prefix; };
struct exp_array_ref { struct exp *name, *index; };
struct exp_array_lit { struct exp *array; int size; };
struct exp_call { enum key_type key; struct exp *arg; };
struct exp {
	enum exp_type type;
	int line_num, start_col;
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
	int line_num, start_col;
	union {
		struct stmt_var *var;
		struct stmt_loop *loop;
		struct stmt_if *ifStmt;
		struct exp * exp;
	};
	struct stmt *next;
};


struct reader {
	struct value val;
	fpos_t line_start_pos;

	FILE *fp;
	char *filename;
	struct stmt *root;
	char *line_buf;

	int ch;
	unsigned int line_cap, line_pos, line_num;
};

struct env;
// TODO: change name to scope
struct var_data {
	char *name;
	bool is_mutable;
	int array_depth;
	//don't need is_defined because variables are default assigned to 0
};

struct env {
	struct stmt *root;
	struct env *parent;
	char *filename;
	struct var_data *vars;
	size_t len;
	size_t cap;
};

enum ir_value_type {
    IR_VAL_NUM,
    IR_VAL_VAR
};

enum ir_type {
	IR_LET,
	IR_EXP,
	IR_LOOP,
};

enum ir_op {
	IR_ADD,
	IR_SUB,
	IR_COPY,
};

struct ir_val {
    enum ir_value_type type;
    union {
        int num;
        char *var;
    };
};

struct ir_assign {
	enum ir_op op;
	char *dest;
	struct ir_val lhs;
	struct ir_val rhs;
};

struct ir_loop {
	char *cond;
	struct ir_node **body;
	int length;
};

struct ir_node {
	enum ir_type type;
	union {
		struct ir_assign assign;
		struct ir_loop loop;
	};
	struct ir_node *next;
};


#endif //STRUCT_H