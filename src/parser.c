#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "parser.h"
#include "interp.h"
#include "utils.h"
#include "stmt.h"
#include "exp.h"

//utility functions

void set_next_stmt(Reader *r, stmt_t *stmt) {
	r->curr_stmt->next = stmt;
	r->curr_stmt = stmt;
}

stmt_t *stealRoot(Reader *r) {
	stmt_t *root = r->root;

	if (!root)
		return NULL;

	if (root->next == root) {
		r->root = NULL;
		free(root);
		return NULL;
	}

	stmt_t *first = root->next;
	root->next = NULL;
	free(root);
	r->root = NULL;
	return first;
}

stmt_t *enter_nested_context(Reader *r) {
	stmt_t *saved = r->curr_stmt;

	stmt_t *dummy = init_stmt();
	dummy->next = dummy;
	set_next_stmt(r, dummy);
	return saved;
}

stmt_t *exit_nested_context(Reader *r, stmt_t *saved) {
	r->curr_stmt = saved;
	stmt_t *dummy = saved->next;
	saved->next = NULL;
	stmt_t *head = dummy->next;
	dummy->next = NULL;

	free_stmt(dummy);

	return head;
}


exp_t *parsePrint(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "print");
    acceptToken(r, VAL_DELIM, "(");

	exp_t *args = parse_exp(0, r);
	
	acceptToken(r, VAL_DELIM, ")");

	return init_call(KW_PRINT, args, r);;
}

exp_t *parseInput(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "input");
    acceptToken(r, VAL_DELIM, "(");
	acceptToken(r, VAL_DELIM, ")");
	
	return init_call(KW_INPUT, NULL, r);
}

exp_t *parseBreak(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "break");
    return init_call(KW_BREAK, NULL, r);
}

exp_t *parseCall(Reader *r) {
	value_t* tok = peekToken(r);
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


exp_t *parseSuffix(exp_t *left, Reader *r) {
	if (!isSuffixOp(peekToken(r)))
		return left;
    char *op = stealTokString(peekToken(r));
    free_value(getToken(r));
    
	return init_unary(left, op, NULL, r);
}

exp_t *parseUnary(Reader *r) {
    char *op = stealTokString(peekToken(r));
    free_value(getToken(r));
    return init_unary(NULL, op, parseSuffix(parse_atom(r), r), r);
}

exp_t *parseParenthesis(Reader *r) {
    acceptToken(r, VAL_DELIM, "(");

    exp_t *exp = parse_exp(0, r);

    acceptToken(r, VAL_DELIM, ")");
    return exp;
}

exp_t *parseNested(Reader *r) {
	acceptToken(r, VAL_DELIM, "{");

	exp_t *exp = init_nested(parse_exp(0, r), r);

	acceptToken(r, VAL_DELIM, "}");
	return exp;
}

exp_t *parseArray(exp_t *name, Reader *r) {
	value_t *tok = peekToken(r);
	if (!tok || (tok->type != VAL_DELIM) || (tok->ch != '['))
		return name;

	acceptToken(r, VAL_DELIM, "[");

	exp_t *index = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, "]");

	return init_array(parseArray(name, r), index, r);
}

exp_t *parseStr(Reader *r) {
	char *str = stealTokString(peekToken(r));
	free_value(getToken(r));

	exp_t *exp_str = init_nested(init_op(init_num(str[0], r), strdup(",", r), NULL, r), r);

	exp_t *currExp = exp_str->nested;
	for (size_t i = 1; i < strlen(str) - 1; i++) {
		currExp->op.right = init_op(init_num(str[i], r), strdup(",", r), NULL, r);;
		currExp = currExp->op.right;
	}
	currExp->op.right = init_num(str[strlen(str) - 1], r);
	free(str);
	return exp_str;
}

exp_t *parse_atom(Reader *r) {
    value_t *tok = peekToken(r);

    if (!tok || atSemicolon(r) || !hasNextStmt(r))
		return NULL;

    switch (tok->type) {
        case VAL_OP:
    	    return parseUnary(r);
    	case VAL_NUM:
			int num = tok->num;
			free_value(getToken(r));

			return init_num(num, r);
		case VAL_NAME:
			char *name = stealTokString(tok);
			free_value(getToken(r));

			return parseSuffix(parseArray(init_str(name, r), r), r); // base name expression
		case VAL_STR:
			return parseStr(r);
		case VAL_DELIM:
			switch (tok->ch) {
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

exp_t *parse_exp(int minPrio, Reader *r) {
    exp_t *left = parse_atom(r);
    if (!left)
		return NULL;

    while ((readerIsAlive(r) && hasNextStmt(r)) && !atSemicolon(r)) {
		value_t *op = peekToken(r);
		if (!op) {
			free_exp(left);
			raise_syntax_error("reached EOF while parsing exp", r);
		}
		if (op->type == VAL_OP) {
			int prio = get_prio(op->str);
			if (((prio < minPrio) && (prio != SET_OP_PRIO)) || (prio == UNARY_OP_PRIO))
				break;

			char *opStr = stealTokString(op);
			free_value(getToken(r));

			left = init_op(left, opStr, parse_exp(prio+1, r), r);
		}
		else
			break;
    }
    return left;
}
   
void parseVar(Reader *r, bool is_mutable) {
	key_t key = is_mutable ? KW_VAR : KW_VAL;
	acceptToken(r, VAL_KEYWORD, getKeyStr(key));

	exp_t *name = parse_atom(r);
	stmt_t *stmt = init_var(name, NULL, r);
	set_next_stmt(r, stmt);

	if (!is_mutable)
		stmt->type = STMT_VAL;

	if ((name->type != EXP_STR) && (name->type != EXP_ARRAY))
		raise_syntax_error("invalid name", r);

	if (atSemicolon(r)) return;

	acceptToken(r, VAL_OP, "=");

	exp_t *value = parse_exp(0, r);
	stmt->var.value = value;

	if ((name->type == EXP_ARRAY) && (value->type != EXP_NESTED))
		raise_syntax_error("array's must be initialized to a list, either {0} or a larger array.", r);
}

void parseWhile(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "while");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *cond = parse_exp(0, r);

	stmt_t *stmt = init_loop(cond, NULL, r);
	set_next_stmt(r, stmt);

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");

	stmt_t *prev = enter_nested_context(r);
	parse_stmt(r);
	stmt->loop.body = exit_nested_context(r, prev);

	acceptToken(r, VAL_DELIM, "}");
}

void parseFor(Reader *r) {
	/* structure of a for loop:
        "for" "("<initialization> ";" <condition> ";" <update> ")" "{" <body> "}" <next>
        
        Output structure:
        <initialization> -> <loop> -> parse_stmt(r)
							  \/
                    <body> -> <update>
    */
	
	acceptToken(r, VAL_KEYWORD, "for");
	acceptToken(r, VAL_DELIM, "(");
	//TODO: make this get statements. (or for now just allow while loops and not for loops)
	parse_single_stmt(r); //init

	stmt_type_t tp = r->curr_stmt->type;
	if ((tp != STMT_VAR) && (tp != STMT_VAL) && (tp != STMT_EXPR))
		raise_error("for initialization is of invalid type");

	exp_t *cond = parse_exp(0, r);
	
	acceptToken(r, VAL_DELIM, ";");

	exp_t *update = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");
	
	stmt_t *prev = enter_nested_context(r);
	parse_stmt(r);
	stmt_t *body = exit_nested_context(r, prev);
	stmt_t *curr = body;

	if (curr) {
		while (curr->next != NULL)
			curr = curr->next;
		curr->next = init_expStmt(update, r);
	} else
		body = init_expStmt(update, r);

	acceptToken(r, VAL_DELIM, "}");

	stmt_t *loop = init_loop(cond, body, r);
	set_next_stmt(r, loop);
}

void parseIf(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "if");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *cond = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");

	stmt_t *prev = enter_nested_context(r);
	parse_stmt(r);
	stmt_t *thenStmt = exit_nested_context(r, prev);

	acceptToken(r, VAL_DELIM, "}");

	stmt_t *stmt = init_ifStmt(cond, thenStmt, r);
	set_next_stmt(r, stmt);

	value_t* tok = peekToken(r);
	if ((tok) && (tok->type == VAL_KEYWORD) && (tok->key == KW_ELSE)) {
		acceptToken(r, VAL_KEYWORD, "else");

		tok = peekToken(r);
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
			acceptToken(r, VAL_DELIM, "{");

			stmt_t *prev = enter_nested_context(r);
			parse_stmt(r);
			stmt->ifStmt.elseStmt = exit_nested_context(r, prev);

			acceptToken(r, VAL_DELIM, "}");
		}
	}
}

void parse_single_stmt(Reader *r) {
	if (!readerIsAlive(r) || !hasNextStmt(r)) return;
	
	value_t *tok = peekToken(r);
	if (tok == NULL) return;
	
	if (tok->type == VAL_KEYWORD) {
		switch (tok->key) {
			case KW_VAR:
			case KW_VAL:
				parseVar(r, tok->key == KW_VAR);
				acceptToken(r, VAL_DELIM, ";");
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
				set_next_stmt(r, init_expStmt(parseCall(r), r));
				acceptToken(r, VAL_DELIM, ";");
				break;
		}
	} else {
		set_next_stmt(r, init_expStmt(parse_exp(0, r), r));
		acceptToken(r, VAL_DELIM, ";");
	}
}

void parse_stmt(Reader *r) {
	while (hasNextStmt(r)) {
		parse_single_stmt(r);
	}
}

stmt_t *parse_file(const char *filename) {
    Reader *r = readInFile(filename);
	if (!r)
		raise_syntax_error("failed to read in file", r);

	parse_stmt(r);
	stmt_t *out = stealRoot(r);
	killReader(r);
	return out;
}
