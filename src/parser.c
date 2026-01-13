#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "structs.h"
#include "value.h"
#include "exp.h"
#include "stmt.h"
#include "reader.h"
#include "parser.h"
#include "utils.h"


//utility functions

void set_next_stmt(struct reader *r, struct stmt *stmt)
{
	r->curr_stmt->next = stmt;
	r->curr_stmt = stmt;
}

struct stmt *stealRoot(struct reader *r)
{
	struct stmt *root = r->root;

	if (!root)
		return NULL;

	if (root->next == root) {
		r->root = NULL;
		free(root);
		return NULL;
	}

	struct stmt *first = root->next;
	root->next = NULL;
	free(root);
	r->root = NULL;
	return first;
}


// tempermental: DO NOT TOUCH WITHOUT REASON
struct stmt *push_context(struct reader *r)
{
	struct stmt *saved = r->curr_stmt;

	struct stmt *dummy = init_stmt();
	dummy->next = dummy;
	set_next_stmt(r, dummy);
	return saved;
}

struct stmt *pop_context(struct reader *r, struct stmt *saved)
{
	r->curr_stmt = saved;
	struct stmt *dummy = saved->next;
	saved->next = NULL;
	struct stmt *head = dummy->next;
	dummy->next = NULL;

	free_stmt(dummy);

	return head;
}


//parsing calls
struct exp  *parsePrint(struct reader *r)
{
	acceptValue(r, VAL_KEYWORD, "print");
    	acceptValue(r, VAL_DELIM, "(");

	struct exp  *args = parse_exp(0, r);
	
	acceptValue(r, VAL_DELIM, ")");

	return init_call(KW_PRINT, args, r);;
}

struct exp  *parseInput(struct reader *r)
{
	acceptValue(r, VAL_KEYWORD, "input");
    	acceptValue(r, VAL_DELIM, "(");
	acceptValue(r, VAL_DELIM, ")");
	
	return init_call(KW_INPUT, NULL, r);
}

struct exp  *parseBreak(struct reader *r)
{
	acceptValue(r, VAL_KEYWORD, "break");
    	return init_call(KW_BREAK, NULL, r);
}

struct exp  *parseCall(struct reader *r)
{
	struct value *tok = peekValue(r);
	if ((!tok) || (tok->type != VAL_KEYWORD))
		return NULL;
	
	switch (tok->key) {
	case KW_PRINT:
		return parsePrint(r);
	case KW_INPUT:
		return parseInput(r);
	case KW_BREAK:
		return parseBreak(r);
	default:
		raise_syntax_error("invalid value", r);
	}
	return NULL;
}

//parsing atoms
struct exp  *parseSuffix(struct exp  *left, struct reader *r)
{
	if (!isSuffixOp(peekValue(r)))
		return left;

	return init_unary(left, stealNextString(r), NULL, r);
}

struct exp  *parseUnary(struct reader *r)
{
	//not folding in so that stealNextStr precedes parseSuffix and parse_atom 
    	char *op = stealNextString(r);
    	return init_unary(NULL, op, parseSuffix(parse_atom(r), r), r);
}

struct exp  *parseParenthesis(struct reader *r)
{
    acceptValue(r, VAL_DELIM, "(");

    struct exp  *exp = parse_exp(0, r);

    acceptValue(r, VAL_DELIM, ")");
    return exp;
}

struct exp  *parseNested(struct reader *r)
{
	acceptValue(r, VAL_DELIM, "{");

	struct exp  *exp = init_nested(parse_exp(0, r), r);

	acceptValue(r, VAL_DELIM, "}");
	return exp;
}

struct exp  *parseArray(struct exp  *name, struct reader *r)
{
	struct value *tok = peekValue(r);
	if (!tok || (tok->type != VAL_DELIM) || (tok->ch != '['))
		return name;

	acceptValue(r, VAL_DELIM, "[");

	struct exp  *index = parse_exp(0, r);

	acceptValue(r, VAL_DELIM, "]");

	return init_array(parseArray(name, r), index, r);
}

struct exp  *parseNum(struct reader *r) {
	int num = peekValue(r)->num;
	freeValue(getValue(r));

	return init_num(num, r);
}

struct exp  *parseStr(struct reader *r)
{ //TODO: clean up
	char *str = stealTokString(peekValue(r));
	freeValue(getValue(r));

	struct exp  *exp_str = init_nested(init_op(init_num(str[0], r), strdup(",", r), NULL, r), r);

	struct exp  *currExp = exp_str->nested;
	for (size_t i = 1; i < strlen(str) - 1; i++) {
		currExp->op.right = init_op(init_num(str[i], r), strdup(",", r), NULL, r);;
		currExp = currExp->op.right;
	}
	currExp->op.right = init_num(str[strlen(str) - 1], r);
	free(str);
	return exp_str;
}

struct exp  *parseName(struct reader *r)
{
	struct exp  *exp = init_str(stealNextString(r), r);
	exp = parseArray(exp, r);
	exp = parseSuffix(exp, r);
	return exp;
}

struct exp  *parse_atom(struct reader *r)
{
    if (!peekValue(r) || atSemicolon(r) || !hasNextStmt(r))
		return NULL;

    switch (peekValue(r)->type) {
	case VAL_OP:
		return parseUnary(r);
	case VAL_NUM:
		return parseNum(r);
	case VAL_NAME:
		return parseName(r);
	case VAL_STR:
		return parseStr(r);
	case VAL_DELIM:
		switch (peekValue(r)->ch) {
		case '{':
			return parseNested(r);
		case '(':
			return parseParenthesis(r);
		default:
			raise_syntax_error("invalid character in code", r);
		}
		break;
	case VAL_KEYWORD:
		return parseCall(r);
	default:
		return NULL;
	}
	return NULL;
}

struct exp  *parse_exp(int minPrio, struct reader *r)
{
    struct exp  *left = parse_atom(r);
    if (!left)
		return NULL;

    while (parserCanProceed(r)) {
		if (isValidOp(peekValue(r), minPrio)) {
			char *op = stealNextString(r);
			left = init_op(left, op, parse_exp(getPrio(op)+1, r), r);
		} else {
			break;
		}
    }
    return left;
}


//parsing stmts
void parseVar(struct reader *r, bool is_mutable) {
	enum key_type key = is_mutable ? KW_VAR : KW_VAL;
	acceptValue(r, VAL_KEYWORD, getKeyStr(key));

	struct exp  *name = parse_atom(r);
	struct stmt *stmt = init_var(name, NULL, r);
	set_next_stmt(r, stmt);

	if (!is_mutable)
		stmt->type = STMT_VAL;

	if ((name->type != EXP_STR) && (name->type != EXP_ARRAY))
		raise_syntax_error("invalid name", r);

	if (atSemicolon(r))
		return;

	acceptValue(r, VAL_OP, "=");

	struct exp  *value = parse_exp(0, r);
	stmt->var.value = value;

	if ((name->type == EXP_ARRAY) && (value->type != EXP_NESTED))
		raise_syntax_error("array's must be initialized to a list, either {0} or a larger array.", r);
}

void parseWhile(struct reader *r) {
	acceptValue(r, VAL_KEYWORD, "while");
	acceptValue(r, VAL_DELIM, "(");

	struct exp  *cond = parse_exp(0, r);
	struct stmt *stmt = init_loop(cond, NULL, r);
	set_next_stmt(r, stmt);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	struct stmt *prev = push_context(r);
	parse_stmt(r);
	stmt->loop.body = pop_context(r, prev);

	acceptValue(r, VAL_DELIM, "}");
}

void parseFor(struct reader *r) {
	/* structure of a for loop:
        "for" "("<initialization> ";" <condition> ";" <update> ")" "{" <body> "}" <next>
        
        Output structure:
        <initialization> -> <loop> -> parse_stmt(r)
							  \/
                    <body> -> <update>
    */
	
	acceptValue(r, VAL_KEYWORD, "for");
	acceptValue(r, VAL_DELIM, "(");
	//TODO: make this get statements. (or for now just allow while loops and not for loops)
	parse_single_stmt(r); //init

	enum stmt_type tp = r->curr_stmt->type;
	if ((tp != STMT_VAR) && (tp != STMT_VAL) && (tp != STMT_EXPR))
		raise_error("for initialization is of invalid type");

	struct exp  *cond = parse_exp(0, r);
	
	acceptValue(r, VAL_DELIM, ";");

	struct exp  *update = parse_exp(0, r);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");
	
	struct stmt *prev = push_context(r);
	parse_stmt(r);
	struct stmt *body = pop_context(r, prev);
	struct stmt *curr = body;

	if (curr) {
		while (curr->next != NULL)
			curr = curr->next;
		curr->next = init_expStmt(update, r);
	} else
		body = init_expStmt(update, r);

	acceptValue(r, VAL_DELIM, "}");

	struct stmt *loop = init_loop(cond, body, r);
	set_next_stmt(r, loop);
}

void parseIf(struct reader *r) {
	acceptValue(r, VAL_KEYWORD, "if");
	acceptValue(r, VAL_DELIM, "(");

	struct exp  *cond = parse_exp(0, r);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	struct stmt *prev = push_context(r);
	parse_stmt(r);
	struct stmt *thenStmt = pop_context(r, prev);

	acceptValue(r, VAL_DELIM, "}");

	struct stmt *stmt = init_ifStmt(cond, thenStmt, r);
	set_next_stmt(r, stmt);

	if (isElseKey(peekValue(r))) {
		acceptValue(r, VAL_KEYWORD, "else");

		struct value *tok = peekValue(r);
		if (!tok)
			raise_syntax_error("expected statement after else", r);

		if ((tok->type == VAL_KEYWORD) && (tok->key == KW_IF)) {
			parseIf(r);

			if (stmt->next == NULL)
				raise_syntax_error("expected statement following else", r);
			
			r->curr_stmt = stmt;
			stmt->ifStmt.elseStmt = stmt->next;
			stmt->next = NULL;
		} else {
			acceptValue(r, VAL_DELIM, "{");

			struct stmt *prev = push_context(r);
			parse_stmt(r);
			stmt->ifStmt.elseStmt = pop_context(r, prev);

			acceptValue(r, VAL_DELIM, "}");
		}
	}
}

void parse_single_stmt(struct reader *r) {
	if (!readerIsAlive(r) || !hasNextStmt(r)) return;
	
	struct value *tok = peekValue(r);
	if (tok == NULL) return;
	
	if (tok->type == VAL_KEYWORD) {
		switch (tok->key) {
			case KW_VAR:
			case KW_VAL:
				parseVar(r, tok->key == KW_VAR);
				acceptValue(r, VAL_DELIM, ";");
				break;
			case KW_WHILE:
				parseWhile(r);
				break;
			case KW_FOR:
				parseFor(r);
				break;
			case KW_IF:
				parseIf(r);
				break;
			default:
				struct stmt *call_stmt = init_expStmt(parseCall(r), r);
				set_next_stmt(r, call_stmt);
				acceptValue(r, VAL_DELIM, ";");
				break;
		}
	} else {
		struct stmt *exp_stmt = init_expStmt(parse_exp(0, r), r);
		set_next_stmt(r, exp_stmt);
		acceptValue(r, VAL_DELIM, ";");
	}
}

void parse_stmt(struct reader *r) {
	while (hasNextStmt(r)) {
		parse_single_stmt(r);
	}
}

struct stmt *parse_file(const char *filename) {
    struct reader *r = readInFile(filename);
	if (!r)
		raise_syntax_error("failed to read in file", r);

	parse_stmt(r);
	struct stmt *out = stealRoot(r);
	killReader(r);
	return out;
}
