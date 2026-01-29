/** @file exp.h
 *  @brief Function prototypes for expressions.
 *
 *  This contains the prototypes for the exp struct
 *  and eventually any macros, constants,
 *  or global variables that are needed.
 *
 *  @author Hawkins Peterson (hawkins03)
 *  @bug no known bugs
 */


#ifndef EXP_H
#define EXP_H

#include <stdbool.h>
#include <string.h>
#include "structs.h"

#define DEFAULT_CAP_SIZE 8

/** @brief Frees a malloc'd expression.
 *  by default it's called by free_stmt.
 * It should set any internal variables
 * to null, and it should free any 
 * subexpressions, as well as any strings 
 * that are malloc'd
 * 
 * @param exp the expression to free
 */
void free_exp(struct exp  *exp);

/** @brief Prints an expression.
 * 
 * By default it's called by print_stmt.
 * It should print out a description of
 * the expression, along with any arguments
 * it holds.
 * 
 * @param exp the expression to free
 */
void print_exp(const struct exp  *exp);

/** @brief Malloc's and initializes an expression
 * 
 * by default it's called by parser
 * functions that are creating subexpressions.
 * 
 * It should malloc the exp, set it's type to
 * EXP_EMPTY, and save a few things for error
 * printing 
 * 
 * @param r the reader struct for if it fails
 * 	and to get the location from
 * @throw ERR_NO_MEM if it fails to malloc the exp
 * @return the malloc'd expression
 */
struct exp *init_exp(struct lexer_ctx *lex);

/** @brief Malloc's and initializes a binary subexpression
 * 
 * i.e. x + 1 or x += 1
 * 
 * by default it's called by parseExp
 * 
 * It should malloc the binary exp, set it's type to
 * tp, and update exp to include the malloc'd subexpression
 * 
 * @param r the reader struct, for if it fails
 * @param exp the expression to update
 * @param tp the type of subexpression
 * 	(EXP_BINOP or EXPASSIGNOP, NOT EXP_UNARY)
 * @param left exp->op->left so the program errors it gets freed
 * @throw ERR_NO_MEM if it fails to malloc the subexpression
 */
void init_binary(struct lexer_ctx *lex, struct exp *exp, enum exp_type tp, struct exp *left);

/** @brief Malloc's and initializes a unary subexpression
 * 
 * i.e. ++x or x--
 * 
 * by default it's called by parseUnary
 * 
 * It should malloc the unary exp, set it's type to
 * EXP_UNARY, and update exp to include the malloc'd subexpression
 * 
 * @param r the reader struct, for if it fails
 * @param exp the expression to update
 * @throw ERR_NO_MEM if it fails to malloc the subexpression
 */
void init_exp_unary(struct lexer_ctx *lex, struct exp *exp, bool is_prefix);

/** @brief Malloc's and initializes a array_ref subexpression
 * 
 * i.e. x[1] or x[y][z]
 * 
 * by default it's called by parseArrayRef
 * 
 * It should malloc the array_ref subexpression,
 * set it's type to EXP_ARRAY_REF, and update
 * exp to include the malloc'd subexpression
 * 
 * @param r the reader struct, for if it fails
 * @param exp the expression to update
 * @param name exp->array_ref->name so the program errors it gets freed
 * @throw ERR_NO_MEM if it fails to malloc the subexpression
 */
void init_exp_array_ref(struct lexer_ctx *lex, struct exp *exp, struct exp *name);

/** @brief Malloc's and initializes a literal array subexpression
 * 
 * i.e. {x, y} or { {x}, {y}} or {[0] = 1, [1] = 2}
 * 
 * by default it's called by parseArrayLit
 * 
 * It should malloc the literal array exp, set it's type to
 * EXP_ARRAY_LIT, and update exp to include the malloc'd subexpression
 * 
 * @param r the reader struct, for if it fails
 * @param exp the expression to update
 * @param size the size of the literal array
 * 	(not the total size of the arrray variable,
 * 	just the number of exps in the literal array)
 *  @throw ERR_NO_MEM if it fails to malloc the subexpression
 */
void init_exp_array_lit(struct lexer_ctx *lex, struct exp *exp, int size);

/** @brief sets the length of the array_lit to a final value
 *  called at the end of parseString and parseArrayLit to shorten the amount of memory used.
 * 
 * @param r for if there's an error
 * @param exp the exp to update (must be of type EXP_ARRAY_LIT)
 * @param final_len the final length to set it to.
 * @throw ERR_NO_MEM if the realloc fails.
 */
void set_exp_arraylit_len(struct lexer_ctx *lex, struct exp *exp, int final_len);

/** @brief Malloc's and initializes a call subexpression
 * 
 * by default it's called by parseCall's subfunctions
 * 
 * It should malloc the call exp, set it's type to
 * EXP_CALL, set exp->call->key to key,
 * and update exp to include the malloc'd subexpression
 * 
 * @param r the reader struct, for if it fails
 * @param exp the expression to update
 * @param key the specific function being called
 * 	(i.e. break / input / print, not else, if, or while)
 *  @throw ERR_NO_MEM if it fails to malloc the subexpression
 */
void init_exp_call(struct lexer_ctx *lex, struct exp *exp, enum key_type key);

/** @brief swaps the  contents of two functions
 * 
 * by default it's called by a few
 * parse functions that update the "in" exp to
 * contain the previous in exp
 * 
 * It should swap the contents of the two functions
 * 
 * @param exp1 the two expressions to swap
 * @param exp2 the two expressions to swap
 */
void swap_exps(struct exp *exp1, struct exp *exp2);

/** @brief checks if two exps are equivalent
 * 
 * recursivly checks if the exp subexpressions are equal.
 * 
 * @param exp1 the first expression to check
 * @param exp2 the second expression to check
 */

bool exps_match(struct exp  *exp1, struct exp  *exp2);

/** @brief confirms that either both exp1 and exp2 are ints or arrays
 * 
 * checks exp1 and exp2 and goes through each to get the type
 * 
 * @param exp1 the first exp to check (usually left or name)
 * @param exp2 the second exp to check (usually right or value)
 */
bool exps_are_compatable(struct exp *exp1, struct exp *exp2);

#endif //EXP_H