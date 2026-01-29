/** @file ir.h
 *  @brief the code to lower my ast into the ir
 * 
 * note, will need to treat all numbers as unsigned integers,
 * and ensure they're not zero (in this case just add 1 to them,
 * and subtract 1 when printing.)
 * 
 * realistally, for an expression, the two arguments will need to be in adjacent cells
 * 
 */

void init_ir_ctx(struct ir_ctx *ctx, struct stmt *root);

struct ir_node *init_node(struct ir_ctx *ctx);

void free_ir_node(struct ir_node *node);

void print_ir_node(struct ir_node *node);

struct ir_node *convert_stmt(struct ir_ctx *ctx, struct stmt *stmt);

struct ir_node *convert_exp(struct ir_ctx *ctx, struct exp *exp);