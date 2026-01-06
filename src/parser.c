#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "interp.h"
#include "utils.h"


void free_exp(exp_t *exp) {
    if (exp == NULL)
		return;
    switch (exp->type) {
		case EXP_STR:
			free(exp->str);
			break;
		case EXP_NUM:
			break;
		case EXP_UNARY:
		case EXP_OP:
			free(exp->op.op);
			if (exp->op.left)
				free_exp(exp->op.left);
			if (exp->op.right)
				free_exp(exp->op.right);
			break;
		case EXP_EMPTY:
			break;
		case EXP_CALL:
			if (exp->call.call)
				free_exp(exp->call.call);
			break;
		case EXP_ARRAY:
			if (exp->arr.arr_name)
				free_exp(exp->arr.arr_name);
			if (exp->arr.index)
				free_exp(exp->arr.index);
			break;
		case EXP_INITLIST:
			if (exp->initlist)
				free_exp(exp->initlist);
			break;
		case EXP_INDEX:
			if (exp->index.index)
				free_exp(exp->index.index);
			if (exp->index.next)
				free_exp(exp->index.next);
			break;
		default:
			raise_error("Unhandled expression type");
    }
    free(exp);
}

void free_stmt(stmt_t *stmt) {
	if (!stmt)
		return;
	switch (stmt->type) {
		case STMT_EMPTY:
			break;
		case STMT_LOOP:
			free_exp(stmt->loop.cond);
			free_stmt(stmt->loop.body);
			break;
		case STMT_IF:
			free_exp(stmt->ifStmt.cond);
			free_stmt(stmt->ifStmt.thenStmt);
			free_stmt(stmt->ifStmt.elseStmt);
			break;
		case STMT_EXPR:
			free_exp(stmt->exp);
			break;
		default:
			raise_error("invalid statement type");
			break;
	}

	if (stmt->next == stmt)
		raise_error("recursive statement definition");
	free(stmt->next);

	free(stmt);
}

void print_full_exp(const exp_t *exp) {
    print_exp(exp);
    printf("\n");
}

void print_exp(const exp_t *exp) {
    if (!exp)
		return;
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
			if (!exp->op.left)
				printf("NULL");
			else
			print_exp(exp->op.left);
			printf(", %s, \n", exp->op.op);
			if (!exp->op.right)
				printf("NULL");
			else
				print_exp(exp->op.right);
			printf(")");
			break;
		case EXP_CALL:
			printf("%s(", getKeyStr(exp->call.key));
			print_exp(exp->call.call);
			printf(")");
			break;
		case EXP_ARRAY:
			printf("ARR(");
			print_exp(exp->arr.arr_name);
			printf(", ");
			print_exp(exp->arr.index);
			printf(")");
			break;
		case EXP_INDEX:
			printf("INDEX(");
			print_exp(exp->index.index);
			printf(", ");
			print_exp(exp->index.next);
			printf(")");
			break;
		case EXP_INITLIST:
			printf("INITLIST(");
			print_exp(exp->initlist);
			printf(")");
			break;
		case EXP_EMPTY:
			break;
		default:
			raise_error("unhandled expression type");
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
			printf("}");
			break;
		case STMT_EXPR:
			printf("EXP_");
			print_exp(stmt->exp);
			break;
		default:
			raise_error("invalid statement type");
			break;
	}
	printf(";\n");

	if (stmt->next == stmt)
		raise_error("recursive statement definition");
	print_stmt(stmt->next);
}

exp_t *init_exp() {
    exp_t *exp = calloc(1, sizeof(*exp));
	
    if (exp == NULL)
		raise_error("Bad malloc");

    exp->type = EXP_EMPTY;
    return exp;
}

exp_t *init_op(exp_t *left, char *op, exp_t *right) {
    exp_t *out = init_exp();
    out->type = EXP_OP;
    out->op.left = left;
    out->op.op = op;
    out->op.right = right;
    return out;
}

exp_t *init_unary(exp_t *left, char *op, exp_t *right) {
    exp_t *out = init_op(left, op, right);
    out->type = EXP_UNARY;
    return out;
}

exp_t *init_num(int num) {
    exp_t *out = init_exp();
    out->type = EXP_NUM;
    out->num= num;
    return out;
}

exp_t *init_str(char *str) {
	exp_t *out = init_exp();
    out->type = EXP_STR;
    out->str = str;
    return out;
}

exp_t *init_call(key_t key, exp_t *call) {
	exp_t *exp = init_exp();
	exp->type = EXP_CALL;
	exp->call.key = key;
	exp->call.call = call;
	return exp;
}

exp_t *init_array(exp_t *name, exp_t *index) {
    exp_t *exp = init_exp();
    exp->type = EXP_ARRAY;
    exp->arr.arr_name = name;
    exp->arr.index = index;
    return exp;
}

exp_t *init_initList(exp_t *op) {
	exp_t *exp = init_exp();
	exp->type = EXP_INITLIST;
	exp->initlist = op;
	return exp;
}

exp_t *init_index(exp_t *index, exp_t *next) {
	exp_t *exp = init_exp();
	exp->type = EXP_INDEX;
	exp->index.index = index;
	exp->index.next = next;
	return exp;
}

stmt_t *init_stmt() {
	stmt_t *stmt = calloc(1, sizeof(*stmt));
	if (stmt == NULL)
		raise_error("Bad mallloc");
	stmt->type = STMT_EMPTY;
	return stmt;
}

stmt_t *init_Loop(exp_t *cond, stmt_t *body, stmt_t *next) {
	stmt_t *stmt = init_stmt();
	stmt->type = STMT_LOOP;
	stmt->loop.cond = cond;
	stmt->loop.body = body;
	stmt->next = next;
	return stmt;
}

stmt_t *init_ifStmt(exp_t *cond, stmt_t *thenStmt, stmt_t *elseStmt, stmt_t *next) {
	stmt_t *stmt = init_stmt();
	stmt->type = STMT_IF;
	stmt->ifStmt.cond = cond;
	stmt->ifStmt.thenStmt = thenStmt;
	stmt->ifStmt.elseStmt = elseStmt;
	stmt->next = next;
	return stmt;
}

stmt_t *init_expStmt(exp_t *exp, stmt_t *next) {
	stmt_t *stmt = init_stmt();
	stmt->type = STMT_EXPR;
	stmt->exp = exp;
	stmt->next = next;
	return stmt;
}


exp_t *parsePrint(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "print");
    acceptToken(r, VAL_DELIM, "(");

	exp_t *exp = init_call(KW_PRINT, parse_atom(r));
	
	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, ";");
	
	return exp;
}

exp_t *parseInput(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "input");
    acceptToken(r, VAL_DELIM, "(");
	
	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, ";");
	
	return init_call(KW_INPUT, NULL);;
}

exp_t *parseBreak(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "break");
    acceptToken(r, VAL_DELIM, ";");
    return init_call(KW_BREAK, NULL);
}

exp_t *parse_call(Reader *r) {
	value_t*tok = peekToken(r);
	if ((!tok) || (tok->type != VAL_KEYWORD))
		return NULL;
	
	switch (tok->key) {
		case KW_PRINT: //PRINT
			return parsePrint(r);
		case KW_INPUT: //INPUT
			return parseInput(r);
		case KW_BREAK: //BREAK
			return parseBreak(r);
		default:
			raise_error("invalid value");
			return NULL;
	}
}



exp_t *parse_suffix(exp_t *left, Reader *r) {
    value_t*tok = peekToken(r);
    if (!isAlive(r) || !tok || !isSuffixOp(tok))
		return left;

    char *op = stealTokString(tok);
    freeValue(getToken(r));
    
    return init_unary(left, op, NULL);
}

exp_t *parse_unary(Reader *r) {
    value_t*tok = peekToken(r);
    if (!tok || !(tok->str) || (!isUnaryOp(tok->str)))
		return NULL;
    
    char *op = stealTokString(tok);
    freeValue(getToken(r));
    exp_t *right = parse_atom(r);
    right = parse_suffix(right, r);
 
    return init_unary(NULL, op, right);
}

exp_t *parse_parenthesis(Reader *r) {
    acceptToken(r, VAL_DELIM, "(");

    exp_t *out = parse_exp(0, r);

    acceptToken(r, VAL_DELIM, ")");
    return out;
}

exp_t *parse_initList(Reader *r) {
	acceptToken(r, VAL_DELIM, "{");

	exp_t *out = init_exp();
	out->type = EXP_INITLIST;
	out->initlist = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, "}");
	return out;
}

exp_t *parse_index(Reader *r) {
	acceptToken(r, VAL_DELIM, "[");
	exp_t *out = parse_exp(0, r);
	acceptToken(r, VAL_DELIM, "]");
	value_t *tok = peekToken(r);

	if (tok && (tok->type == VAL_DELIM) && (tok->ch == '['))
		out = init_index(out, parse_index(r));
	else
		out = init_index(out, NULL);
	return out;
}

exp_t *parse_atom(Reader *r) {
    value_t *tok = peekToken(r);

    if (!tok)
		return NULL;

    exp_t *exp = NULL;
    switch (tok->type) {
        case VAL_OP:
    	    exp = parse_unary(r);
			break;
    	case VAL_NUM:
			exp = init_num(tok->num);

			freeValue(getToken(r));
			break;
		case VAL_NAME:
			char *name = stealTokString(tok);
			freeValue(getToken(r));
			exp = init_str(name); // base name expression

			//check for array indexing
			value_t *nextTok = peekToken(r);
			if (nextTok && (nextTok->type == VAL_DELIM) && (nextTok->ch == '['))
				exp = init_array(exp, parse_index(r));
			break;
		case VAL_STR:
			char *str = stealTokString(tok);
			freeValue(getToken(r));
			char *comma = strdup(",");
			exp = init_initList(init_op(init_num(str[0]), comma, NULL));
			exp_t *currExp = exp->initlist;
			for (size_t i = 1; i < strlen(str) - 1; i++) {
				comma = strdup(",");
				currExp->op.right = init_op(init_num(str[i]), comma, NULL);
				currExp = currExp->op.right;
			}
			currExp->op.right = init_num(str[strlen(str) - 1]);
			free(str);
			break;
		case VAL_DELIM:
			switch (tok->ch) {
				case '{':
					exp = parse_initList(r);
					break;
				case '(':
					exp = parse_parenthesis(r);
					break;
				case '[':
					exp = parse_index(r);
					break;
				default:
					raise_error("invalid character in code");
					break;
			}
			break;
		case VAL_KEYWORD:
			exp = parse_call(r);
			break;
		case VAL_EMPTY:
		default:
			return NULL;
			break;
	}
    return exp;
}

exp_t *parse_exp(int minPrio, Reader *r) {
    exp_t *left = parse_atom(r);
    if (!left)
		return NULL;

    while (isAlive(r)) {
		value_t*op = peekToken(r);
		if (!op)
			break;

		if (op->type == VAL_OP) {
			int prio = getPrio(op->str); //TODO: add helper function to shorten if
			if (((prio < minPrio) && (prio != RIGHT_ASSOC_PRIO)) || (prio == UNARY_OP_PRIO))
				break;

			char *opStr = stealTokString(op);
			freeValue(getToken(r));

			left = init_op(left, opStr, parse_exp(prio+1, r));
		}
		else
			break;
    }
    return left;
}
   
stmt_t *parseVar(Reader *r, bool is_mutable) {
	key_t key = is_mutable ? KW_VAR : KW_VAL;
	acceptToken(r, VAL_KEYWORD, getKeyStr(key));

	value_t*tok = peekToken(r);
	if ((!tok) || (tok->type != VAL_NAME) || (!tok->str))
		raise_error("invalid variable name");
	
	exp_t *exp = init_call(key, parse_exp(0, r)); //= is an op, so this should be OP(NAME(...), =, OP(...))
	
	acceptToken(r, VAL_DELIM, ";");
	return init_expStmt(exp, parse_stmt(r));
}

stmt_t *parseWhile(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "while");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *cond = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");

	stmt_t *body = parse_stmt(r);

	acceptToken(r, VAL_DELIM, "}");
	
	return init_Loop(cond, body, parse_stmt(r));
}

stmt_t *parseFor(Reader *r) { //TODO
	/* structure of a for loop:
        "for" "("<initialization> ";" <condition> ";" <update> ")" "{" <body> "}" <next>
        
        Output structure:
        <initialization> -> <loop> -> parse_stmt(r)
							  \/
                    <body> -> <update>
    */
	
	acceptToken(r, VAL_KEYWORD, "for");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *init = parse_exp(0, r);
	
	acceptToken(r, VAL_DELIM, ";");

	exp_t *cond = parse_exp(0, r);
	
	acceptToken(r, VAL_DELIM, ";");

	exp_t *update = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");
	
	stmt_t *body = parse_stmt(r);
	
	acceptToken(r, VAL_DELIM, "}");
	
	stmt_t *curr = body;
	if (curr) {
		while (curr->next != NULL)
			curr = curr->next;
		curr->next = init_expStmt(update, NULL);
	} else
		body = init_expStmt(update, NULL);
	return init_expStmt(init, init_Loop(cond, body, parse_stmt(r)));
}

extern stmt_t *parseElse(Reader *r);

stmt_t *parseIf(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "if");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *cond = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");

	stmt_t *then = parse_stmt(r);

	acceptToken(r, VAL_DELIM, "}");

	value_t*tok = peekToken(r);
	stmt_t *elseStmt = NULL;
	if ((tok) && (tok->type == VAL_KEYWORD) && (tok->key == KW_ELSE))
		elseStmt = parseElse(r);
	
	return init_ifStmt(cond, then, elseStmt, parse_stmt(r));
}

stmt_t *parseElse(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "else");

	value_t *tok = peekToken(r);
	if (!tok)
		raise_error("expected statement after else");


	if ((tok->type == VAL_KEYWORD) && (tok->key == KW_IF))
		return parseIf(r);
	else {
		acceptToken(r, VAL_DELIM, "{");

		stmt_t *then = parse_stmt(r);

		acceptToken(r, VAL_DELIM, "}");
		return then;
	}
}

stmt_t *parse_keyword(Reader *r) {
	value_t *tok = peekToken(r);
	if (!tok)
		return NULL;
	if (tok->type != VAL_KEYWORD)
		return NULL;
	
	switch (tok->key) {
		case KW_VAR:
		case KW_VAL:
			return parseVar(r, tok->key == KW_VAR);
		case KW_WHILE:
			return parseWhile(r);
		case KW_FOR:
			return parseFor(r);
		case KW_IF:
			return parseIf(r);
		default:
			exp_t *expcall = parse_call(r);
			if (!expcall)
				raise_error("Invalid Keyword");
			else
				return init_expStmt(expcall, parse_stmt(r));
	}
	return NULL;
}

stmt_t *parse_stmt(Reader *r) {
	if (!isAlive(r)) return NULL;
	
	value_t *tok = peekToken(r);
	if (tok == NULL)
		return NULL;
	
	stmt_t *out = NULL;
	
	if (tok->type == VAL_KEYWORD)
		out = parse_keyword(r);
	else { //anything else can safely be assumed to be an expression.
		out = init_expStmt(parse_exp(0, r), NULL);
		tok = peekToken(r);
		if (tok)
			acceptToken(r, VAL_DELIM, ";");
	}

	if (!hasNextStmt(r)) //i.e. it's a closing bracket or EOF
		return out;

	out->next = parse_stmt(r);
	return out;
}


stmt_t *parse_file(const char *filename) {
    Reader *r = readInFile(filename);
    stmt_t *out = parse_stmt(r);
    killReader(r);
    return out;
}
