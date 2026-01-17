#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stmt.h"
#include "exp.h"
#include "reader.h"

// freeing functions
void free_stmt(struct stmt *stmt) {
	if (!stmt)
		return;
	switch (stmt->type) {
	case STMT_EMPTY:
		break;
	case STMT_VAR:
		if (!stmt->var)
			return;
		free_exp(stmt->var->name);
		stmt->var->name = NULL;
		free_exp(stmt->var->value);
		stmt->var->value = NULL;
		free(stmt->var);
		stmt->var = NULL;
		break;
	case STMT_LOOP:
		if (!stmt->loop)
			return;
		free_exp(stmt->loop->cond);
		stmt->loop->cond = NULL;
		free_stmt(stmt->loop->body);
		stmt->loop->body = NULL;
		free(stmt->loop);
		stmt->loop = NULL;
		break;
	case STMT_IF:
		if (!stmt->ifStmt)
			return;
		free_exp(stmt->ifStmt->cond);
		stmt->ifStmt->cond = NULL;
		free_stmt(stmt->ifStmt->thenStmt);
		stmt->ifStmt->thenStmt = NULL;
		free_stmt(stmt->ifStmt->elseStmt);
		stmt->ifStmt->elseStmt = NULL;
		free(stmt->ifStmt);
		stmt->ifStmt = NULL;
		break;
	case STMT_EXPR:
		free_exp(stmt->exp);
		stmt->exp = NULL;
		break;
	}

	if (stmt->next != stmt) {
		free_stmt(stmt->next);
		stmt->next = NULL;
	}
	free(stmt);
}

//print functions
void print_stmt(const struct stmt *stmt) {
	if (!stmt) {
		printf("NULL\n");
		return;
	}
	switch (stmt->type) {
	case STMT_EMPTY:
		printf("EMPTY();\n");
		break;
	case STMT_VAR:
		char *name = (stmt->var->is_mutable) ? "VAR" : "VAL";
		printf("%s(", name);
		print_exp(stmt->var->name);
		printf(", ");
		print_exp(stmt->var->value);
		printf(");\n");
		break;
		break;
	case STMT_LOOP:
		printf("LOOP(");
		print_exp(stmt->loop->cond);
		printf(") {\n");
		print_stmt(stmt->loop->body);
		printf("}\n");
		break;
	case STMT_IF:
		printf("IF(");
		print_exp(stmt->ifStmt->cond);
		printf(") {\n");
		print_stmt(stmt->ifStmt->thenStmt);
		printf("} else {\n");
		print_stmt(stmt->ifStmt->elseStmt);
		printf("}\n");
		break;
	case STMT_EXPR:
		printf("EXP_");
		print_exp(stmt->exp);
		printf(";\n");
		break;
	}

	if (stmt->next != stmt)
		print_stmt(stmt->next);
}


struct stmt *init_stmt(struct reader *r) {
	struct stmt *s = calloc(1, sizeof(*s));
	if (!s)
		raise_syntax_error(ERR_NO_MEM, r);
	
	s->type = STMT_EMPTY;
	s->filename = r->filename;
	s->pos = r->line_start_pos;
	s->start_col = r->val.start_pos;
        return s;
}

void init_varStmt(struct reader *r, struct stmt *stmt, bool is_mutable) {
	stmt->type = STMT_VAR;
	stmt->var = calloc(1, sizeof(*(stmt->var)));
	
	if (!stmt->var)
		raise_syntax_error(ERR_NO_MEM, r);
	stmt->var->is_mutable = is_mutable;
}

void init_ifStmt(struct reader *r, struct stmt *stmt) {
	stmt->type = STMT_IF;
	stmt->ifStmt = calloc(1, sizeof(*(stmt->ifStmt)));
	
	if (!stmt->ifStmt)
		raise_syntax_error(ERR_NO_MEM, r);
}

void init_loopStmt(struct reader *r, struct stmt *stmt) {
	stmt->type = STMT_LOOP;
	stmt->loop = calloc(1, sizeof(*(stmt->loop)));
	
	if (!stmt->loop)
		raise_syntax_error(ERR_NO_MEM, r);
}

void init_expStmt(struct stmt *stmt, struct exp *exp) {
	stmt->type = STMT_EXPR;
	stmt->exp = exp;
}

// for later error checking
bool stmts_match(const struct stmt *stmt1, const struct stmt *stmt2) {
	if (!stmt1 != !stmt2)
		return false;
	else if (stmt1 == stmt2) //handles the two pointing to the same address and both being null
		return true;
	else if ((stmt1->type != stmt2->type) || !stmts_match(stmt1->next, stmt2->next))
		return false;

	switch (stmt1->type) {
	case STMT_EMPTY:
		return true;
	case STMT_VAR:
		bool names_match = exps_match(stmt1->var->name, stmt2->var->name);
		bool vals_match = exps_match(stmt1->var->value, stmt2->var->value);
		bool muts_match = stmt1->var->is_mutable == stmt2->var->is_mutable;
		return names_match && vals_match && muts_match;
	case STMT_LOOP:
		bool conds_match = exps_match(stmt1->loop->cond, stmt2->loop->cond);
		bool bodys_match = stmts_match(stmt1->loop->body, stmt2->loop->body);
		return conds_match && bodys_match;
	case STMT_IF:
		bool if_conds_match = exps_match(stmt1->ifStmt->cond, stmt2->ifStmt->cond);
		bool thens_match = stmts_match(stmt1->ifStmt->thenStmt, stmt2->ifStmt->thenStmt);
		bool elses_match = stmts_match(stmt1->ifStmt->elseStmt, stmt2->ifStmt->elseStmt);
		return if_conds_match && thens_match && elses_match;
	case STMT_EXPR:
		return exps_match(stmt1->exp, stmt2->exp);
	default:
		raise_error(ERR_INV_STMT);
		//ERR_INV_STMT
	}
	return false;
}