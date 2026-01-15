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

		in->type = EXP_CALL;
		in->call.key = KW_PRINT;
		in->call.call = initExpOrKill(r);

		parse_exp(0, r, in->call.call);
		acceptValue(r, VAL_DELIM, ")");
		break;
	case KW_INPUT:
		acceptValue(r, VAL_KEYWORD, "input");
		acceptValue(r, VAL_DELIM, "(");
		acceptValue(r, VAL_DELIM, ")");
		in->type = EXP_CALL;
		in->call.key = KW_INPUT;

		break;
	case KW_BREAK:
		acceptValue(r, VAL_KEYWORD, "break");
    		in->type = EXP_CALL;
		in->call.key = KW_BREAK;
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
		new_left = initExpOrKill(r);
		swap_exps(left, new_left);	
	}
	in->type = EXP_UNARY;
	in->op.left = new_left;
	in->op.op = stealNextString(r);
}

void parsePrefix(struct reader *r, struct exp *in) {
	if (!isPrefixVal(peekValue(r)))
		raise_syntax_error("invalid unary prefix", r);

	//not folding in so that stealNextStr precedes parseSuffix and parse_atom 
	

	in->type = EXP_UNARY;
	in->op.op = stealNextString(r);
	in->op.right  = initExpOrKill(r);
	parse_atom(r, in->op.right);
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

	while (parserCanProceed(r) && !isDelimChar(peekValue(r), '}')) {
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
	
	in->type = EXP_NUM;
	in->num = val->num;
	freeValue(getValue(r));
}

void parseStr(struct reader *r, struct exp *in) {
	char *str = stealNextString(r);
	size_t len = strlen(str);

	if (len > INT_MAX) {
		free(str);
		raise_syntax_error("invalid string", r);
	}

	init_rightarray(NULL, (int) len, r, in);
	for (size_t i = 0; i < len; i++) {
		struct exp *curr = in->right_array.array + i;
		curr->type = EXP_NUM;
		curr->num = str[i];
	}

	free(str);
}

void parseArray(struct reader *r, struct exp *in) {
	struct value *tok = peekValue(r);
	if (!tok || (tok->type != VAL_DELIM) || (tok->ch != '['))
		return;

	acceptValue(r, VAL_DELIM, "[");
	struct exp *name = initExpOrKill(r);
	swap_exps(in, name);
	
	
	in->type = EXP_ARRAY;
	in->array.name = name;
	in->array.index = initExpOrKill(r);
	parse_exp(0, r, in->array.index);

	acceptValue(r, VAL_DELIM, "]");

	parseArray(r, in);
}

void parseName(struct reader *r, struct exp *in)
{
	in->type = EXP_NAME;
	in->name = stealNextString(r);
	parseArray(r, in);
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
		if (peekValue(r)->ch == '(')
			parseParenthesis(r, in);
		else if (isDelimChar(peekValue(r), '{'))
			parseRightArray(r, in);
		break;
	case VAL_KEYWORD:
		parseCall(r, in);
		break;
	}

	if (isSuffixVal(peekValue(r))) {
		struct exp *left = initExpOrKill(r);
		swap_exps(in, left);
		parseSuffix(left, r, in);
	}
}

void parse_exp(int minPrio, struct reader *r, struct exp *in)
{
    	parse_atom(r, in);

    	while ((parserCanProceed(r)) && isValidOp(peekValue(r), minPrio)) {
		struct exp *left = initExpOrKill(r);
		swap_exps(in, left);
		in->op.left = left;
		in->op.op = stealNextString(r);
		in->op.right = initExpOrKill(r);

		parse_exp(getPrio(in->op.op)+1, r, in->op.right);

		if (isAssignOp(in->op.op))
			in->type = EXP_ASSIGN_OP;
		else
			in->type = EXP_BINARY_OP;
	}
}


//parsing stmts
void parseVar(struct reader *r, bool is_mutable, struct stmt *in) {
	enum key_type key = is_mutable ? KW_VAR : KW_VAL;
	acceptValue(r, VAL_KEYWORD, getKeyStr(key));

	in->type = is_mutable ? STMT_VAR : STMT_VAL;
	in->var.name =  initExpOrKill(r);
	parseName(r, in->var.name);

	if (!is_mutable)
		in->type = STMT_VAL;

	if (atSemicolon(r))
		return;

	acceptValue(r, VAL_OP, "=");

	in->var.value = initExpOrKill(r);
	parse_exp(0, r, in->var.value);

	if (!exps_are_compatable(in->var.name, in->var.value))
		raise_syntax_error("array's must be initialized to a list, either {0} or a larger array.", r);
}

void parseWhile(struct reader *r, struct stmt *in) {
	acceptValue(r, VAL_KEYWORD, "while");
	acceptValue(r, VAL_DELIM, "(");

	in->type = STMT_LOOP;

	in->loop.cond = initExpOrKill(r);
	parse_exp(0, r, in->loop.cond);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	in->loop.body = initStmtOrKill(r);
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

	struct stmt *loop = initStmtOrKill(r);
	in->next = loop;
	loop->type = STMT_LOOP;

	loop->loop.cond = initExpOrKill(r);
	parse_exp(0, r, loop->loop.cond);
	
	acceptValue(r, VAL_DELIM, ";");

	// find a way to add it into the structure here
	loop->next = initStmtOrKill(r);
	loop->next->type = STMT_EXPR;
	loop->next->exp = initExpOrKill(r);
	parse_exp(0, r, loop->next->exp);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");
	
	loop->loop.body = initStmtOrKill(r);
	parse_stmt(r, loop->loop.body);
	struct stmt *curr = loop->loop.body;
	if (curr) {
		while (curr->next != NULL)
			curr = curr->next;

		curr->next = loop->next;
	} else {
		curr = loop->next;
	}
	loop->next = NULL;

	acceptValue(r, VAL_DELIM, "}");
}

void parseIf(struct reader *r, struct stmt *in) {
	acceptValue(r, VAL_KEYWORD, "if");
	acceptValue(r, VAL_DELIM, "(");

	in->type = STMT_IF;
	in->ifStmt.cond = initExpOrKill(r);
	parse_exp(0, r, in->ifStmt.cond);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	in->ifStmt.thenStmt = initStmtOrKill(r);
	parse_stmt(r, in->ifStmt.thenStmt);

	acceptValue(r, VAL_DELIM, "}");

	if (isElseKey(peekValue(r))) {
		acceptValue(r, VAL_KEYWORD, "else");
		struct value *tok = peekValue(r);
		if (!tok)
			raise_syntax_error("expected statement after else", r);

		in->ifStmt.elseStmt = initStmtOrKill(r);
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
				in->type = STMT_EXPR;
				in->exp = initExpOrKill(r);
				parseCall(r, in->exp);
				acceptValue(r, VAL_DELIM, ";");
				break;
		}
	} else {
		in->type = STMT_EXPR;
		in->exp = initExpOrKill(r);
		parse_exp(0, r, in->exp);
		acceptValue(r, VAL_DELIM, ";");
	}
}

void parse_stmt(struct reader *r, struct stmt *in) {
	struct stmt *curr = in;
	parse_single_stmt(r, curr);
	while (parserCanProceed(r)) {
		curr->next = initStmtOrKill(r);
		curr = curr->next;
		parse_single_stmt(r, curr);
	}
}

struct stmt *parse_file(const char *filename) {
    struct reader *r = readInFile(filename);
	if (!r)
		raise_syntax_error("failed to read in file", r);

	parse_stmt(r, r->root);
	struct stmt *out = r->root;

	r->root = NULL;
	killReader(r);
	return out;
}
