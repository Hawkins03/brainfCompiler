/** @file parser.c
 *  @brief Functions for parsing expressions
 *
 *  This contains the functions that
 *  parse a series of values from a
 *  reader struct into a linked-list
 *  of statements.
 *
 *  @author Hawkins Peterson (hawkins03)
 *  @bug No known bugs.
 */


#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "structs.h"
#include "exp.h"
#include "stmt.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"

static inline bool hasNextStmt(struct lexer_ctx *lex) {

	return lex && ((lex->val.type != VAL_DELIM) || (lex->val.ch != '}'));
}

static inline bool hasCommaNext(struct lexer_ctx *lex) {
	return lex && (lex->val.type == VAL_DELIM) && (lex->val.ch == ',');
}
static inline bool atSemicolon(struct lexer_ctx *lex) {
	return lex && (lex->val.type == VAL_DELIM) && (lex->val.ch == ';');
}

static inline bool parserCanProceed(struct lexer_ctx *lex) {
	return lex && hasNextStmt(lex) && !atSemicolon(lex);
}

static inline bool is_assignable(const struct exp *exp) {
	return (exp) && ((exp->type == EXP_NAME) || (exp->type == EXP_ARRAY_REF));
}

static inline bool is_atomic(const struct exp *exp) {
	if (!exp)
		return false;
	if (is_assignable(exp))
		return true;
	
	return false;
}

static inline bool parses_to_int(struct exp *exp) {
	if (!exp)
		return false;
	switch (exp->type) {
	case EXP_NAME:
	case EXP_ARRAY_REF:
		return true;
	case EXP_BINARY_OP:
	case EXP_ASSIGN_OP:
		return (parses_to_int(exp->op->left) && parses_to_int(exp->op->right));
	case EXP_UNARY:
		return parses_to_int(exp->unary->operand);
	default:
		return false;
	}
}

static inline bool parses_to_assignable(struct exp *exp) {
	if (!exp)
		return false;
	if ((exp->type == EXP_NAME) || (exp->type == EXP_ARRAY_REF))
		return true;
	if (exp->type == EXP_UNARY)
		return parses_to_assignable(exp->unary->operand);
	return false;
}

static inline bool isOpType(const struct value v) {
	return (v.type == VAL_OP) && v.num != OP_UNKNOWN;
}
static inline bool isBinaryOpVal(const struct value v) {
	return isOpType(v) && isBinaryOp(v.num);
}
static inline bool isAssignOpVal(const struct value v) {
	return isOpType(v) && isAssignOp(v.num);
}
static inline bool isSuffixVal(const struct value v) {
	return isOpType(v) && is_suffix_unary(v.num);
}
static inline bool isPrefixVal(const struct value v) {
	return isOpType(v) && is_prefix_unary(v.num);
}
static inline bool isValidOp(const struct value v, const int minPrio) {
	if (!isOpType(v))
		return false;
	if (isAssignOpVal(v) || (isPrefixVal(v) && !isBinaryOpVal(v)))
		return true;
	return getPrio(v.num) >= minPrio;
}

static inline bool isElseKey(const struct value v) {
	return (v.type == VAL_KEYWORD) && (v.num == KW_ELSE);
}

static inline bool isDelimChar(const struct value v, char match) {
	return (v.type == VAL_DELIM) && (v.ch == match);
}

static inline bool isValidInitStmt(const struct stmt *stmt) {
	return 	(stmt->type == STMT_VAR) ||
		((stmt->type == STMT_EXPR) && (stmt->exp->type == EXP_ASSIGN_OP || is_atomic(stmt->exp)));
}

//parsing atoms
static inline void parsePrint(struct lexer_ctx *lex, struct exp *exp) {
	acceptValue(lex, VAL_KEYWORD, "print");
	acceptValue(lex, VAL_DELIM, "(");

	init_exp_call(lex, exp, KW_PRINT);

	exp->call->arg = init_exp(lex);
	parse_exp(0, lex, exp->call->arg);

	acceptValue(lex, VAL_DELIM, ")");
}
static inline void parseInput(struct lexer_ctx *lex, struct exp *exp) {
	acceptValue(lex, VAL_KEYWORD, "input");
	acceptValue(lex, VAL_DELIM, "(");
	acceptValue(lex, VAL_DELIM, ")");

	init_exp_call(lex, exp, KW_INPUT);
}
static inline void parseBreak(struct lexer_ctx *lex, struct exp *exp) {
	acceptValue(lex, VAL_KEYWORD, "break");
	init_exp_call(lex, exp, KW_BREAK);
}

static void parseCall(struct lexer_ctx *lex, struct exp *exp) {
	struct value tok = lex->val;
	if (tok.type != VAL_KEYWORD)
		return;
	
	switch (tok.num) {
	case KW_PRINT:
		parsePrint(lex, exp);
		break;
	case KW_INPUT:
		parseInput(lex, exp);
		break;
	case KW_BREAK:
		parseBreak(lex, exp);
		break;
	default:
		raise_syntax_error(ERR_INV_VAL, lex);
	}
}

static inline void parseSuffix(struct exp  *left, struct lexer_ctx *lex, struct exp *exp) {
	init_exp_unary(lex, exp, false);
	exp->unary->operand = left;
	exp->unary->op = stealNextOp(lex);
}

static inline void parsePrefix(struct lexer_ctx *lex, struct exp *exp) {
	if (!lex || !isPrefixVal(lex->val))
		raise_syntax_error(ERR_INV_OP, lex);

	//not folding in so that stealNextStr precedes parseSuffix and parse_atom 
	

	init_exp_unary(lex, exp, true);
	exp->unary->op = stealNextOp(lex);
	exp->unary->operand  = init_exp(lex);
	if (!parse_assignable(lex, exp->unary->operand))
		raise_syntax_error(ERR_INV_EXP, lex);
}

static inline void parseParenthesis(struct lexer_ctx *lex, struct exp *exp) {
    acceptValue(lex, VAL_DELIM, "(");
    parse_exp(0, lex, exp);
    acceptValue(lex, VAL_DELIM, ")");
}

static inline void parseArrayLit(struct lexer_ctx *lex, struct exp *exp)
{
	acceptValue(lex, VAL_DELIM, "{");

	int len = 0;
	init_exp_array_lit(lex, exp, DEFAULT_CAP_SIZE);

	while (parserCanProceed(lex) && !isDelimChar(lex->val, '}')) {
		parse_exp(0, lex, exp->array_lit->array + len++);
		if (!hasCommaNext(lex))
			break;
		acceptValue(lex, VAL_DELIM, ",");

		if (len >= exp->array_lit->size) { //TODO: add test?
			exp->array_lit->size *= 2;
			struct exp *temp = realloc(exp->array_lit->array, exp->array_lit->size);
			if (!temp)
				raise_syntax_error(ERR_NO_MEM, lex);

			exp->array_lit->array = temp;
		}
	}

	acceptValue(lex, VAL_DELIM, "}");

	set_exp_arraylit_len(lex, exp, len);
}

static inline void parseNum(struct lexer_ctx *lex, struct exp *exp) {
	exp->type = EXP_NUM;
	exp->num = lex->val.num;
	nextValue(lex);
}

static inline void parseStr(struct lexer_ctx *lex, struct exp *exp) {
	char *str = stealNextString(lex);
	
	size_t len = strlen(str);

	if (len > INT_MAX) {
		free(str);
		raise_syntax_error(ERR_TOO_LONG, lex);
	}

	init_exp_array_lit(lex, exp, len);
	for (size_t i = 0; i < len; i++) {
		struct exp *curr = exp->array_lit->array + i;
		curr->type = EXP_NUM;
		curr->num = str[i];
	}
	set_exp_arraylit_len(lex, exp, len);
	free(str);
}

static void parseArrayRef(struct lexer_ctx *lex, struct exp *exp) {
	if (!isDelimChar(lex->val, '['))
		return;

	acceptValue(lex, VAL_DELIM, "[");
	struct exp *name = init_exp(lex);
	swap_exps(exp, name);
	
	
	init_exp_array_ref(lex, exp, name);
	if (!isDelimChar(lex->val, ']')) {
		exp->array_ref->index = init_exp(lex);
		parse_exp(0, lex, exp->array_ref->index);
	}

	acceptValue(lex, VAL_DELIM, "]");
	parseArrayRef(lex, exp);
}

static inline void parseName(struct lexer_ctx *lex, struct exp *exp)
{
	exp->type = EXP_NAME;
	exp->name = stealNextName(lex);
	parseArrayRef(lex, exp);
}

bool parse_assignable(struct lexer_ctx *lex, struct exp *exp) {
	switch (lex->val.type) {
		case VAL_OP:
			parsePrefix(lex, exp);
			break;
		case VAL_NAME:
			parseName(lex, exp);
			break;
		default:
			return false;
	}

	if (isSuffixVal(lex->val)) {
		struct exp *left = init_exp(lex);
		swap_exps(exp, left);
		parseSuffix(left, lex, exp);
	}

	return true;
}

//ensure at the end of parse_atom it checks for a unary suffix

void parse_atom(struct lexer_ctx *lex, struct exp *exp) {
    	if (!parserCanProceed(lex))
		return;

	if (parse_assignable(lex, exp))
		return;

    	switch (lex->val.type) {
	case VAL_NUM:
		parseNum(lex, exp);
		break;
	case VAL_STR:
		parseStr(lex, exp);
		break;
	case VAL_DELIM:
		if (lex->val.ch == '(')
			parseParenthesis(lex, exp);
		else if (lex->val.ch == '{')
			parseArrayLit(lex, exp);
		break;
	case VAL_KEYWORD: // this is already covered by parse_single_stmt
		parseCall(lex, exp);
		break;
	default: //mainly to shut up the compiler
		break;
	}

	
}

void parse_exp(int minPrio, struct lexer_ctx *lex, struct exp *exp)
{
    	parse_atom(lex, exp);

    	while ((parserCanProceed(lex)) && isValidOp(lex->val, minPrio)) {
		struct exp *left = init_exp(lex);
		swap_exps(exp, left);

		init_binary(lex, exp, EXP_BINARY_OP, left);
		enum operator op = stealNextOp(lex);
		exp->type = (isAssignOp(op)) ?  EXP_ASSIGN_OP: EXP_BINARY_OP;
	
		if (isAssignOp(op) && !parses_to_assignable(left))
			raise_syntax_error(ERR_INV_EXP, lex);

		exp->op->left = left;
		exp->op->op = op;
		exp->op->right = init_exp(lex);
		parse_exp(getPrio(exp->op->op)+1, lex, exp->op->right);

		exp->line_num = left->line_num;
		exp->start_col = left->line_num;
	}
}


//parsing stmts
static void parseVar(struct lexer_ctx *lex, bool is_mutable, struct stmt *exp) {
	enum key_type key = is_mutable ? KW_VAR : KW_VAL;
	acceptValue(lex, VAL_KEYWORD, getKeyStr(key));

	init_varStmt(lex, exp, is_mutable);

	exp->var->name =  init_exp(lex);
	parseName(lex, exp->var->name);

	if (atSemicolon(lex))
		return;

	acceptValue(lex, VAL_OP, "=");

	exp->var->value = init_exp(lex);
	parse_exp(0, lex, exp->var->value);

	if (!exps_are_compatable(exp->var->name, exp->var->value))
		raise_syntax_error(ERR_INV_EXP, lex);
}

static void parseWhile(struct lexer_ctx *lex, struct stmt *exp) {
	init_loopStmt(lex, exp);

	acceptValue(lex, VAL_KEYWORD, "while");
	acceptValue(lex, VAL_DELIM, "(");

	exp->loop->cond = init_exp(lex);
	parse_exp(0, lex, exp->loop->cond);

	acceptValue(lex, VAL_DELIM, ")");

	if (!atSemicolon(lex)) {
		acceptValue(lex, VAL_DELIM, "{");

		exp->loop->body = init_stmt(lex);
		parse_stmt(lex, exp->loop->body);

		acceptValue(lex, VAL_DELIM, "}");
	}
}

static void parseFor(struct lexer_ctx *lex, struct stmt *exp) {
	/* structure of a for loop:
        "for" "("<initialization> ";" <condition> ";" <update> ")" "{" <body> "}" <next>
        
        Output structure:
        <initialization> -> <loop> -> parse_stmt(lex)
							  \/
                    <body> -> <update>
    */
	int for_start_pos = lex->line_pos;
	acceptValue(lex, VAL_KEYWORD, "for");
	acceptValue(lex, VAL_DELIM, "(");
	
	parse_single_stmt(lex, exp); //init
	
	if (!isValidInitStmt(exp))
		raise_error(ERR_INV_EXP);

	exp->next = init_stmt(lex);
	exp->next->start_col = for_start_pos;
	init_loopStmt(lex, exp->next);
	struct stmt *loop = exp->next;

	loop->loop->cond = init_exp(lex);
	parse_exp(0, lex, loop->loop->cond);
	
	acceptValue(lex, VAL_DELIM, ";");

	loop->next = init_stmt(lex);
	init_expStmt(loop->next, init_exp(lex));
	struct stmt *update = loop->next;
	
	parse_exp(0, lex, update->exp);

	acceptValue(lex, VAL_DELIM, ")");
	if (atSemicolon(lex)) {
		acceptValue(lex, VAL_DELIM, ";");
		loop->loop->body = update;
	} else {
		acceptValue(lex, VAL_DELIM, "{");
		
		loop->loop->body = init_stmt(lex);
		parse_stmt(lex, loop->loop->body);
		struct stmt *curr = loop->loop->body;
		if (!curr || curr->type == STMT_EMPTY)
			raise_syntax_error(ERR_INV_STMT, lex);
		
		while (curr->next != NULL)
			curr = curr->next;

		curr->next = update;
	
		acceptValue(lex, VAL_DELIM, "}");
	} 
	loop->next = NULL;

	
}

static void parseIf(struct lexer_ctx *lex, struct stmt *exp) {
	init_ifStmt(lex, exp);

	acceptValue(lex, VAL_KEYWORD, "if");
	acceptValue(lex, VAL_DELIM, "(");

	exp->ifStmt->cond = init_exp(lex);
	parse_exp(0, lex, exp->ifStmt->cond);

	acceptValue(lex, VAL_DELIM, ")");
	acceptValue(lex, VAL_DELIM, "{");

	exp->ifStmt->thenStmt = init_stmt(lex);
	parse_stmt(lex, exp->ifStmt->thenStmt);

	acceptValue(lex, VAL_DELIM, "}");

	if ((lex->val.type == VAL_KEYWORD) && (lex->val.num == KW_ELSE)) {
		acceptValue(lex, VAL_KEYWORD, "else");

		exp->ifStmt->elseStmt = init_stmt(lex);
		struct stmt *elseStmt = exp->ifStmt->elseStmt;
		if ((lex->val.type == VAL_KEYWORD) && (lex->val.num == KW_IF)) {
			parseIf(lex, elseStmt);
		} else {
			if (!isDelimChar(lex->val, '{'))
				raise_syntax_error(ERR_BAD_ELSE, lex);
			acceptValue(lex, VAL_DELIM, "{");
			parse_stmt(lex, elseStmt);
			acceptValue(lex, VAL_DELIM, "}");
		}
	}
}

void parse_single_stmt(struct lexer_ctx *lex, struct stmt *exp) {
	if (!lex || !hasNextStmt(lex))
		return;
	
	struct value tok = lex->val;
	
	if (tok.type == VAL_KEYWORD) {
		switch (tok.num) {
			case KW_VAR:
			case KW_VAL:
				parseVar(lex, tok.num == KW_VAR, exp);
				acceptValue(lex, VAL_DELIM, ";");
				break;
			case KW_WHILE:
				parseWhile(lex, exp);
				break;
			case KW_FOR:
				parseFor(lex, exp);
				break;
			case KW_IF:
				parseIf(lex, exp);
				break;
			default:
				exp->type = STMT_EXPR;
				exp->exp = init_exp(lex);
				parseCall(lex, exp->exp);
				acceptValue(lex, VAL_DELIM, ";");
				break;
		}
	} else {
		exp->type = STMT_EXPR;
		exp->exp = init_exp(lex);
		parse_exp(0, lex, exp->exp);
		acceptValue(lex, VAL_DELIM, ";");
	}
}

void parse_stmt(struct lexer_ctx *lex, struct stmt *exp) {
	struct stmt *curr = exp;
	parse_single_stmt(lex, curr);
	if (curr->next == curr)
		curr->next = NULL;
	while (parserCanProceed(lex)) {
		curr->next = init_stmt(lex);
		curr = curr->next;
		parse_single_stmt(lex, curr);
	}
}

struct stmt *parse_file(const char *filename) {
    struct lexer_ctx *lex = readInFile(filename);
	if (!lex)
		raise_syntax_error(ERR_NO_FILE, lex);

	parse_stmt(lex, lex->root);
	struct stmt *out = lex->root;

	lex->root = NULL;
	killReader(lex);
	return out;
}
