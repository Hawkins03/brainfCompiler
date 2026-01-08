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

void free_stmt(stmt_t *root, stmt_t *stmt) { //TODO: make so you can't double free r->root
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

stmt_t *stealRoot(Reader *r) {
	stmt_t *root = r->root;

	if (root && root->next == root) {
		r->root = NULL;
		free(root);
		return NULL;
	} else 

	if (root) {
		stmt_t *first = root->next;
		free(root);
		root = first;
	}

	r->root = NULL;
	return root;
}

void set_next_stmt(Reader *r, stmt_t *stmt) {
	if (stmt) {
		r->curr_stmt->next = stmt;
		r->curr_stmt = stmt;
	}
}

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
		killReader(r);
		raise_error("failed to initialize exp", r);
	}
    exp->type = EXP_OP;
    exp->op.left = left;
    exp->op.op = op;
    exp->op.right = right;
    return exp;
}

exp_t *init_unary(exp_t *left, char *op, exp_t *right, Reader *r) {
    exp_t *exp = init_op(left, op, right, r);
	if (!exp) {
		free_exp(left);
		free(op);
		free_exp(right);
		killReader(r);
		raise_error("failed to initialize exp", r);
	}
    exp->type = EXP_UNARY;
    return exp;
}

exp_t *init_num(int num, Reader *r) {
    exp_t *exp = init_exp();
	if (!exp) {
		killReader(r);
		raise_error("failed to initialize exp", r);
	}
    exp->type = EXP_NUM;
    exp->num= num;
    return exp;
}

exp_t *init_str(char *str, Reader *r) {
	exp_t *exp = init_exp();
	if (!exp) {
		free(str);
		killReader(r);
		raise_error("failed to initialize exp", r);
	}
    exp->type = EXP_STR;
    exp->str = str;
    return exp;
}

exp_t *init_call(key_t key, exp_t *call, Reader *r) {
	exp_t *exp = init_exp();
	if (!exp) {
		free_exp(call);
		killReader(r);
		raise_error("failed to initialize exp", r);
	}
	exp->type = EXP_CALL;
	exp->call.key = key;
	exp->call.call = call;
	return exp;
}

exp_t *init_array(exp_t *name, exp_t *index, Reader *r) {
    exp_t *exp = init_exp();
	if (!exp) {
		free_exp(name);
		free_exp(index);
		killReader(r);
		raise_error("failed to initialize exp", r);
	}
    exp->type = EXP_ARRAY;
    exp->arr.arr_name = name;
    exp->arr.index = index;
    return exp;
}

exp_t *init_initList(exp_t *op, Reader *r) {
	exp_t *exp = init_exp();
	if (!exp) {
		free_exp(op);
		killReader(r);
		raise_error("failed to initialize exp", r);
	}
	exp->type = EXP_INITLIST;
	exp->initlist = op;
	return exp;
}

exp_t *init_index(exp_t *index, exp_t *next, Reader *r) {
	exp_t *exp = init_exp();
	if (!exp) {
		free_exp(index);
		free_exp(next);
		killReader(r);
		raise_error("failed to initialize exp", r);
	}
	exp->type = EXP_INDEX;
	exp->index.index = index;
	exp->index.next = next;
	return exp;
}

stmt_t *init_stmt() {
	stmt_t *stmt = calloc(1, sizeof(*stmt));
	if (!stmt) return NULL;
	stmt->type = STMT_EMPTY;
	//stmt->next = NULL; //redundant due to calloc
	return stmt;
}

stmt_t *init_var(exp_t *name, exp_t *value, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(name);
		free_exp(value);
		killReader(r);
		raise_error("failed to initialize stmt", r);
	}
	stmt->type = STMT_VAR;
	stmt->var.name = name;
	stmt->var.value = value;
	return stmt;
}

stmt_t *init_Loop(exp_t *cond, stmt_t *body, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(cond);
		free_stmt(r->root, body);
		killReader(r);
		raise_error("failed to initialize stmt", r);
	}
	stmt->type = STMT_LOOP;
	stmt->loop.cond = cond;
	stmt->loop.body = body;
	return stmt;
}

stmt_t *init_ifStmt(exp_t *cond, stmt_t *thenStmt, stmt_t *elseStmt, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(cond);
		free_stmt(r->root, thenStmt);
		free_stmt(r->root, elseStmt);
		killReader(r);
		raise_error("failed to initialize stmt", r);
	}
	stmt->type = STMT_IF;
	stmt->ifStmt.cond = cond;
	stmt->ifStmt.thenStmt = thenStmt;
	stmt->ifStmt.elseStmt = elseStmt;
	return stmt;
}

stmt_t *init_expStmt(exp_t *exp, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(exp);
		killReader(r);
		raise_error("failed to initialize stmt", r);
	}
	stmt->type = STMT_EXPR;
	stmt->exp = exp;
	return stmt;
}


exp_t *parsePrint(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "print");
    acceptToken(r, VAL_DELIM, "(");

	exp_t *exp = init_call(KW_PRINT, parse_atom(r), r);
	if (!exp) {
		raise_error("failed initialize call", r);
		return exp;
	}
	
	acceptToken(r, VAL_DELIM, ")");
	
	return exp;
}

exp_t *parseInput(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "input");
    acceptToken(r, VAL_DELIM, "(");
	
	acceptToken(r, VAL_DELIM, ")");
	
	exp_t *exp = init_call(KW_INPUT, NULL, r);
	if (!exp)
		raise_error("failed to initialize call", r);
	return exp;
}

exp_t *parseBreak(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "break");

	exp_t *exp = init_call(KW_BREAK, NULL, r);
	if (!exp)
		raise_error("failed to initialize call", r);
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
			raise_error("invalid value", r);
			return NULL;
	}
}



exp_t *parse_suffix(exp_t *left, Reader *r) {
    value_t*tok = peekToken(r);
    if (!isAlive(r) || !tok || !isSuffixOp(tok))
		return left;

    char *op = stealTokString(tok);
    freeValue(getToken(r));
    
	return init_unary(left, op, NULL, r);
}

exp_t *parse_unary(Reader *r) {
    char *op = stealTokString(peekToken(r));
    freeValue(getToken(r));

    exp_t *right = parse_atom(r);
    right = parse_suffix(right, r);
 
	exp_t *exp = init_unary(NULL, op, right, r);
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
	if (!exp)
		raise_error("failed to initialize exp", r);

	exp->type = EXP_INITLIST;
	exp->initlist = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, "}");
	return exp;
}

exp_t *parse_index(Reader *r) {
	acceptToken(r, VAL_DELIM, "[");

	exp_t *exp = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, "]");

	value_t *tok = peekToken(r);
	if (tok && (tok->type == VAL_DELIM) && (tok->ch == '['))
		return init_index(exp, parse_index(r), r);

	return init_index(exp, NULL, r);
}

exp_t *parse_atom(Reader *r) { //TODO: add more sub parse functions
    value_t *tok = peekToken(r);

    if (!tok || atSemicolon(r))
		return NULL;

    switch (tok->type) {
        case VAL_OP:
    	    return parse_unary(r);
    	case VAL_NUM:
			int num = tok->num;
			freeValue(getToken(r));
			return init_num(num, r);
		case VAL_NAME:
			char *name = stealTokString(tok);
			freeValue(getToken(r));
			exp_t *exp_name = init_str(name, r); // base name expression
	
			//check for array indexing
			value_t *nextTok = peekToken(r);
			if (nextTok && (nextTok->type == VAL_DELIM) && (nextTok->ch == '['))
				return init_array(exp_name, parse_index(r), r);
			return exp_name;
		case VAL_STR:
			char *str = stealTokString(tok);
			freeValue(getToken(r));
			exp_t *exp_str = init_initList(init_op(init_num(str[0], r), strdup(",", r), NULL, r), r);

			exp_t *currExp = exp_str->initlist;
			for (size_t i = 1; str[i+1] != '\0'; i++) {
				exp_t *right = init_op(init_num(str[i], r), strdup(",", r), NULL, r);
				currExp->op.right = right;
				currExp = currExp->op.right;
			}
			currExp->op.right = init_num(str[strlen(str) - 1], r);
			free(str);
			return exp_str;
		case VAL_DELIM:
			switch (tok->ch) {
				case '{':
					return parse_initList(r);
				case '(':
					return parse_parenthesis(r);
				case '[':
					return parse_index(r);
				default:
					raise_error("invalid character in code", r);
			}
			break;
		case VAL_KEYWORD:
			return parse_call(r);
			break;
		default:
			return NULL;
			break;
	}
	return NULL;
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

			left = init_op(left, opStr, parse_exp(prio+1, r), r);
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

	if ((name->type != EXP_STR) && (name->type != EXP_ARRAY)) {
		free_exp(name);
		raise_error("invalid name", r);
		return NULL;
	}

	acceptToken(r, VAL_OP, "=");

	exp_t *value = parse_exp(0, r);

	if ((name->type == EXP_ARRAY) && (value->type != EXP_INITLIST)) {
		free_exp(name);
		free_exp(value);
		raise_error("array's must be initialized to a list, either {0} or a larger array.", r);
		return NULL;
	}
	
	stmt_t *stmt = init_var(name, value, r);

	if (!is_mutable)
		stmt->type = STMT_VAL;
	return stmt;
}


typedef struct { stmt_t *saved_stmt; } ctx_t;

ctx_t enter_nested_context(Reader *r) {
	ctx_t ctx;
	ctx.saved_stmt = r->curr_stmt;

	stmt_t *dummy = init_stmt();
	dummy->next = dummy;
	r->curr_stmt->next = dummy;
	r->curr_stmt = dummy;
	return ctx;
}

stmt_t *exit_nested_context(Reader *r, ctx_t ctx) {
	r->curr_stmt = ctx.saved_stmt;
	stmt_t *dummy = r->curr_stmt->next;
	stmt_t *head = dummy->next;
	
	dummy->next = NULL;
	free_stmt(dummy, dummy);
	
	return head;
}

stmt_t *parseWhile(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "while");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *cond = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");

	ctx_t ctx = enter_nested_context(r);
	parse_stmt(r);
	stmt_t *body = exit_nested_context(r, ctx);

	acceptToken(r, VAL_DELIM, "}");
	
	stmt_t *stmt = init_Loop(cond, body, r);

	set_next_stmt(r, stmt);

	if (hasNextStmt(r))
		set_next_stmt(r, parse_stmt(r));
    
    return stmt;
}

stmt_t *parseFor(Reader *r) {
	/* structure of a for loop:
        "for" "("<initialization> ";" <condition> ";" <update> ")" "{" <body> "}" <next>
        
        Output structure:
        <initialization> -> <loop> -> parse_stmt(r)
							  \/
                    <body> -> <update>
    */
	
	acceptToken(r, VAL_KEYWORD, "for");
	acceptToken(r, VAL_DELIM, "(");

	stmt_t *init_stmt = init_expStmt(parse_exp(0, r), r);
	set_next_stmt(r, init_stmt);
	
	acceptToken(r, VAL_DELIM, ";");

	exp_t *cond = parse_exp(0, r);
	
	acceptToken(r, VAL_DELIM, ";");

	exp_t *update = parse_exp(0, r);


	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");
	
	ctx_t ctx = enter_nested_context(r);
	parse_stmt(r);
	stmt_t *body = exit_nested_context(r, ctx);
	stmt_t *curr = body;

	if (curr) {
		while (curr->next != NULL)
			curr = curr->next;
		curr->next = init_expStmt(update, r);
	} else
		body = init_expStmt(update, r);

	acceptToken(r, VAL_DELIM, "}");

	stmt_t *loop = init_Loop(cond, body, r);

	set_next_stmt(r, loop);

	if (hasNextStmt(r))
		set_next_stmt(r, parse_stmt(r));

	return init_stmt;
}

stmt_t *parseIf(Reader *r) {
	acceptToken(r, VAL_KEYWORD, "if");
	acceptToken(r, VAL_DELIM, "(");

	exp_t *cond = parse_exp(0, r);

	acceptToken(r, VAL_DELIM, ")");
	acceptToken(r, VAL_DELIM, "{");

	ctx_t ctx = enter_nested_context(r);
	parse_stmt(r);
	stmt_t *thenStmt = exit_nested_context(r, ctx);

	acceptToken(r, VAL_DELIM, "}");

	stmt_t *elseStmt = NULL;
	value_t* tok = peekToken(r);
	if ((tok) && (tok->type == VAL_KEYWORD) && (tok->key == KW_ELSE)) {
		acceptToken(r, VAL_KEYWORD, "else");

		tok = peekToken(r);
		if (!tok)
			raise_error("expected statement after else", r);

		if ((tok->type == VAL_KEYWORD) && (tok->key == KW_IF))
			elseStmt = parseIf(r);
		else {
			acceptToken(r, VAL_DELIM, "{");

			ctx_t ctx = enter_nested_context(r);
			parse_stmt(r);
			elseStmt = exit_nested_context(r, ctx);

			acceptToken(r, VAL_DELIM, "}");
		}
	}

	stmt_t *stmt = init_ifStmt(cond, thenStmt, elseStmt, r);

	set_next_stmt(r, stmt);

	if (hasNextStmt(r))
		set_next_stmt(r, parse_stmt(r));

	return stmt;
}

stmt_t *parse_stmt(Reader *r) {
	if (!isAlive(r))
		return NULL;
	
	value_t *tok = peekToken(r);
	if (tok == NULL)
		return NULL;
	
	stmt_t *out;
	
	if (tok->type == VAL_KEYWORD) {
		switch (tok->key) {
			case KW_VAR:
			case KW_VAL:
				out = parseVar(r, tok->key == KW_VAR);
				acceptToken(r, VAL_DELIM, ";");
				break;
			case KW_WHILE:
				out = parseWhile(r);
				break;
			case KW_FOR:
				out = parseFor(r);
				break;
			case KW_IF:
				out = parseIf(r);
				break;
			default:
				out = init_expStmt(parse_call(r), r);
				acceptToken(r, VAL_DELIM, ";");
				break;
		}
	} else {
		out = init_expStmt(parse_exp(0, r), r);
		acceptToken(r, VAL_DELIM, ";");
	}

	set_next_stmt(r, out);

	if (hasNextStmt(r)) //i.e. it's not a closing bracket or EOF
		set_next_stmt(r, parse_stmt(r));
	
	return out;
}

stmt_t *parse_file(const char *filename) {
    Reader *r = readInFile(filename);
	if (!r)
		raise_error("failed to read in file", r);

	parse_stmt(r);
	stmt_t *out = stealRoot(r);
	killReader(r);
	return out;
}
