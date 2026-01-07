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
			if (exp->op.left) //TODO: check if needed
				free_exp(exp->op.left);
			exp->op.left = NULL;
			if (exp->op.right)
				free_exp(exp->op.right);
			exp->op.right = NULL;
			break;
		case EXP_EMPTY:
			break;
		case EXP_CALL:
			if (exp->call.call)
				free_exp(exp->call.call);
			exp->call.call = NULL;
			break;
		case EXP_ARRAY:
			if (exp->arr.arr_name)
				free_exp(exp->arr.arr_name);
			exp->arr.arr_name = NULL;
			if (exp->arr.index)
				free_exp(exp->arr.index);
			exp->arr.index = NULL;
			break;
		case EXP_INITLIST:
			if (exp->initlist)
				free_exp(exp->initlist);
			exp->initlist = NULL;
			break;
		case EXP_INDEX:
			if (exp->index.index)
				free_exp(exp->index.index);
			exp->index.index = NULL;
			if (exp->index.next)
				free_exp(exp->index.next);
			exp->index.next = NULL;
			break;
    }
    free(exp);
}

void free_stmt(stmt_t *stmt) {
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
			free_stmt(stmt->loop.body);
			stmt->loop.body = NULL;
			break;
		case STMT_IF:
			free_exp(stmt->ifStmt.cond);
			stmt->ifStmt.cond = NULL;
			free_stmt(stmt->ifStmt.thenStmt);
			stmt->ifStmt.thenStmt = NULL;
			free_stmt(stmt->ifStmt.elseStmt);
			stmt->ifStmt.elseStmt = NULL;
			break;
		case STMT_EXPR:
			free_exp(stmt->exp);
			stmt->exp = NULL;
			break;
	}

	if (stmt->next == stmt)
		return;

	free_stmt(stmt->next);
	stmt->next = NULL;
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
		case STMT_VAR:
			printf("VAR(");
			print_exp(stmt->var.name);
			printf(", ");
			print_exp(stmt->var.value);
			printf(")");
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
	}
	printf(";\n");

	if (stmt->next == stmt)
		return;
	print_stmt(stmt->next);
}

exp_t *init_exp() {
    exp_t *exp = calloc(1, sizeof(*exp));
	
    if (exp == NULL)
		return NULL;

    exp->type = EXP_EMPTY;
    return exp;
}

exp_t *init_op(exp_t *left, char *op, exp_t *right) {
    exp_t *exp = init_exp();
	if (!exp) return NULL;
    exp->type = EXP_OP;
    exp->op.left = left;
    exp->op.op = op;
    exp->op.right = right;
    return exp;
}

exp_t *init_unary(exp_t *left, char *op, exp_t *right) {
    exp_t *exp = init_op(left, op, right);
	if (!exp) return NULL;
    exp->type = EXP_UNARY;
    return exp;
}

exp_t *init_num(int num) {
    exp_t *exp = init_exp();
	if (!exp) return NULL;
    exp->type = EXP_NUM;
    exp->num= num;
    return exp;
}

exp_t *init_str(char *str) {
	exp_t *exp = init_exp();
	if (!exp) return NULL;
    exp->type = EXP_STR;
    exp->str = str;
    return exp;
}

exp_t *init_call(key_t key, exp_t *call) {
	exp_t *exp = init_exp();
	if (!exp) return NULL;
	exp->type = EXP_CALL;
	exp->call.key = key;
	exp->call.call = call;
	return exp;
}

exp_t *init_array(exp_t *name, exp_t *index) {
    exp_t *exp = init_exp();
	if (!exp) return NULL;
    exp->type = EXP_ARRAY;
    exp->arr.arr_name = name;
    exp->arr.index = index;
    return exp;
}

exp_t *init_initList(exp_t *op) {
	exp_t *exp = init_exp();
	if (!exp) return NULL;
	exp->type = EXP_INITLIST;
	exp->initlist = op;
	return exp;
}

exp_t *init_index(exp_t *index, exp_t *next) {
	exp_t *exp = init_exp();
	if (!exp) return NULL;
	exp->type = EXP_INDEX;
	exp->index.index = index;
	exp->index.next = next;
	return exp;
}

stmt_t *init_stmt() {
	stmt_t *stmt = calloc(1, sizeof(*stmt));
	if (!stmt) return NULL;
	stmt->type = STMT_EMPTY;
	return stmt;
}

stmt_t *init_var(exp_t *name, exp_t *value) {
	stmt_t *stmt = init_stmt();
	if (!stmt) return NULL;
	stmt->type = STMT_VAR;
	stmt->var.name = name;
	stmt->var.value = value;
	return stmt;
}

stmt_t *init_Loop(exp_t *cond, stmt_t *body, stmt_t *next) {
	stmt_t *stmt = init_stmt();
	if (!stmt) return NULL;
	stmt->type = STMT_LOOP;
	stmt->loop.cond = cond;
	stmt->loop.body = body;
	stmt->next = next;
	return stmt;
}

stmt_t *init_ifStmt(exp_t *cond, stmt_t *thenStmt, stmt_t *elseStmt, stmt_t *next) {
	stmt_t *stmt = init_stmt();
	if (!stmt) return NULL;
	stmt->type = STMT_IF;
	stmt->ifStmt.cond = cond;
	stmt->ifStmt.thenStmt = thenStmt;
	stmt->ifStmt.elseStmt = elseStmt;
	stmt->next = next;
	return stmt;
}

stmt_t *init_expStmt(exp_t *exp, stmt_t *next) {
	stmt_t *stmt = init_stmt();
	if (!stmt) return NULL;
	stmt->type = STMT_EXPR;
	stmt->exp = exp;
	stmt->next = next;
	return stmt;
}


exp_t *parsePrint(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "print");
    acceptToken(r, VAL_DELIM, "(");

	exp_t *exp = init_call(KW_PRINT, parse_atom(r));
	if (!exp) {
		raise_error("failed initialize call", ERR_OOM, r);
		return exp;
	}
	
	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, ";");
	
	return exp;
}

exp_t *parseInput(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "input");
    acceptToken(r, VAL_DELIM, "(");
	
	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, ";");
	
	exp_t *exp = init_call(KW_INPUT, NULL);
	if (!exp)
		raise_error("failed to initialize call", ERR_OOM, r);
	return exp;
}

exp_t *parseBreak(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "break");
    acceptToken(r, VAL_DELIM, ";");

	exp_t *exp = init_call(KW_BREAK, NULL);
	if (!exp)
		raise_error("failed to initialize call", ERR_OOM, r);
    return exp;
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
			raise_error("invalid value", ERR_SYNTAX, r);
			return NULL;
	}
}



exp_t *parse_suffix(exp_t *left, Reader *r) {
    value_t*tok = peekToken(r);
    if (!isAlive(r) || !tok || !isSuffixOp(tok))
		return left;

    char *op = stealTokString(tok);
    freeValue(getToken(r));
    
	exp_t *exp = init_unary(left, op, NULL);
	if (!exp) {
		//free(op);
		free_exp(left);
		raise_error("failed to initialize suffix", ERR_OOM, r);
	}
    return exp;
}

exp_t *parse_unary(Reader *r) {
    value_t*tok = peekToken(r);
    if (!tok || !(tok->str) || (!isUnaryOp(tok->str)))
		return NULL;
    
    char *op = stealTokString(tok);
    freeValue(getToken(r));
    exp_t *right = parse_atom(r);
    right = parse_suffix(right, r);
 
	exp_t *exp = init_unary(NULL, op, right);
	if (!exp) {
		//free(op);
		free_exp(right);
		raise_error("failed to initialize unary", ERR_OOM, r);
	}
    return exp;
}

exp_t *parse_parenthesis(Reader *r) {
    acceptToken(r, VAL_DELIM, "(");

    exp_t *exp = parse_exp(0, r);

    acceptToken(r, VAL_DELIM, ")");
    return exp;
}

exp_t *parse_initList(Reader *r) {
	acceptToken(r, VAL_DELIM, "{");

	exp_t *exp = init_exp();
	if (!exp) {
		raise_error("failed to initialize exp", ERR_OOM, r);
		return NULL;
	}
	exp->type = EXP_INITLIST;
	exp->initlist = parse_exp(0, r);
	if (!exp->initlist)
		return exp;

	acceptToken(r, VAL_DELIM, "}");
	return exp;
}

exp_t *parse_index(Reader *r) {
	acceptToken(r, VAL_DELIM, "[");
	exp_t *exp = parse_exp(0, r);
	if (!exp)
		return exp;
	acceptToken(r, VAL_DELIM, "]");
	value_t *tok = peekToken(r);
	exp_t *temp;
	if (tok && (tok->type == VAL_DELIM) && (tok->ch == '['))
		temp = init_index(exp, parse_index(r));
	else
		temp = init_index(exp, NULL);
	if (!temp) {
		free_exp(exp);
		raise_error("failed to initialize index", ERR_OOM, r);
	}
	exp = temp;
	return exp;
}

exp_t *parse_atom(Reader *r) { //TODO: add more sub parse functions
    value_t *tok = peekToken(r);

    if (!tok || atSemicolon(r))
		return NULL;

    exp_t *exp = NULL;
    switch (tok->type) {
        case VAL_OP:
    	    exp = parse_unary(r);
			break;
    	case VAL_NUM:
			exp = init_num(tok->num);
			if (!exp) {
				raise_error("failed to initialize num", ERR_OOM, r);
				return exp;
			}
			freeValue(getToken(r));
			break;
		case VAL_NAME:
			char *name = stealTokString(tok);
			freeValue(getToken(r));
			exp = init_str(name); // base name expression
			if (!exp) {
				free(name);
				raise_error("failed to initialize string", ERR_OOM, r);
				return exp;
			}
			//check for array indexing
			value_t *nextTok = peekToken(r);
			if (nextTok && (nextTok->type == VAL_DELIM) && (nextTok->ch == '[')) {
				exp_t *temp = init_array(exp, parse_index(r));
				if (!temp) {
					free(exp);
					raise_error("failed to initialize string", ERR_OOM, r);
					return temp;
				}
				exp = temp;
			}
			break;
		case VAL_STR:
			char *str = stealTokString(tok);
			freeValue(getToken(r));
			char *comma = strdup(",");
			exp = init_initList(init_op(init_num(str[0]), comma, NULL));
			if (!exp) {
				free(str);
				free_exp(exp);
				raise_error("failed to initialize array", ERR_OOM, r);
				return exp;
			} else if (!exp->initlist) {
				free(str);
				free_exp(exp);
				raise_error("failed to initialize op", ERR_OOM, r);
				return exp;
			} else if (!exp->initlist->op.left) {
				free(str);
				free_exp(exp);
				raise_error("failed to initialize num", ERR_OOM, r);
				return exp;
			}
			exp_t *currExp = exp->initlist;
			for (size_t i = 1; str[i+1] != '\0'; i++) {
				comma = strdup(",");
				if (!comma) {
					raise_error("failed to initialize string", ERR_OOM, r);
					return exp;
				}
				exp_t *right = init_op(init_num(str[i]), comma, NULL);
				if (!right) {
					free_exp(currExp->op.right);
					currExp->op.right = NULL;
					raise_error("failed to initialize op", ERR_OOM, r);
					return exp;
				} else if (!right->op.left) {
					free_exp(currExp->op.right->op.left);
					currExp->op.right = NULL;
					raise_error("failed to initialize num", ERR_OOM, r);
					return exp;
				}

				currExp->op.right = right;
				currExp = currExp->op.right;
			}
			currExp->op.right = init_num(str[strlen(str) - 1]);
			if (!currExp->op.right) {
				free_exp(currExp->op.right);
				currExp->op.right = NULL;
				raise_error("failed to initialize num", ERR_OOM, r);
				return exp;
			}
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
					raise_error("invalid character in code", ERR_SYNTAX, r);
					break;
			}
			break;
		case VAL_KEYWORD:
			exp = parse_call(r);
			break;
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

    while ((isAlive(r) && hasNextStmt(r)) && !atSemicolon(r)) {
		value_t*op = peekToken(r);
		if (!op)
			break;

		if (op->type == VAL_OP) {
			int prio = getPrio(op->str);
			if (((prio < minPrio) && (prio != RIGHT_ASSOC_PRIO)) || (prio == UNARY_OP_PRIO))
				break;

			char *opStr = stealTokString(op);
			freeValue(getToken(r));

			exp_t *temp = init_op(left, opStr, parse_exp(prio+1, r));
			if (!temp) {
				free_exp(temp);
				raise_error("failed to initialize op", ERR_OOM, r);
				return NULL;
			}
			left = temp;
		}
		else
			break;
    }
    return left;
}
   
stmt_t *parseVar(Reader *r, bool is_mutable) {
	key_t key = is_mutable ? KW_VAR : KW_VAL;
	acceptToken(r, VAL_KEYWORD, getKeyStr(key));

	exp_t *name = parse_atom(r);
	if (!name || !isAlive(r)) {
		free_exp(name);
		return NULL;
	} else if ((name->type != EXP_STR) && (name->type != EXP_ARRAY)) {
		free_exp(name);
		raise_error("invalid name", ERR_SYNTAX, r);
		return NULL;
	}

	acceptToken(r, VAL_OP, "=");

	exp_t *value = parse_exp(0, r);

	if (!value || !isAlive(r)) {
		free_exp(name);
		free_exp(value);
		return NULL;
	} else  if ((name->type == EXP_ARRAY) && (value->type != EXP_INITLIST)) {
		free_exp(name);
		free_exp(value);
		raise_error("array's must be initialized to a list, either {0} or a larger array.", ERR_SYNTAX, r);
		return NULL;
	}
	
	stmt_t *stmt = init_var(name, value);

	if (!stmt) {
		free_exp(name);
		free_exp(value);
		raise_error("failed to initialize num", ERR_OOM, r);
		return NULL;
	}

	if (!is_mutable)
		stmt->type = STMT_VAL;
	return stmt;
}

stmt_t *parseWhile(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "while");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *cond = parse_exp(0, r);
	if ((!cond) || (!isAlive(r))) {
		free_exp(cond);
		return NULL;
	}

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");

	stmt_t *body = parse_stmt(r);

	if ((!body) || (!isAlive(r))) {
		free_exp(cond);
		free_stmt(body);
		return NULL;
	}

	acceptToken(r, VAL_DELIM, "}");
	
	stmt_t *stmt = init_Loop(cond, body, parse_stmt(r));

	if (!stmt) {
		free_exp(cond);
		free_stmt(body);
		raise_error("failed to initialize loop", ERR_OOM, r);
		return NULL;
	}
	return stmt;
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
	if (!init || !isAlive(r)) {
		free_exp(init);
		return NULL;
	}
	
	acceptToken(r, VAL_DELIM, ";");

	exp_t *cond = parse_exp(0, r);
	if (!cond || !isAlive(r)) {
		free_exp(init);
		free_exp(cond);
		return NULL;
	}
	
	acceptToken(r, VAL_DELIM, ";");

	exp_t *update = parse_exp(0, r);

	if (!update || !isAlive(r)) {
		free_exp(init);
		free_exp(cond);
		free_exp(update);
		return NULL;
	}

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");
	
	stmt_t *body = parse_stmt(r);
	if (!body || !isAlive(r)) {
		free_exp(init);
		free_exp(cond);
		free_exp(update);
		free_stmt(body);
		return NULL;
	}
	
	acceptToken(r, VAL_DELIM, "}");
	
	stmt_t *curr = body;
	if (curr) {
		while (curr->next != NULL)
			curr = curr->next;
		curr->next = init_expStmt(update, NULL);
		if (!curr->next) {
			free_exp(init);
			free_exp(cond);
			free_exp(update);
			free_stmt(body);
			raise_error("failed to initialize exp", ERR_OOM, r);
			return NULL;
		}
	} else
		body = init_expStmt(update, NULL);

	if (!body) {
		free_exp(init);
		free_exp(cond);
		free_exp(update);
		free_stmt(body);
		raise_error("failed to initialize exp", ERR_OOM, r);
		return NULL;
	}
	stmt_t *loop = init_Loop(cond, body, parse_stmt(r));
	if (!loop) {
		free_exp(init);
		free_exp(cond);
		free_exp(update);
		free_stmt(body);
		raise_error("failed to initialize loop", ERR_OOM, r);
		return NULL;
	}

	stmt_t *stmt = init_expStmt(init, loop);

	if (!stmt) {
		free_exp(init);
		free_exp(cond);
		free_exp(update);
		free_stmt(body);
		free_stmt(loop);
		raise_error("failed to initialize expStmt", ERR_OOM, r);
		return NULL;
	}

	return stmt;
}

extern stmt_t *parseElse(Reader *r);

stmt_t *parseIf(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "if");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *cond = parse_exp(0, r);

	if (!cond || !isAlive(r)) {
		free_exp(cond);
		return NULL;
	}

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");

	stmt_t *then = parse_stmt(r);

	if (!then || !isAlive(r)) {
		free_exp(cond);
		free_stmt(then);
		return NULL;
	}

	acceptToken(r, VAL_DELIM, "}");

	value_t* tok = peekToken(r);
	stmt_t *elseStmt = NULL;
	if ((tok) && (tok->type == VAL_KEYWORD) && (tok->key == KW_ELSE)) {
		elseStmt = parseElse(r);
		if (!elseStmt || !isAlive(r)) {
			free_exp(cond);
			free_stmt(then);
			free_stmt(elseStmt);
			return NULL;
		}
	}
	stmt_t *next = parse_stmt(r);
	if (!next || !isAlive(r)) {
		free_exp(cond);
		free_stmt(then);
		free_stmt(elseStmt);
		free_stmt(next);
		return NULL;
	}
	stmt_t *ifStmt = init_ifStmt(cond, then, elseStmt, next);

	if (!ifStmt || !isAlive(r)) {
		free_exp(cond);
		free_stmt(then);
		free_stmt(elseStmt);
		free_stmt(next);
		free_stmt(ifStmt);
		return NULL;
	}

	return ifStmt;
}

stmt_t *parseElse(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "else");

	value_t *tok = peekToken(r);
	if (!tok)
		raise_error("expected statement after else", ERR_SYNTAX, r);


	if ((tok->type == VAL_KEYWORD) && (tok->key == KW_IF))
		return parseIf(r);
	else {
		acceptToken(r, VAL_DELIM, "{");

		stmt_t *then = parse_stmt(r);
		if (!then || !isAlive(r)) {
			free_stmt(then);
			return NULL;
		}

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
				raise_error("Invalid Keyword", ERR_SYNTAX, r);
			stmt_t *next = parse_stmt(r);
			if (!next || !isAlive(r)) {
				free_exp(expcall);
				free_stmt(next);
				return NULL;
			}
			stmt_t *expStmt = init_expStmt(expcall, parse_stmt(r));
			if (!expStmt || !isAlive(r)) {
				free_exp(expcall);
				free_stmt(next);
				free_stmt(expStmt);
				return NULL;
			}
				
			return expStmt;
	}
}

stmt_t *parse_stmt(Reader *r) {
	if (!isAlive(r)) return NULL;
	
	value_t *tok = peekToken(r);
	if (tok == NULL)
		return NULL;
	
	stmt_t *out = NULL;
	
	if (tok->type == VAL_KEYWORD)
		out = parse_keyword(r);
	else { //anything else can safely be assumed to be an expression
		exp_t *exp = parse_exp(0, r);
		if (!exp || !isAlive(r)) {
			free_exp(exp);
			return NULL;
		}
			
		out = init_expStmt(exp, NULL);
		if (!out) {
			free_exp(exp);
			raise_error("failed to initialize expStmt", ERR_OOM, r);
			return NULL;
		}
			
	}

	if (peekToken(r) && atSemicolon(r))
		acceptToken(r, VAL_DELIM, ";");

	if (!hasNextStmt(r)) //i.e. it's a closing bracket or EOF
		return out;

	out->next = parse_stmt(r);
	return out;
}

stmt_t *parse_file(const char *filename) {
    Reader *r = readInFile(filename);
	if (!r)
		raise_error("failed to read in file", ERR_FILE, r);
	stmt_t *out = parse_stmt(r);
	killReader(r);
	return out;
}
