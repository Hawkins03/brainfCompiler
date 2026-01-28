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
#include "reader.h"
#include "parser.h"
#include "utils.h"

static inline bool hasNextStmt(struct reader *r) {

	return r && ((r->val.type != VAL_DELIM) || (r->val.ch != '}'));
}

static inline bool hasCommaNext(struct reader *r) {
	return r && (r->val.type == VAL_DELIM) && (r->val.ch == ',');
}
static inline bool atSemicolon(struct reader *r) {
	return r && (r->val.type == VAL_DELIM) && (r->val.ch == ';');
}

static inline bool parserCanProceed(struct reader *r) {
	return r && hasNextStmt(r) && !atSemicolon(r);
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
static inline void parsePrint(struct reader *r, struct exp *exp) {
	acceptValue(r, VAL_KEYWORD, "print");
	acceptValue(r, VAL_DELIM, "(");

	init_exp_call(r, exp, KW_PRINT);

	exp->call->arg = init_exp(r);
	parse_exp(0, r, exp->call->arg);

	acceptValue(r, VAL_DELIM, ")");
}
static inline void parseInput(struct reader *r, struct exp *exp) {
	acceptValue(r, VAL_KEYWORD, "input");
	acceptValue(r, VAL_DELIM, "(");
	acceptValue(r, VAL_DELIM, ")");

	init_exp_call(r, exp, KW_INPUT);
}
static inline void parseBreak(struct reader *r, struct exp *exp) {
	acceptValue(r, VAL_KEYWORD, "break");
	init_exp_call(r, exp, KW_BREAK);
}

static void parseCall(struct reader *r, struct exp *exp) {
	struct value tok = r->val;
	if (tok.type != VAL_KEYWORD)
		return;
	
	switch (tok.num) {
	case KW_PRINT:
		parsePrint(r, exp);
		break;
	case KW_INPUT:
		parseInput(r, exp);
		break;
	case KW_BREAK:
		parseBreak(r, exp);
		break;
	default:
		raise_syntax_error(ERR_INV_VAL, r);
	}
}

static inline void parseSuffix(struct exp  *left, struct reader *r, struct exp *exp) {
	init_exp_unary(r, exp, false);
	exp->unary->operand = left;
	exp->unary->op = stealNextOp(r);
}

static inline void parsePrefix(struct reader *r, struct exp *exp) {
	if (!r || !isPrefixVal(r->val))
		raise_syntax_error(ERR_INV_OP, r);

	//not folding in so that stealNextStr precedes parseSuffix and parse_atom 
	

	init_exp_unary(r, exp, true);
	exp->unary->op = stealNextOp(r);
	exp->unary->operand  = init_exp(r);
	if (!parse_assignable(r, exp->unary->operand))
		raise_syntax_error(ERR_INV_EXP, r);
}

static inline void parseParenthesis(struct reader *r, struct exp *exp) {
    acceptValue(r, VAL_DELIM, "(");
    parse_exp(0, r, exp);
    acceptValue(r, VAL_DELIM, ")");
}

static inline void parseArrayLit(struct reader *r, struct exp *exp)
{
	acceptValue(r, VAL_DELIM, "{");

	int len = 0;
	init_exp_array_lit(r, exp, DEFAULT_CAP_SIZE);

	while (parserCanProceed(r) && !isDelimChar(r->val, '}')) {
		parse_exp(0, r, exp->array_lit->array + len++);
		if (!hasCommaNext(r))
			break;
		acceptValue(r, VAL_DELIM, ",");

		if (len >= exp->array_lit->size) { //TODO: add test?
			exp->array_lit->size *= 2;
			struct exp *temp = realloc(exp->array_lit->array, exp->array_lit->size);
			if (!temp)
				raise_syntax_error(ERR_NO_MEM, r);

			exp->array_lit->array = temp;
		}
	}

	acceptValue(r, VAL_DELIM, "}");

	set_exp_arraylit_len(r, exp, len);
}

static inline void parseNum(struct reader *r, struct exp *exp) {
	exp->type = EXP_NUM;
	exp->num = r->val.num;
	nextValue(r);
}

static inline void parseStr(struct reader *r, struct exp *exp) {
	char *str = stealNextString(r);
	
	size_t len = strlen(str);

	if (len > INT_MAX) {
		free(str);
		raise_syntax_error(ERR_TOO_LONG, r);
	}

	init_exp_array_lit(r, exp, len);
	for (size_t i = 0; i < len; i++) {
		struct exp *curr = exp->array_lit->array + i;
		curr->type = EXP_NUM;
		curr->num = str[i];
	}
	set_exp_arraylit_len(r, exp, len);
	free(str);
}

static void parseArrayRef(struct reader *r, struct exp *exp) {
	if (!isDelimChar(r->val, '['))
		return;

	acceptValue(r, VAL_DELIM, "[");
	struct exp *name = init_exp(r);
	swap_exps(exp, name);
	
	
	init_exp_array_ref(r, exp, name);
	if (!isDelimChar(r->val, ']')) {
		exp->array_ref->index = init_exp(r);
		parse_exp(0, r, exp->array_ref->index);
	}

	acceptValue(r, VAL_DELIM, "]");
	parseArrayRef(r, exp);
}

static inline void parseName(struct reader *r, struct exp *exp)
{
	exp->type = EXP_NAME;
	exp->name = stealNextName(r);
	parseArrayRef(r, exp);
}

bool parse_assignable(struct reader *r, struct exp *exp) {
	switch (r->val.type) {
		case VAL_OP:
			parsePrefix(r, exp);
			break;
		case VAL_NAME:
			parseName(r, exp);
			break;
		default:
			return false;
	}

	if (isSuffixVal(r->val)) {
		struct exp *left = init_exp(r);
		swap_exps(exp, left);
		parseSuffix(left, r, exp);
	}

	return true;
}

//ensure at the end of parse_atom it checks for a unary suffix

void parse_atom(struct reader *r, struct exp *exp) {
    	if (!parserCanProceed(r))
		return;

	if (parse_assignable(r, exp))
		return;

    	switch (r->val.type) {
	case VAL_NUM:
		parseNum(r, exp);
		break;
	case VAL_STR:
		parseStr(r, exp);
		break;
	case VAL_DELIM:
		if (r->val.ch == '(')
			parseParenthesis(r, exp);
		else if (r->val.ch == '{')
			parseArrayLit(r, exp);
		break;
	case VAL_KEYWORD: // this is already covered by parse_single_stmt
		parseCall(r, exp);
		break;
	default: //mainly to shut up the compiler
		break;
	}

	
}

void parse_exp(int minPrio, struct reader *r, struct exp *exp)
{
    	parse_atom(r, exp);

    	while ((parserCanProceed(r)) && isValidOp(r->val, minPrio)) {
		struct exp *left = init_exp(r);
		swap_exps(exp, left);

		init_binary(r, exp, EXP_BINARY_OP, left);
		enum operator op = stealNextOp(r);
		exp->type = (isAssignOp(op)) ?  EXP_ASSIGN_OP: EXP_BINARY_OP;
	
		if (isAssignOp(op) && !parses_to_assignable(left))
			raise_syntax_error(ERR_INV_EXP, r);

		exp->op->left = left;
		exp->op->op = op;
		exp->op->right = init_exp(r);
		parse_exp(getPrio(exp->op->op)+1, r, exp->op->right);

		exp->line_num = left->line_num;
		exp->start_col = left->line_num;
	}
}


//parsing stmts
static void parseVar(struct reader *r, bool is_mutable, struct stmt *exp) {
	enum key_type key = is_mutable ? KW_VAR : KW_VAL;
	acceptValue(r, VAL_KEYWORD, getKeyStr(key));

	init_varStmt(r, exp, is_mutable);

	exp->var->name =  init_exp(r);
	parseName(r, exp->var->name);

	if (atSemicolon(r))
		return;

	acceptValue(r, VAL_OP, "=");

	exp->var->value = init_exp(r);
	parse_exp(0, r, exp->var->value);

	if (!exps_are_compatable(exp->var->name, exp->var->value))
		raise_syntax_error(ERR_INV_EXP, r);
}

static void parseWhile(struct reader *r, struct stmt *exp) {
	init_loopStmt(r, exp);

	acceptValue(r, VAL_KEYWORD, "while");
	acceptValue(r, VAL_DELIM, "(");

	exp->loop->cond = init_exp(r);
	parse_exp(0, r, exp->loop->cond);

	acceptValue(r, VAL_DELIM, ")");

	if (!atSemicolon(r)) {
		acceptValue(r, VAL_DELIM, "{");

		exp->loop->body = init_stmt(r);
		parse_stmt(r, exp->loop->body);

		acceptValue(r, VAL_DELIM, "}");
	}
}

static void parseFor(struct reader *r, struct stmt *exp) {
	/* structure of a for loop:
        "for" "("<initialization> ";" <condition> ";" <update> ")" "{" <body> "}" <next>
        
        Output structure:
        <initialization> -> <loop> -> parse_stmt(r)
							  \/
                    <body> -> <update>
    */
	int for_start_pos = r->line_pos;
	acceptValue(r, VAL_KEYWORD, "for");
	acceptValue(r, VAL_DELIM, "(");
	
	parse_single_stmt(r, exp); //init
	
	if (!isValidInitStmt(exp))
		raise_error(ERR_INV_EXP);

	exp->next = init_stmt(r);
	exp->next->start_col = for_start_pos;
	init_loopStmt(r, exp->next);
	struct stmt *loop = exp->next;

	loop->loop->cond = init_exp(r);
	parse_exp(0, r, loop->loop->cond);
	
	acceptValue(r, VAL_DELIM, ";");

	loop->next = init_stmt(r);
	init_expStmt(loop->next, init_exp(r));
	struct stmt *update = loop->next;
	
	parse_exp(0, r, update->exp);

	acceptValue(r, VAL_DELIM, ")");
	if (atSemicolon(r)) {
		acceptValue(r, VAL_DELIM, ";");
		loop->loop->body = update;
	} else {
		acceptValue(r, VAL_DELIM, "{");
		
		loop->loop->body = init_stmt(r);
		parse_stmt(r, loop->loop->body);
		struct stmt *curr = loop->loop->body;
		if (!curr || curr->type == STMT_EMPTY)
			raise_syntax_error(ERR_INV_STMT, r);
		
		while (curr->next != NULL)
			curr = curr->next;

		curr->next = update;
	
		acceptValue(r, VAL_DELIM, "}");
	} 
	loop->next = NULL;

	
}

static void parseIf(struct reader *r, struct stmt *exp) {
	init_ifStmt(r, exp);

	acceptValue(r, VAL_KEYWORD, "if");
	acceptValue(r, VAL_DELIM, "(");

	exp->ifStmt->cond = init_exp(r);
	parse_exp(0, r, exp->ifStmt->cond);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	exp->ifStmt->thenStmt = init_stmt(r);
	parse_stmt(r, exp->ifStmt->thenStmt);

	acceptValue(r, VAL_DELIM, "}");

	if ((r->val.type == VAL_KEYWORD) && (r->val.num == KW_ELSE)) {
		acceptValue(r, VAL_KEYWORD, "else");

		exp->ifStmt->elseStmt = init_stmt(r);
		struct stmt *elseStmt = exp->ifStmt->elseStmt;
		if ((r->val.type == VAL_KEYWORD) && (r->val.num == KW_IF)) {
			parseIf(r, elseStmt);
		} else {
			if (!isDelimChar(r->val, '{'))
				raise_syntax_error(ERR_BAD_ELSE, r);
			acceptValue(r, VAL_DELIM, "{");
			parse_stmt(r, elseStmt);
			acceptValue(r, VAL_DELIM, "}");
		}
	}
}

void parse_single_stmt(struct reader *r, struct stmt *exp) {
	if (!r || !hasNextStmt(r))
		return;
	
	struct value tok = r->val;
	
	if (tok.type == VAL_KEYWORD) {
		switch (tok.num) {
			case KW_VAR:
			case KW_VAL:
				parseVar(r, tok.num == KW_VAR, exp);
				acceptValue(r, VAL_DELIM, ";");
				break;
			case KW_WHILE:
				parseWhile(r, exp);
				break;
			case KW_FOR:
				parseFor(r, exp);
				break;
			case KW_IF:
				parseIf(r, exp);
				break;
			default:
				exp->type = STMT_EXPR;
				exp->exp = init_exp(r);
				parseCall(r, exp->exp);
				acceptValue(r, VAL_DELIM, ";");
				break;
		}
	} else {
		exp->type = STMT_EXPR;
		exp->exp = init_exp(r);
		parse_exp(0, r, exp->exp);
		acceptValue(r, VAL_DELIM, ";");
	}
}

void parse_stmt(struct reader *r, struct stmt *exp) {
	struct stmt *curr = exp;
	parse_single_stmt(r, curr);
	if (curr->next == curr)
		curr->next = NULL;
	while (parserCanProceed(r)) {
		curr->next = init_stmt(r);
		curr = curr->next;
		parse_single_stmt(r, curr);
	}
}

struct stmt *parse_file(const char *filename) {
    struct reader *r = readInFile(filename);
	if (!r)
		raise_syntax_error(ERR_NO_FILE, r);

	parse_stmt(r, r->root);
	struct stmt *out = r->root;

	r->root = NULL;
	killReader(r);
	return out;
}
