#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "lexer.h"
#include "ir.h"

void init_ir_ctx(struct ir_ctx *ctx, struct stmt *root) {
	ctx->root = root;
	ctx->ir_root = init_node(ctx);
	ctx->ir_root->next = ctx->ir_root;
	ctx->var_num = 0;
}

struct ir_node *init_node(struct ir_ctx *ctx) {
	struct ir_node *node = calloc(1, sizeof(*node));
	if (!node)
		raise_ir_error(ERR_NO_MEM, ctx);
	
	node->type = NODE_EMPTY;
        return node;
}

static void init_ir_assign(struct ir_ctx *ctx, struct ir_node *node) {
	if (!node)
		raise_ir_error(ERR_INTERNAL, ctx);
	
	node->type = NODE_ASSIGN;
	node->assign = calloc(1, sizeof(*(node->assign)));
	if (!node->assign)
		raise_ir_error(ERR_NO_MEM, ctx);
}

static void init_ir_loop(struct ir_ctx *ctx, struct ir_node *node) {
	if (!node)
		raise_ir_error(ERR_INTERNAL, ctx);
	
	node->type = NODE_LOOP;
	node->loop = calloc(1, sizeof(*(node->loop)));
	if (!node->loop)
		raise_ir_error(ERR_NO_MEM, ctx);
}

static void free_ir_val(struct ir_val val) {
	if ((val.type == IR_VAL_VAR) && val.var && (val.var[0] == '_')) {
		free(val.var);
		val.var = NULL;
	}
}

void free_ir_node(struct ir_node *node) {
	if (!node)
		return;

	switch (node->type) {
	case NODE_ASSIGN:
		if ((node->assign->dest) && (node->assign->dest[0] == '_')) {
			free(node->assign->dest);
			node->assign->dest = NULL;
		}

		free_ir_val(node->assign->lhs);
		free_ir_val(node->assign->rhs);
		free(node->assign);
		node->assign = NULL;
		break;
	case NODE_LOOP:
		free(node->loop);
		node->loop = NULL;
		break;
	case NODE_EMPTY:
		break;
	}
	free_ir_node(node->next);
	free(node);
}

static inline void print_ir_val(struct ir_val val) {
	if(val.type == IR_VAL_NUM)
		printf("%d", val.num);
	else if (val.type == IR_VAL_VAR)
		printf("%s", val.var);
}

void print_ir_node(struct ir_node *node) {
	if (!node)
		return;

	switch (node->type) {
	case NODE_EMPTY:
		printf("Empty();\n");
	case NODE_ASSIGN:
		printf("%s = ", node->assign->dest);
		print_ir_val(node->assign->lhs);
		printf(" %s, ", getOpStr(node->assign->op));
		print_ir_val(node->assign->rhs);
		printf(";\n");
	case NODE_LOOP:
		printf("LOOP(%s)\n{", node->loop->cond);
		print_ir_node(node->loop->body);
		printf("}\n");
	}
	print_ir_node(node->next);
}

static char *create_temp_name(struct ir_ctx *ctx) {
	char *out = calloc(14, sizeof(*out));
	if (!out)
		raise_ir_error(ERR_NO_MEM, ctx);
	sprintf(out, "_r%d", ctx->var_num);
	return out;
}

//returns the deepest node.
struct ir_node *convert_stmt(struct ir_ctx *ctx, struct stmt *stmt, struct ir_node *node) {
	raise_ir_error(ERR_INTERNAL, ctx); //not ready for use yet
	if (!stmt)
		return;

	switch (stmt->type) {
	case STMT_EMPTY:
		raise_ir_error(ERR_INTERNAL, ctx);
		break;
	case STMT_EXPR:
		struct ir_node *last = covert_exp(ctx, stmt->exp, node);
		if (!stmt->next)
			return last;
		last->next = init_node(ctx);
		return convert_stmt(ctx, stmt->next, last->next);
		break;
	case STMT_VAR:
		char *val_name = create_temp_name(ctx);
		ctx->var_num--;

		//TODO: if !value return 0;
		struct ir_node *last = convert_exp(ctx, stmt->var->value, node);

		last->next = init_node(ctx);
		init_ir_assign(ctx, last->next);
		last->next->assign->dest = stmt->var->name;
		last->next->assign->lhs.type = IR_VAL_VAR;
		last->next->assign->lhs.var = val_name;
		last->next->assign->op = OP_ASSIGN;
	case STMT_IF:
		char *cond_name = create_temp_name(ctx);
		ctx->var_num--;

		struct ir_node *cond = convert_exp(ctx, stmt->ifStmt->cond, node);
		cond->next = init_node(ctx);
		struct ir_node *loop = cond->next;
		init_ir_loop(ctx, loop);
		loop->loop->cond = cond_name;

		loop->loop->body = init_node(ctx);
		struct ir_node *body_end = convert_stmt(ctx, stmt->ifStmt->thenStmt, loop->loop->body);

		body_end->next = init_node(ctx);
		struct ir_node *update = body_end->next;
		init_ir_assign(ctx, update);

		update->assign->dest = cond_name;
		update->assign->op = OP_DECREMENT;

		if (stmt->ifStmt->elseStmt) {
			loop->next = init_node(ctx);
			struct ir_node *update;
			
			//loop then --
		}

	case STMT_LOOP:

	}

	return init_node(ctx);
}

struct ir_node *convert_exp(struct ir_ctx *ctx, struct exp *exp, struct ir_node *node) {
	raise_ir_error(ERR_INTERNAL, ctx); //not ready for use yet
	if (!exp)
		return NULL;

	return init_node(ctx);
}