#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stmt.h"
#include "exp.h"

// freeing functions
void free_stmt(stmt_t *stmt) {
    if (!stmt)
        return;
	else if (stmt->next == stmt)
		stmt->next = NULL;
	stmt->free_fn(stmt);
    free_stmt(stmt->next);
}

void free_empty_stmt(stmt_t *stmt) {
    free_stmt(stmt->next);
    stmt->next = NULL;
    free(stmt);
}

void free_var_stmt(stmt_t *stmt) {
    if (stmt->type != STMT_VAR)
        raise_error("failed polymorphism");
    free_exp(stmt->var.name);
    stmt->var.name = NULL;
    free_exp(stmt->var.value);
    stmt->var.value = NULL;
    free_empty_stmt(stmt);
}

void free_loop_stmt(stmt_t *stmt) {
    if (stmt->type != STMT_LOOP)
        raise_error("failed polymorphism");
    free_exp(stmt->loop.cond);
    stmt->loop.cond = NULL;
    free_stmt(stmt->loop.body);
    free_empty_stmt(stmt);
}

void free_if_stmt(stmt_t *stmt) {
    if (stmt->type != STMT_IF)
        raise_error("failed polymorphism");
    free_exp(stmt->ifStmt.cond);
    stmt->ifStmt.cond = NULL;
    free_stmt(stmt->ifStmt.thenStmt);
    stmt->ifStmt.thenStmt = NULL;
    free_stmt(stmt->ifStmt.elseStmt);
    stmt->ifStmt.elseStmt = NULL;
    free_empty_stmt(stmt);
}

void free_exp_stmt(stmt_t *stmt) {
    if (stmt->type != STMT_EXPR)
        raise_error("failed polymorphism");
    free_exp(stmt->exp);
    free_empty_stmt(stmt);
}

//print functions
void print_stmt(const stmt_t *stmt) {
    if ((!stmt) || (stmt == stmt->next)) {
        printf("NULL;");
        return;
    }
	stmt->print_fn(stmt);
	print_stmt(stmt->next);	
}

void print_empty_stmt(const stmt_t *stmt) {
    if (stmt->type != STMT_EMPTY)
        raise_error("failed polymorphism");
    printf("EMPTY();\n");
}

void print_var_stmt(const stmt_t *stmt) {
    if (stmt->type != STMT_VAR)
        raise_error("failed polymorphism");
    char *name = (stmt->var.is_mutable) ? "VAR" : "VAL";
    printf("%s(", name);
    print_exp(stmt->var.name);
    printf(", ");
    print_exp(stmt->var.value);
    printf(");\n");
}

void print_loop_stmt(const stmt_t *stmt) {
    if (stmt->type != STMT_LOOP)
        raise_error("failed polymorphism");
    printf("LOOP(");
    print_exp(stmt->loop.cond);
    printf(") {\n");
    print_stmt(stmt->loop.body);
    printf("}");
}

void print_if_stmt(const stmt_t *stmt) {
    if (stmt->type != STMT_IF)
        raise_error("failed polymorphism");
    printf("IF(");
    print_exp(stmt->ifStmt.cond);
    printf(") {\n");
    print_stmt(stmt->ifStmt.thenStmt);
    printf("} else {\n");
    print_stmt(stmt->ifStmt.elseStmt);
    printf("}\n");
}

void print_exp_stmt(const stmt_t *stmt) {
    if (stmt->type != STMT_EXPR)
        raise_error("failed polymorphism");
    printf("EXP_");
    print_exp(stmt->exp);
    printf(";\n");
}


// initialization functions
stmt_t *init_stmt() {
	stmt_t *stmt = calloc(1, sizeof(*stmt));
	if (!stmt) return NULL;
	stmt->type = STMT_EMPTY;

    stmt->free_fn = free_empty_stmt;
    stmt->print_fn = print_empty_stmt;
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

    stmt->free_fn = free_var_stmt;
    stmt->print_fn = print_var_stmt;
	return stmt;
}

stmt_t *init_loop(exp_t *cond, stmt_t *body, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(cond);
		free_stmt(body);
		raise_syntax_error("failed to initialize stmt", r);
	}

	stmt->type = STMT_LOOP;
	stmt->loop.cond = cond;
	stmt->loop.body = body;

    stmt->free_fn = free_loop_stmt;
    stmt->print_fn = print_loop_stmt;
	return stmt;
}

stmt_t *init_ifStmt(exp_t *cond, stmt_t *thenStmt, Reader *r) {
	stmt_t *stmt = init_stmt();
	if (!stmt) {
		free_exp(cond);
		free_stmt(thenStmt);
		raise_syntax_error("failed to initialize stmt", r);
	}

	stmt->type = STMT_IF;
	stmt->ifStmt.cond = cond;
	stmt->ifStmt.thenStmt = thenStmt;

    stmt->free_fn = free_if_stmt;
    stmt->print_fn = print_if_stmt;
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

    stmt->free_fn = free_exp_stmt;
    stmt->print_fn = print_exp_stmt;
	return stmt;
}

bool compare_stmts(const stmt_t *stmt1, const stmt_t *stmt2) {
    if (!stmt1 != !stmt2)
        return false;
    else if (stmt1 == stmt2) //handles the two pointing to the same address and both being null
        return true;
    else if ((stmt1->type != stmt2->type) || !compare_stmts(stmt1->next, stmt2->next))
        return false;

    switch (stmt1->type) {
        case STMT_EMPTY:
            return true;
        case STMT_VAR:
            bool names_match = compare_exps(stmt1->var.name, stmt2->var.name);
            bool vals_match = compare_exps(stmt1->var.value, stmt2->var.value);
            bool muts_match = stmt1->var.is_mutable == stmt2->var.is_mutable;
            return names_match && vals_match && muts_match;
        case STMT_LOOP:
            bool conds_match = compare_exps(stmt1->loop.cond, stmt2->loop.cond);
            bool bodys_match = compare_stmts(stmt1->loop.body, stmt2->loop.body);
            return conds_match && bodys_match;
        case STMT_IF:
            bool if_conds_match = compare_exps(stmt1->ifStmt.cond, stmt2->ifStmt.cond);
            bool thens_match = compare_stmts(stmt1->ifStmt.thenStmt, stmt2->ifStmt.thenStmt);
            bool elses_match = compare_stmts(stmt1->ifStmt.elseStmt, stmt2->ifStmt.elseStmt);
            return if_conds_match && thens_match && elses_match;
        default:
            raise_error("invalid stmt type");
    }
    return false;
}



