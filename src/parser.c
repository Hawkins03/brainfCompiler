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
static inline void parsePrint(struct reader *r, struct exp *in) {
	acceptValue(r, VAL_KEYWORD, "print");
	acceptValue(r, VAL_DELIM, "(");

	init_exp_call(r, in, KW_PRINT);

	in->call->arg = init_exp(r);
	parse_exp(0, r, in->call->arg);

	acceptValue(r, VAL_DELIM, ")");
}
static inline void parseInput(struct reader *r, struct exp *in) {
	acceptValue(r, VAL_KEYWORD, "input");
	acceptValue(r, VAL_DELIM, "(");
	acceptValue(r, VAL_DELIM, ")");

	init_exp_call(r, in, KW_INPUT);
}
static inline void parseBreak(struct reader *r, struct exp *in) {
	acceptValue(r, VAL_KEYWORD, "break");
	init_exp_call(r, in, KW_BREAK);
}

static void parseCall(struct reader *r, struct exp *in) {
	struct value *tok = peekValue(r);
	if ((!tok) || (tok->type != VAL_KEYWORD))
		return;
	
	switch (tok->key) {
	case KW_PRINT:
		parsePrint(r, in);
		break;
	case KW_INPUT:
		parseInput(r, in);
		break;
	case KW_BREAK:
		parseBreak(r, in);
		break;
	default:
		raise_syntax_error("invalid value", r);
	}
}

static inline void parseSuffix(struct exp  *left, struct reader *r, struct exp *in) {
	if (!isSuffixVal(peekValue(r)))
		raise_syntax_error("expected a suffix op", r);
	struct exp *operand = left;
	if (left == in) {
		operand = init_exp(r);
		swap_exps(left, operand);	
	}
	init_exp_unary(r, in, false);
	in->unary->op = stealNextString(r);
	in->unary->operand = operand;
}

static inline void parsePrefix(struct reader *r, struct exp *in) {
	if (!isPrefixVal(peekValue(r)))
		raise_syntax_error("invalid unary prefix", r);

	//not folding in so that stealNextStr precedes parseSuffix and parse_atom 
	

	init_exp_unary(r, in, true);
	in->unary->op = stealNextString(r);
	in->unary->operand  = init_exp(r);
	parse_atom(r, in->unary->operand);
}

static inline void parseParenthesis(struct reader *r, struct exp *in) {
    acceptValue(r, VAL_DELIM, "(");
    parse_exp(0, r, in);
    acceptValue(r, VAL_DELIM, ")");
}

static inline void parseArrayLit(struct reader *r, struct exp *in)
{
	acceptValue(r, VAL_DELIM, "{");

	int len = 0;
	//TODO: make default cap variable
	init_exp_array_lit(r, in, DEFAULT_CAP_SIZE);

	while (parserCanProceed(r) && !isDelimChar(peekValue(r), '}')) {
		parse_exp(0, r, in->array_lit->array + len++);
		acceptValue(r, VAL_DELIM, ",");

		if (len >= in->array_lit->size) {
			in->array_lit->size *= 2;
			struct exp *temp = realloc(in->array_lit->array, in->array_lit->size);
			if (!temp)
				raise_syntax_error("failed to realloc exp list", r);

			in->array_lit->array = temp;
		}
	}

	acceptValue(r, VAL_DELIM, "}");
}

static inline void parseNum(struct reader *r, struct exp *in) {
	struct value *val = peekValue(r);
	if (val->type != VAL_NUM)
		raise_syntax_error("expected a num value", r);
	
	in->type = EXP_NUM;
	in->num = val->num;
	freeValue(getValue(r));
}

static inline void parseStr(struct reader *r, struct exp *in) {
	char *str = stealNextString(r);
	size_t len = strlen(str);

	if (len > INT_MAX) {
		free(str);
		raise_syntax_error("invalid string", r);
	}

	init_exp_array_lit(r, in, len);
	for (size_t i = 0; i < len; i++) {
		struct exp *curr = in->array_lit->array + i;
		curr->type = EXP_NUM;
		curr->num = str[i];
	}

	free(str);
}

static void parseArrayRef(struct reader *r, struct exp *in) {
	struct value *tok = peekValue(r);
	if (!tok || (tok->type != VAL_DELIM) || (tok->ch != '['))
		return;

	acceptValue(r, VAL_DELIM, "[");
	struct exp *name = init_exp(r);
	swap_exps(in, name);
	
	
	init_exp_array_ref(r, in);
	in->array_ref->name = name;
	in->array_ref->index = init_exp(r);
	parse_exp(0, r, in->array_ref->index);

	acceptValue(r, VAL_DELIM, "]");

	parseArrayRef(r, in);
}

static inline void parseName(struct reader *r, struct exp *in)
{
	in->type = EXP_NAME;
	in->name = stealNextString(r);
	parseArrayRef(r, in);
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
			parseArrayLit(r, in);
		break;
	case VAL_KEYWORD:
		parseCall(r, in);
		break;
	}

	if (isSuffixVal(peekValue(r))) {
		struct exp *left = init_exp(r);
		swap_exps(in, left);
		parseSuffix(left, r, in);
	}
}

void parse_exp(int minPrio, struct reader *r, struct exp *in)
{
    	parse_atom(r, in);

    	while ((parserCanProceed(r)) && isValidOp(peekValue(r), minPrio)) {
		struct exp *left = init_exp(r);
		swap_exps(in, left);

		char *op = stealNextString(r);
		init_binary(r, in, (isAssignOp(op)) ?  EXP_ASSIGN_OP: EXP_BINARY_OP);

		in->op->left = left;
		in->op->op = op;
		in->op->right = init_exp(r);
		parse_exp(getPrio(in->op->op)+1, r, in->op->right);
	}
}


//parsing stmts
static void parseVar(struct reader *r, bool is_mutable, struct stmt *in) {
	enum key_type key = is_mutable ? KW_VAR : KW_VAL;
	acceptValue(r, VAL_KEYWORD, getKeyStr(key));

	init_varStmt(r, in, is_mutable);

	in->var->name =  init_exp(r);
	parseName(r, in->var->name);

	if (atSemicolon(r))
		return;

	acceptValue(r, VAL_OP, "=");

	in->var->value = init_exp(r);
	parse_exp(0, r, in->var->value);

	if (!exps_are_compatable(in->var->name, in->var->value))
		raise_syntax_error("array's must be initialized to a list, either {0} or a larger array.", r);
}

static void parseWhile(struct reader *r, struct stmt *in) {
	acceptValue(r, VAL_KEYWORD, "while");
	acceptValue(r, VAL_DELIM, "(");

	init_loopStmt(r, in);

	in->loop->cond = init_exp(r);
	parse_exp(0, r, in->loop->cond);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	in->loop->body = init_stmt(r);
	parse_stmt(r, in->loop->body);

	acceptValue(r, VAL_DELIM, "}");
}

static void parseFor(struct reader *r, struct stmt *in) {
	/* structure of a for loop:
        "for" "("<initialization> ";" <condition> ";" <update> ")" "{" <body> "}" <next>
        
        Output structure:
        <initialization> -> <loop> -> parse_stmt(r)
							  \/
                    <body> -> <update>
    */
	
	acceptValue(r, VAL_KEYWORD, "for");
	acceptValue(r, VAL_DELIM, "(");
	
	parse_single_stmt(r, in); //init
	if (!isValidInitStmt(in))
		raise_error("for initialization is of invalid type");

	in->next = init_stmt(r);
	init_loopStmt(r, in->next);
	struct stmt *loop = in->next;

	loop->loop->cond = init_exp(r);
	parse_exp(0, r, loop->loop->cond);
	
	acceptValue(r, VAL_DELIM, ";");

	loop->next = init_stmt(r);
	init_expStmt(loop->next, init_exp(r));
	parse_exp(0, r, loop->next->exp);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");
	
	loop->loop->body = init_stmt(r);
	parse_stmt(r, loop->loop->body);
	struct stmt *curr = loop->loop->body;
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

static void parseIf(struct reader *r, struct stmt *in) {
	acceptValue(r, VAL_KEYWORD, "if");
	acceptValue(r, VAL_DELIM, "(");

	init_ifStmt(r, in);
	in->ifStmt->cond = init_exp(r);
	parse_exp(0, r, in->ifStmt->cond);

	acceptValue(r, VAL_DELIM, ")");
	acceptValue(r, VAL_DELIM, "{");

	in->ifStmt->thenStmt = init_stmt(r);
	parse_stmt(r, in->ifStmt->thenStmt);

	acceptValue(r, VAL_DELIM, "}");

	if (isElseKey(peekValue(r))) {
		acceptValue(r, VAL_KEYWORD, "else");
		struct value *tok = peekValue(r);
		if (!tok)
			raise_syntax_error("expected statement after else", r);

		in->ifStmt->elseStmt = init_stmt(r);
		struct stmt *elseStmt = in->ifStmt->elseStmt;
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
				in->exp = init_exp(r);
				parseCall(r, in->exp);
				acceptValue(r, VAL_DELIM, ";");
				break;
		}
	} else {
		in->type = STMT_EXPR;
		in->exp = init_exp(r);
		parse_exp(0, r, in->exp);
		acceptValue(r, VAL_DELIM, ";");
	}
}

void parse_stmt(struct reader *r, struct stmt *in) {
	struct stmt *curr = in;
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
		raise_syntax_error("failed to read in file", r);

	parse_stmt(r, r->root);
	struct stmt *out = r->root;

	r->root = NULL;
	killReader(r);
	return out;
}
