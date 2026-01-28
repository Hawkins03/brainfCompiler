/** @file parser.h
 *  @brief Function prototypes for parsing individual values into stmts.
 *
 *  This contains the prototypes for parsing individual values into stmts.
 *  there are a lot of subfunctions that are called by the ones here,
 *  but these are the "levers" that can be pulled from outside the file.
 *  
 *  @author Hawkins Peterson (hawkins03)
 *  @bug No known bugs.
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "utils.h"
#include "stmt.h"
#include "exp.h"
#include "parser.h"

/** @brief parses values into an assignable value
 * 
 * parses names, array refrences, and unary expressions only.
 * (this exists mainly to catch someone trying to do 1++)
 * 
 * @param r the reader struct for throwing errors and getting values.
 * @param exp the expression the output is returned to
 * 
 * @throw ERR_INV_OP if an invalid op is used as a unary op.
 * @throw ERR_NO_MEM if an init function fails 
*/
bool parse_assignable(struct reader *r, struct exp *exp);

/** @brief parses values into an atomic expression and stores it in the in argument
 * 
 * parses names, array_lits/refs, unary expressions, numbers and strings
 * 
 * @param r the reader struct for throwing errors and getting values.
 * @param exp the expression the output is returned to
 * 
 * @throw ERR_TO_LONG if the string is too long
 * @throw ERR_INV_VAL if there is an unexpected
 * 	token when acceptValue catches an invalid value
 * 	or an invalid keyword is invoked (i.e. else for example)
 * @throw ERR_INV_OP if an invalid op is used as a unary op in parse_assignable.
 * @throw ERR_NO_MEM if an init function fails.
*/
void parse_atom(struct reader *r, struct exp *exp);

/** @brief parses expressions, usually in the form of <Atom> <op> <Atom>
 * 
 * parses expressions, usually in the form of <Atom> <op> <Atom>
 * specifically binary ops and assign-ops.
 * 
 * @param minPrio used to ensure that the
 * 	operators are of the correct arangement.
 * 	i.e. x + 1 * 2 ~= x + (1 * 2), not (x + 1) * 2 and so on
 * @param r the reader struct for throwing errors and getting values.
 * @param exp the expression to return to
 *
 * @throw any errors from parse_atom
 * @throw ERR_INV_EXP if you try to set a non-assignable value (1=x for example)
*/
void parse_exp(int minPrio, struct reader *r, struct exp *exp);

/** @brief parses values into a single statement
 * 
 * parses variable definitions, while loops,
 * if/else statements and for loops.
 * 
 * it exists specifically so for loops can parse
 * the initial expression (i.e. for (int i = 0; ... it's that part). 
 * 
 * It also eats the semicolon at the end of expressions.
 * 
 * @param r the reader struct for throwing errors and getting values.
 * @param stmt the statement the output is returned to
 * 
 * @throw any errors from parse_exp
 * @throw ERR_INV_EXP if you try to define an integer
 * 	equal to an array, or an array to an integer 
 * 	and not an array_lit, or the initializer
 * 	statement of a for loop is invalid 
*/
void parse_single_stmt(struct reader *r, struct stmt *stmt);

/** @brief calls parse_single_stmt for an
 * 	infinitely large series of statements
 * 
 *  @param r the reader struct for throwing errors and getting values
 * @param stmt the statement the output is returned to
 * @throw any errors from parse_single_stmt
 */
void parse_stmt(struct reader *r, struct stmt *stmt);

/** @brief creates a reader and initial stmt
 * 	and then calls parse_stmt on it
 * 
 * @param filename the name of the file to
 * 	parse into a statement
 * @throw any errors form parse_stmt
 * @return the statement linked list
 * 	interpretation of the file.
 */
struct stmt *parse_file(const char *filename);


#endif //PARSER_H