#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"
#include "parser.h"
#include "utils.h"
#include "test.h"
#include "stmt.h"
#include "exp.h"

struct strbuf {
    char *buf;
    size_t len;
    size_t cap;
};

void strbuf_init(struct strbuf *sb) {
    sb->cap = 256;
    sb->len = 0;
    sb->buf = malloc(sb->cap);
    if (!sb->buf) {
        raise_error("failed to allocate strbuf");
    }
}

void strbuf_ensure(struct strbuf *sb, size_t needed) {
    while (sb->len + needed >= sb->cap) {
        sb->cap *= 2;
        char *new_buf = realloc(sb->buf, sb->cap);
        if (!new_buf) {
            free(sb->buf);
            raise_error("failed to realloc strbuf");
        }
        sb->buf = new_buf;
    }
}

void strbuf_appendf(struct strbuf *sb, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int needed = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    
    if (needed < 0) {
        raise_error("vsnprintf failed");
    }
    
    strbuf_ensure(sb, needed + 1);
    va_start(ap, fmt);
    sb->len += vsprintf(sb->buf + sb->len, fmt, ap);
    va_end(ap);
}

void strbuf_free(struct strbuf *sb) {
    if (sb && sb->buf) {
        free(sb->buf);
        sb->buf = NULL;
        sb->len = 0;
        sb->cap = 0;
    }
}

// Forward declarations
void build_exp_str(struct strbuf *sb, const struct exp *exp);
void build_stmt_str(struct strbuf *sb, const struct stmt *stmt);

void build_exp_str(struct strbuf *sb, const struct exp *exp) {
    if (!exp) {
        strbuf_appendf(sb, "NULL");
        return;
    }
    
    switch (exp->type) {
    case EXP_EMPTY:
        strbuf_appendf(sb, "EMPTY()");
        break;
    case EXP_NAME:
        strbuf_appendf(sb, "STR(%s)", exp->name);
        break;
    case EXP_ARRAY_REF:
        strbuf_appendf(sb, "ARR(");
        build_exp_str(sb, exp->array_ref->name);
        strbuf_appendf(sb, ", ");
        build_exp_str(sb, exp->array_ref->index);
        strbuf_appendf(sb, ")");
        break;
    case EXP_ARRAY_LIT:
        strbuf_appendf(sb, "ARR_LIT(");
        for (int i = 0; i < exp->array_lit->size; i++) {
            if (i > 0)
                strbuf_appendf(sb, ", ");

            build_exp_str(sb, exp->array_lit->array + i);
        }
        strbuf_appendf(sb, ")");
        break;
    case EXP_NUM:
        strbuf_appendf(sb, "NUM(%d)", exp->num);
        break;
    case EXP_BINARY_OP:
    case EXP_ASSIGN_OP:
        strbuf_appendf(sb, "OP(");
        build_exp_str(sb, exp->op->left);
        strbuf_appendf(sb, ", %s, ", getOpStr(exp->op->op));
        build_exp_str(sb, exp->op->right);
        strbuf_appendf(sb, ")");
        break;
    case EXP_UNARY:
        strbuf_appendf(sb, "UNARY(");
        if (exp->unary->is_prefix) {
            strbuf_appendf(sb, "%s, ", getOpStr(exp->unary->op));
            build_exp_str(sb, exp->unary->operand);
        } else {
            build_exp_str(sb, exp->unary->operand);
            strbuf_appendf(sb, ", %s", getOpStr(exp->unary->op));
        }
        strbuf_appendf(sb, ")");
        break;
    case EXP_CALL:
        strbuf_appendf(sb, "CALL(%d, ", exp->call->key);
        build_exp_str(sb, exp->call->arg);
        strbuf_appendf(sb, ")");
        break;
    }
}

void build_stmt_str(struct strbuf *sb, const struct stmt *stmt) {
    if (!stmt) {
	strbuf_appendf(sb, "NULL");
        return;
    }
    
    switch (stmt->type) {
    case STMT_EMPTY:
        strbuf_appendf(sb, "EMPTY();");
        break;
    case STMT_EXPR:
        build_exp_str(sb, stmt->exp);
        strbuf_appendf(sb, ";");
        break;
    case STMT_VAR: {
        const char *name = (stmt->var->is_mutable) ? "VAR" : "VAL";
        strbuf_appendf(sb, "%s(", name);
        build_exp_str(sb, stmt->var->name);
        strbuf_appendf(sb, ", ");
        build_exp_str(sb, stmt->var->value);
        strbuf_appendf(sb, ");");
        break;
    }
    case STMT_IF:
        strbuf_appendf(sb, "IF(");
        build_exp_str(sb, stmt->ifStmt->cond);
        strbuf_appendf(sb, ", ");
        build_stmt_str(sb, stmt->ifStmt->thenStmt);
        if (stmt->ifStmt->elseStmt) {
            strbuf_appendf(sb, ", ");
            build_stmt_str(sb, stmt->ifStmt->elseStmt);
        }
        strbuf_appendf(sb, ");");
        break;
    case STMT_LOOP:
        strbuf_appendf(sb, "LOOP(");
        build_exp_str(sb, stmt->loop->cond);
        strbuf_appendf(sb, ", ");
        build_stmt_str(sb, stmt->loop->body);
        strbuf_appendf(sb, ");");
        break;
    }
    
    if (stmt->next && stmt->next != stmt) {
        strbuf_appendf(sb, " ");
        build_stmt_str(sb, stmt->next);
    }
}

int test_file(const char *input_file, const char *expected) {
    struct stmt *stmt = parse_file(input_file);

    if (stmt->next == stmt) {
        free_stmt(stmt);
        raise_error("infinite recursion of statement");
	//ERR_INF_REC
    }
    
    struct strbuf sb;
    strbuf_init(&sb);
    build_stmt_str(&sb, stmt);
    
    int status = 0;
    if (strcmp(sb.buf, expected)) {
        fprintf(stderr, "input: \"%s\" mismatch\nact: \t\t\"%s\"\nexpected:\t\"%s\"\n\n",
                input_file, sb.buf, expected);
        status = 1;
    }

    strbuf_free(&sb);
    free_stmt(stmt);
    return status;
}