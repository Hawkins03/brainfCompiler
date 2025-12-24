#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "interp.h"
#include "utils.h"


void free_exp(Exp *exp) {
    //printf("freeing expression\n");
    if (exp == NULL) return; //recursion end
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
	default:
	    raise_error("Unhandled expression type");
    }
    free(exp);
}

void free_stmt(Stmt *stmt) {
	if (!stmt)
		return;
	switch (stmt->type) {
		case STMT_EMPTY:
			break;
		case STMT_VAR:
		case STMT_VAL:
			if (stmt->var.name)
				free(stmt->var.name);
			free_exp(stmt->var.init);
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

	free(stmt);

	if (stmt->next == stmt)
		raise_error("recursive statement definition");
	free(stmt->next);
}

void print_full_exp(const Exp *exp) {
    print_exp(exp);
    printf("\n");
}

void print_exp(const Exp *exp) {
    //printf("printing expression:, %p\n", exp);
    if (!exp)
	return;
    switch (exp->type) {
	case EXP_STR:
	    printf("NAME(%s) ", exp->str);
	    break;
	case EXP_NUM:
	    printf("NUM(%d) ", exp->num);
	    break;
	case EXP_OP:
	    printf("OP( ");
	    print_exp(exp->op.left);
	    printf(", %s, ", exp->op.op);
	    print_exp(exp->op.right);
	    printf(") ");
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
	case EXP_EMPTY:
	    break;
	default:
	    raise_error("unhandled expression type");
    }   
}

void print_stmt(const Stmt *stmt) {
	if (!stmt) {
		printf("NULL");
		return;
	}
	switch (stmt->type) {
		case STMT_EMPTY:
			printf("EMPTY()");
			break;
		case STMT_VAR:
			printf("VAR(%s, ", stmt->var.name);
			print_exp(stmt->var.init);
			printf(")");
			break;
		case STMT_VAL:
			printf("VAL(%s, ", stmt->var.name);
			print_exp(stmt->var.init);
			printf(")");
			break;
		case STMT_LOOP:
			printf("LOOP(");
			print_exp(stmt->loop.cond);
			printf(") {\n");
			free_stmt(stmt->loop.body);
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


Exp *init_exp() {
    Exp *exp = calloc(1, sizeof(*exp));
	
    if (exp == NULL)
		raise_error("Bad malloc");

    exp->type = EXP_EMPTY;
    return exp;
}

Exp *init_op(Exp *left, char *op, Exp *right) {
    Exp *out = init_exp();
    out->type = EXP_OP;
    out->op.left = left;
    out->op.op = op;
    out->op.right = right;
    return out;
}

Exp *init_unary(Exp *left, char *op, Exp *right) {
    Exp *out = init_op(left, op, right);
    out->type = EXP_UNARY;
    return out;
}

Exp *init_num(int num) {
    Exp *out = init_exp();
    out->type = EXP_NUM;
    out->num= num;
    return out;
}

Exp *init_str(char *str) {    Exp *out = init_exp();
    out->type = EXP_STR;
    out->str = str;
    return out;
}

Stmt *init_stmt() {
	Stmt *stmt = calloc(1, sizeof(*stmt));
	if (stmt == NULL)
		raise_error("Bad mallloc");
	stmt->type = STMT_EMPTY;
	return stmt;
}

Stmt *init_var(char *name, Exp *init, Stmt *next) {
	Stmt *stmt = init_stmt();
	stmt->type = STMT_VAR;
	stmt->var.name = name;
	stmt->var.init = init;
	stmt->next = next;
	return stmt;
}

Stmt *init_Val(char *name, Exp *init, Stmt *next) {
	Stmt *stmt = init_var(name, init, next);
	stmt->type = STMT_VAL;
	return stmt;
}

Stmt *init_Loop(Exp *cond, Stmt *body, Stmt *next) {
	Stmt *stmt = init_stmt();
	stmt->type = STMT_LOOP;
	stmt->loop.cond = cond;
	stmt->loop.body = body;
	stmt->next = next;
	return stmt;
}

Stmt *init_ifStmt(Exp *cond, Stmt *thenStmt, Stmt *elseStmt, Stmt *next) {
	Stmt *stmt = init_stmt();
	stmt->type = STMT_IF;
	stmt->ifStmt.cond = cond;
	stmt->ifStmt.thenStmt = thenStmt;
	stmt->ifStmt.elseStmt = elseStmt;
	stmt->next = next;
	return stmt;
}

Stmt *init_expStmt(Exp *exp, Stmt *next) {
	Stmt *stmt = init_stmt();
	stmt->type = STMT_EXPR;
	stmt->exp = exp;
	stmt->next = next;
	return stmt;
}

Exp *parse_suffix(Exp *left, Reader *r) {
    //printf("in parseSuffix\n");
    Value *tok = peekToken(r);
    if (!isAlive(r) || !tok || !isSuffixOp(tok))
	return left;

    char *op = stealTokString(tok);
    freeValue(getToken(r));
    
    return init_unary(left, op, NULL);
}

Exp *parse_unary(Reader *r) { //TODO: ensure recursion works properly
    Value *tok = peekToken(r);
    //printf("in parse_unary:%p\n", tok);
    if (!tok || !(tok->str) || (!isUnaryOp(tok->str))) {
	//printf("tok = %p, type = %d, str = %s, %d\n", tok, tok->type == VAL_OP, tok->str, isUnaryOp(tok->str));
	return NULL;
    }
    
    char *op = stealTokString(tok);
    freeValue(getToken(r));
    Exp *right = parse_atom(r);

    right = parse_suffix(right, r);    
    return init_unary(NULL, op, right);
}

Exp *parse_char(Reader *r) {
    acceptToken(peekToken(r), VAL_DELIM, "'");
    freeValue(getToken(r));

    Value *next = peekToken(r);
    Exp *out;


    switch (next->type) { //TODO: rework
	case VAL_STR:
	case VAL_OP:
	    if ((!next->str) || (strlen(next->str) > 1))
		raise_error("expected single character");
	    
	    out = init_num(next->str[0]);
	    break;
	case VAL_NUM:
	    if (next->num >= 10)
		raise_error("expected single character");
	    
	    out = init_num(next->num + '0');
	    break;
	case VAL_DELIM:
	    if (next->type == '\'')
		return init_num(0);

	    out = init_num(next->ch);
	    break;
	default:
	    raise_error("invalid character type");
	    break;
    }
    freeValue(getToken(r)); // don't move to parse_atom

    acceptToken(peekToken(r), VAL_DELIM, "'");
    freeValue(getToken(r));

    return out;
}

Exp *parse_parenthesis(Reader *r) {
    //printf("parenthesis\n");
    acceptToken(peekToken(r), VAL_DELIM, "(");
    freeValue(getToken(r));

    Exp *out = parse_exp(0, r);
    //printf("looking for closing parenthesis:\n");

    acceptToken(peekToken(r), VAL_DELIM, ")");
    freeValue(getToken(r));
    return out;
}

Exp *parse_atom(Reader *r) {
    Value *tok = peekToken(r);
    //printf("in parse_atom\n");
    if (!tok)
	return NULL;
    //printf("continuing\n");

    Exp *exp = NULL;
    switch (tok->type) {
        case VAL_OP:
    	    exp = parse_unary(r);
	    if (!exp)
		return NULL;
	    break;
    	case VAL_NUM:
	    exp = init_num(tok->num);

	    freeValue(getToken(r));
	    break;
	case VAL_STR:
	    exp = init_str(stealTokString(tok)); // helper fn makes ownership explicit
	    freeValue(getToken(r));
	    //printf("after parse_str\n");
	    break;
	case VAL_DELIM:
	    if (tok->ch == '\'') {
	        return parse_char(r);
	    } else if (tok->ch == '(') {
	    	return parse_parenthesis(r);
	    } else {
    		return NULL;
	    }
	    break;
	case VAL_KEYWORD:
	case VAL_EMPTY:
	default:
	    return NULL;
	    break;
    }
    //check for suffix
    return exp;
}

Exp *parse_exp(int minPrio, Reader *r) {
    Exp *left = parse_atom(r);
    //printf("after parse_atom\n");
    if (!left)
	return NULL;

    while (isAlive(r)) {
	//printVal(peekToken(r));
	Value *op = peekToken(r);
	if (!op)
	    break;

	if (op->type == VAL_OP) {
	    int prio = getPrio(op->str);
	    if (((prio < minPrio) && (prio != RIGHT_ASSOC_PRIO)) || (prio == UNARY_OP_PRIO))
		break;

	    char *opStr = stealTokString(op);
	    freeValue(getToken(r));

	    left = init_op(left, opStr, parse_exp(prio+1, r));
	} else if ((op->type == VAL_DELIM) && (op->ch == ';')) {
	    freeValue(getToken(r));
	    return left;
	} else {
	    //printf("op->type = %d", op->type);
	    break;
	}
    }
    return left;
}
    

Stmt *parse_keyword(Reader *r) {
	//TODO: finish later.
}

Stmt *parse_stmt(Reader *r) {
	if (!isAlive(r)) return NULL;
	
	Value *tok = peekToken(r);
	if (tok == NULL)
		return NULL;
	
	Stmt *out = NULL;
	
	switch (tok->type) {
		case VAL_KEYWORD:
			out = parse_keyword(r);
			break;
		default: //is an exp
			out = init_expStmt(parse_exp(0, r), NULL);
			tok = peekToken(r);
			if ((!tok) || (tok->type != VAL_DELIM) || (tok->ch != ';'))
				raise_error("expected semicolon at end of statement");
			freeValue(getToken(r));
	}
	
	out->next = parse_stmt(r);
	return out;
}


Stmt *parse_file(const char *filename) {
    Reader *r = readInFile(filename);
    Stmt *out = parse_stmt(r);
    //printf("after parse_exp\n");
    killReader(r);
    //printf("after killReader\n");
    return out;
}
