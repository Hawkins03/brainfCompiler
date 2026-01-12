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


void free_exp(exp_t *exp) {
    if (exp == NULL)
		return;
    switch (exp->type) {
		case EXP_STR:
			free(exp->str);
			exp->str = NULL;
			break;
		case EXP_NUM:
			break;
		case EXP_UNARY:
		case EXP_OP:
			free(exp->op.op);
			exp->op.op = NULL;
			free_exp(exp->op.left);
			exp->op.left = NULL;
			free_exp(exp->op.right);
			exp->op.right = NULL;
			break;
		case EXP_EMPTY:
			break;
		case EXP_CALL:
			free_exp(exp->call.call);
			exp->call.call = NULL;
			break;
		case EXP_ARRAY:
			free_exp(exp->arr.name);
			exp->arr.name = NULL;
			free_exp(exp->arr.index);
			exp->arr.index = NULL;
			break;
		case EXP_NESTED:
			free_exp(exp->nested);
			exp->nested = NULL;
			break;
    }
    free(exp);
}

void free_stmt(stmt_t *root, stmt_t *stmt) {
	if (!stmt)
		return;
	switch (stmt->type) {
		case STMT_EMPTY:
			break;
		case STMT_VAR:
		case STMT_VAL:
			free_exp(stmt->var.name);
			stmt->var.name = NULL;
			free_exp(stmt->var.value);
			stmt->var.name = NULL;
			break;
		case STMT_LOOP:
			free_exp(stmt->loop.cond);
			stmt->loop.cond = NULL;
			free_stmt(root, stmt->loop.body);
			stmt->loop.body = NULL;
			break;
		case STMT_IF:
			free_exp(stmt->ifStmt.cond);
			stmt->ifStmt.cond = NULL;
			free_stmt(root, stmt->ifStmt.thenStmt);
			stmt->ifStmt.thenStmt = NULL;
			free_stmt(root, stmt->ifStmt.elseStmt);
			stmt->ifStmt.elseStmt = NULL;
			break;
		case STMT_EXPR:
			free_exp(stmt->exp);
			stmt->exp = NULL;
			break;
	}

	if (stmt->next == stmt)
		return;
	if (!root || (stmt->next != root))
		free_stmt(root, stmt->next);
	stmt->next = NULL;
	free(stmt);
}

void print_exp(const exp_t *exp) {
    if (!exp) {
		printf("NULL");
		return;
	}
	switch (exp->type) {
		case EXP_STR:
			printf("NAME(%s)", exp->str);
			break;
		case EXP_NUM:
			printf("NUM(%d)", exp->num);
			break;
		case EXP_OP:
			printf("OP( ");
			print_exp(exp->op.left);
			printf(", %s, ", exp->op.op);
			print_exp(exp->op.right);
			printf(")");
			break;
		case EXP_UNARY:
			printf("UNARY(");
			print_exp(exp->op.left);
			printf(", %s, \n", exp->op.op);
			print_exp(exp->op.right);
			printf(")");
			break;
		case EXP_CALL:
			printf("CALL(%s,", getKeyStr(exp->call.key));
			print_exp(exp->call.call);
			printf(")");
			break;
		case EXP_ARRAY:
			printf("ARR(");
			print_exp(exp->arr.name);
			printf(", ");
			print_exp(exp->arr.index);
			printf(")");
			break;
		case EXP_NESTED:
			printf("NESTED(");
			print_exp(exp->nested);
			printf(")");
			break;
		case EXP_EMPTY:
			printf("EMPTY()");
			break;
    }   
}

void print_stmt(const stmt_t *stmt) {
	if (!stmt) {
		printf("NULL\n");
		return;
	}
	switch (stmt->type) {
		case STMT_EMPTY:
			printf("EMPTY()");
			printf(";\n");
			break;
		case STMT_VAR:
			printf("VAR(");
			print_exp(stmt->var.name);
			printf(", ");
			print_exp(stmt->var.value);
			printf(")");
			printf(";\n");
			break;
		case STMT_VAL:
			printf("VAL(");
			print_exp(stmt->var.name);
			printf(", ");
			print_exp(stmt->var.value);
			printf(")");
			printf(";\n");
			break;
		case STMT_LOOP:
			printf("LOOP(");
			print_exp(stmt->loop.cond);
			printf(") {\n");
			print_stmt(stmt->loop.body);
			printf("}");
			break;
		case STMT_IF:
			printf("IF(");
			print_exp(stmt->ifStmt.cond);
			printf(") {\n");
			print_stmt(stmt->ifStmt.thenStmt);
			printf("} else {\n");
			print_stmt(stmt->ifStmt.elseStmt);
			printf("}\n");
			break;
		case STMT_EXPR:
			printf("EXP_");
			print_exp(stmt->exp);
			printf(";\n");
			break;
	}

	if (stmt->next == stmt)
		return;
	print_stmt(stmt->next);
}

//utility functions

char *get_name_from_exp(exp_t *exp) {
	if (!exp)
		return NULL;
	
	switch (exp->type) {
		case EXP_ARRAY:
			return get_name_from_exp(exp->arr.name);
		case EXP_UNARY:
			char *left_name = get_name_from_exp(exp->op.left);
			if (left_name)
				return left_name;
			return get_name_from_exp(exp->op.right);
		case EXP_STR:
			return exp->str;
		default:
			return NULL;
	}
}

bool is_array(exp_t *exp) {
	return exp && (exp->type == EXP_ARRAY);
}

bool is_unary_exp(exp_t *exp) {
	return (exp && ((exp->type == EXP_ARRAY) || (exp->type == EXP_UNARY) || (exp->type == EXP_STR)));
}

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

bool compare(exp_t *exp1, exp_t *exp2) {
	if (!((exp1 != NULL) ^ (exp2 != NULL)) || (exp1->type != exp2->type))
		return false;
	switch (exp1->type) {
		case EXP_EMPTY:
			return true;
		case EXP_STR:
			return !strcmp(exp1->str, exp2->str);
		case EXP_NUM:
			return exp1->num == exp2->num;
		case EXP_UNARY:
		case EXP_OP:
			return (compare(exp1->op.left, exp2->op.left) && strcmp(exp1->op.op, exp2->op.op) && compare(exp1->op.right, exp2->op.right));
		case EXP_CALL:
			return ((exp1->call.key == exp2->call.key) && compare(exp1->call.call, exp2->call.call));
		case EXP_ARRAY:
			return ((exp1->arr.index == exp2->arr.index) && compare(exp1->arr.name, exp2->arr.name));
		case EXP_NESTED:
			return compare(exp1->nested, exp2->nested);
		default:
			return false;
	}
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

	free_stmt(dummy, dummy);

	return head;
}


// exp initialization functions
exp_t *init_exp() {
    exp_t *exp = calloc(1, sizeof(*exp));
	
    if (exp == NULL)
		return NULL;

    exp->type = EXP_EMPTY;
    return exp;
}

exp_t *init_op(exp_t *left, char *op, exp_t *right, Reader *r) {
    exp_t *exp = init_exp();
	if (!exp) {
		free_exp(left);
		free(op);
		free_exp(right);
		raise_syntax_error("failed to initialize exp", r);
	}
   
	exp->type = EXP_OP;
    exp->op.left = left;
    exp->op.op = op;
    exp->op.right = right;
    return exp;
}

exp_t *init_unary(exp_t *left, char *op, exp_t *right, Reader *r) {
    exp_t *exp = init_op(left, op, right, r);
	exp->type = EXP_UNARY;
    return exp;
}

exp_t *init_num(int num, Reader *r) {
    exp_t *exp = init_exp();
	if (!exp)
		raise_syntax_error("failed to initialize exp", r);

    exp->type = EXP_NUM;
    exp->num= num;
    return exp;
}

exp_t *init_str(char *str, Reader *r) {
	exp_t *exp = init_exp();
	if (!exp) {
		free(str);
		raise_syntax_error("failed to initialize exp", r);
	}

    exp->type = EXP_STR;
    exp->str = str;
    return exp;
}

exp_t *init_call(key_t key, exp_t *call, Reader *r) {
	exp_t *exp = init_exp();
	if (!exp) {
		free_exp(call);
		raise_syntax_error("failed to initialize exp", r);
	}

	exp->type = EXP_CALL;
	exp->call.key = key;
	exp->call.call = call;
	return exp;
}

exp_t *init_array(exp_t *name, exp_t *index, Reader *r) {
    exp_t *exp = init_exp(); //TODO: ensure index is an int-type, and name is a str-type
	if (!exp) {
		free_exp(name);
		free_exp(index);
		raise_syntax_error("failed to initialize exp", r);
	}

    exp->type = EXP_ARRAY;
    exp->arr.name = name;
    exp->arr.index = index;
    return exp;
}

exp_t *init_nested(exp_t *op, Reader *r) {
	exp_t *exp = init_exp();
	if (!exp) {
		free_exp(op);
		killReader(r);
		raise_syntax_error("failed to initialize exp", r);
	}

	exp->type = EXP_NESTED;
	exp->nested = op;
	return exp;
}


// stmt initialization functions
stmt_t *init_stmt() {
	stmt_t *stmt = calloc(1, sizeof(*stmt));
	if (!stmt) return NULL;
	stmt->type = STMT_EMPTY;
	return stmt;
}

stmt_t *init_var(exp_t *name, exp_t *value, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(name);
		free_exp(value);
		raise_syntax_error("failed to initialize stmt", r);
	}

	stmt->type = STMT_VAR;
	stmt->var.name = name;
	stmt->var.value = value;
	return stmt;
}

stmt_t *init_loop(exp_t *cond, stmt_t *body, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(cond);
		free_stmt(r->root, body);
		raise_syntax_error("failed to initialize stmt", r);
	}

	stmt->type = STMT_LOOP;
	stmt->loop.cond = cond;
	stmt->loop.body = body;
	return stmt;
}

stmt_t *init_ifStmt(exp_t *cond, stmt_t *thenStmt, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(cond);
		free_stmt(r->root, thenStmt);
		raise_syntax_error("failed to initialize stmt", r);
	}

	stmt->type = STMT_IF;
	stmt->ifStmt.cond = cond;
	stmt->ifStmt.thenStmt = thenStmt;
	return stmt;
}

stmt_t *init_expStmt(exp_t *exp, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(exp);
		raise_syntax_error("failed to initialize stmt", r);
	}

	stmt->type = STMT_EXPR;
	stmt->exp = exp;
	return stmt;
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
