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
#include "value.h"
#include "exp.h"
#include "stmt.h"
#include "reader.h"
#include "parser.h"
#include "utils.h"

//parsing atoms
void parseCall(struct reader *r, struct exp *in) {
	struct value *tok = peekValue(r);
	if ((!tok) || (tok->type != VAL_KEYWORD))
		return;
	
	switch (tok->key) {
	case KW_PRINT:
		acceptValue(r, VAL_KEYWORD, "print");
		acceptValue(r, VAL_DELIM, "(");

		struct exp *exp = init_exp_or_free(r, NULL);
		parse_exp(0, r, exp);
		init_call(KW_PRINT, exp, in);

		acceptValue(r, VAL_DELIM, ")");
		break;
	case KW_INPUT:
		acceptValue(r, VAL_KEYWORD, "input");
		acceptValue(r, VAL_DELIM, "(");
		acceptValue(r, VAL_DELIM, ")");
		init_call(KW_INPUT, NULL, in);
		break;
	case KW_BREAK:
		acceptValue(r, VAL_KEYWORD, "break");
    		init_call(KW_BREAK, NULL, in);
		break;
	default:
		raise_syntax_error("invalid value", r);
	}
}

void parseSuffix(struct exp  *left, struct reader *r, struct exp *in) {
	if (!isSuffixVal(peekValue(r)))
		raise_syntax_error("expected a suffix op", r);
	struct exp *new_left = left;
	if (left == in) {
		new_left = init_exp_or_free(r, NULL);
		reinit_exp(left, new_left, r);	
	}
	
	init_unary(new_left, stealNextString(r), NULL, in);
}

void parsePrefix(struct reader *r, struct exp *in) {
	if (!isPrefixVal(peekValue(r)))
		raise_syntax_error("invalid unary prefix", r);

	//not folding in so that stealNextStr precedes parseSuffix and parse_atom 
	struct exp *exp = init_exp_or_free(r, NULL);

    	init_unary(NULL, stealNextString(r), exp, in);

	parse_atom(r, exp);
}

void parseParenthesis(struct reader *r, struct exp *in) {
    acceptValue(r, VAL_DELIM, "(");
    parse_exp(0, r, in);
    acceptValue(r, VAL_DELIM, ")");
}

void parseRightArray(struct reader *r, struct exp *in)
{
	acceptValue(r, VAL_DELIM, "{");

	int len = 0;
	//TODO: make default cap variable
	init_rightarray(NULL, 8, r, in);

	while (parserCanProceed(r) && !isEndingBracket(peekValue(r))) {
		parse_exp(0, r, in->right_array.array + len++);
		acceptValue(r, VAL_DELIM, ",");

		if (len >= in->right_array.size) {
			in->right_array.size *= 2;
			struct exp *temp = realloc(in->right_array.array, in->right_array.size);
			if (!temp)
				raise_syntax_error("failed to realloc exp list", r);

			in->right_array.array = temp;
		}
	}

	acceptValue(r, VAL_DELIM, "}");
}

void parseNum(struct reader *r, struct exp *in) {
	struct value *val = peekValue(r);
	if (val->type != VAL_NUM)
		raise_syntax_error("expected a num value", r);
	int num = val->num;
	freeValue(getValue(r));

	init_num(num, in);
}

void parseStr(struct reader *r, struct exp *in) {
	char *str = stealNextString(r);
	size_t len = strlen(str);

	if (len > INT_MAX) {
		free(str);
		raise_syntax_error("invalid string", r);
	}

	init_rightarray(NULL, (int) len, r, in);
	for (size_t i = 1; i < len; i++)
		init_num(str[i], in->right_array.array + i);

	free(str);
}

void parseName(struct reader *r, struct exp *in)
{
	init_name(stealNextString(r), in);
	struct value *tok = peekValue(r);
	if (!tok || (tok->type != VAL_DELIM) || (tok->ch != '['))
		return;

	struct exp *arr_name = init_exp_or_free(r, NULL);
	struct exp *to_free_on_fail[] = {arr_name, NULL};
	struct exp *index = init_exp_or_free(r, to_free_on_fail);
	reinit_exp(in, arr_name, r);
	init_array(arr_name, index, in);

	acceptValue(r, VAL_DELIM, "[");
	parse_exp(0, r, index);
	acceptValue(r, VAL_DELIM, "]");
}

//ensure at the end of parse_atom it checks for a unary suffix

void parse_atom(struct reader *r, struct exp *in) {
    	if (!parserCanProceed(r))
		return;

    	switch (peekValue(r)->type) {
	case VAL_EMPTY:
		raise_syntax_error("unexpected empty value", r);
		break;
	case VAL_OP:
		parsePrefix(r, in);
		break;
	case VAL_NUM:
		parseNum(r, in);
		break;
	case VAL_NAME:
		parseName(r, in);
		break;
	case VAL_STR:
		parseStr(r, in);
		break;
	case VAL_DELIM:
		switch (peekValue(r)->ch) {
		case '{':
			parseRightArray(r, in);
			break;
		case '(':
			parseParenthesis(r, in);
			break;
		default:
			raise_syntax_error("invalid character in code", r);
			break;
		}
		break;
	case VAL_KEYWORD:
		parseCall(r, in);
		break;
	}

	if (isSuffixVal(peekValue(r))) {
		struct exp *left = init_exp_or_free(r, NULL);
		reinit_exp(in, left, r);
		parseSuffix(left, r, in);
	}
}

void parse_exp(int minPrio, struct reader *r, struct exp *in)
{
    	parse_atom(r, in);

    	while ((parserCanProceed(r)) && isValidOp(peekValue(r), minPrio)) {
		struct exp *left = init_exp_or_free(r, NULL);
		reinit_exp(in, left, r);

		char *op = stealNextString(r);

		struct exp *to_free[] = {left, NULL};
		struct exp *right = init_exp_or_free_str(r, to_free, op);

		parse_exp(getPrio(op)+1, r, right);

		if (isAssignOp(op))
			init_assignop(left, op, right, in);
		else
			init_binop(left, op, right, in);
	}
}


//parsing stmts
void parseVar(struct reader *r, bool is_mutable, struct stmt *in) {
	enum key_type key = is_mutable ? KW_VAR : KW_VAL;
	acceptValue(r, VAL_KEYWORD, getKeyStr(key));

	in->type = is_mutable ? STMT_VAR : STMT_VAL;
	in->var.name =  init_exp_or_free(r, NULL);
	parseName(r, in->var.name);

	if (!is_mutable)
		in->type = STMT_VAL;

	if (atSemicolon(r))
		return;

	acceptValue(r, VAL_OP, "=");

	in->var.value = init_exp_or_free(r, NULL);
	parse_exp(0, r, in->var.value);

	if (exps_are_compatable(in->var.name, in->var.value))
		raise_syntax_error("array's must be initialized to a list, either {0} or a larger array.", r);
}

void parseWhile(struct reader *r, struct stmt *in) {
	acceptValue(r, VAL_KEYWORD, "while");
	acceptValue(r, VAL_DELIM, "(");

	struct exp  *cond = init_exp_or_free(r, NULL);
	init_loop(cond, NULL, in);

	parse_exp(0, r, cond);
	//set_next_stmt(r, in);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	in->loop.body = init_stmt_or_free(r, NULL, NULL);
	parse_stmt(r, in->loop.body);

	acceptValue(r, VAL_DELIM, "}");
}

void parseFor(struct reader *r, struct stmt *in) {
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
	
	parse_single_stmt(r, in); //init
	if (!isValidInitStmt(in))
		raise_error("for initialization is of invalid type");

	in->next = init_stmt_or_free(r, NULL, NULL);
	struct stmt *loop = in->next;
	init_loop(NULL, NULL, loop);

	loop->loop.cond = init_exp_or_free(r, NULL);
	parse_exp(0, r, loop->loop.cond);
	
	acceptValue(r, VAL_DELIM, ";");

	struct exp  *update = init_exp_or_free(r, NULL);
	parse_exp(0, r, update);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");
	
	struct exp *exps[] = {update, NULL};
	loop->loop.body = init_stmt_or_free(r, exps, NULL);
	parse_stmt(r, loop->loop.body);
	struct stmt *curr = loop->loop.body;
	if (curr) {
		while (curr->next != NULL)
			curr = curr->next;
		struct stmt *update_stmt = init_stmt_or_free(r, exps, NULL);
		init_expStmt(update, update_stmt);
	} else
		init_expStmt(update, loop->loop.body);

	acceptValue(r, VAL_DELIM, "}");
}

void parseIf(struct reader *r, struct stmt *in) {
	acceptValue(r, VAL_KEYWORD, "if");
	acceptValue(r, VAL_DELIM, "(");

	init_ifStmt(NULL, NULL, in);
	in->ifStmt.cond = init_exp_or_free(r, NULL);
	parse_exp(0, r, in->ifStmt.cond);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	in->ifStmt.thenStmt = init_stmt_or_free(r, NULL, NULL);
	parse_stmt(r, in->ifStmt.thenStmt);

	acceptValue(r, VAL_DELIM, "}");

	if (isElseKey(peekValue(r))) {
		acceptValue(r, VAL_KEYWORD, "else");
		struct value *tok = peekValue(r);
		if (!tok)
			raise_syntax_error("expected statement after else", r);

		in->ifStmt.elseStmt = init_stmt_or_free(r, NULL, NULL);
		struct stmt *elseStmt = in->ifStmt.elseStmt;
		if ((tok->type == VAL_KEYWORD) && (tok->key == KW_IF)) {
			parseIf(r, elseStmt);
		} else {
			acceptValue(r, VAL_DELIM, "{");
			parse_stmt(r, elseStmt);
			acceptValue(r, VAL_DELIM, "}");
		}
	}
}

void parse_single_stmt(struct reader *r, struct stmt *in) {
	if (!readerIsAlive(r) || !hasNextStmt(r)) return;
	
	struct value *tok = peekValue(r);
	if (tok == NULL) return;
	
	if (tok->type == VAL_KEYWORD) {
		switch (tok->key) {
			case KW_VAR:
			case KW_VAL:
				parseVar(r, tok->key == KW_VAR, in);
				acceptValue(r, VAL_DELIM, ";");
				break;
			case KW_WHILE:
				parseWhile(r, in);
				break;
			case KW_FOR:
				parseFor(r, in);
				break;
			case KW_IF:
				parseIf(r, in);
				break;
			default:
				init_expStmt(NULL, in);
				in->exp = init_exp_or_free(r, NULL);
				parseCall(r, in->exp);
				acceptValue(r, VAL_DELIM, ";");
				break;
		}
	} else {
		init_expStmt(NULL, in);
		in->exp = init_exp_or_free(r, NULL);
		parse_exp(0, r, in->exp);
		acceptValue(r, VAL_DELIM, ";");
	}
}

void parse_stmt(struct reader *r, struct stmt *in) {
	struct stmt *curr = in;
	while (hasNextStmt(r)) {
		parse_single_stmt(r, curr);
		curr->next = init_stmt_or_free(r, NULL, NULL);
		curr = curr->next;
	}
}

struct stmt *parse_file(const char *filename) {
    struct reader *r = readInFile(filename);
	if (!r)
		raise_syntax_error("failed to read in file", r);

	r->root = init_stmt_or_free(r, NULL, NULL);
	parse_stmt(r, r->root);
	struct stmt *out = r->root;
	r->root = NULL;
	killReader(r);
	return out;
}
