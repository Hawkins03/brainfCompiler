/** @file stmt.h
 *  @brief the functions for the stmt struct
 *
 *  This contains the prototypes for stmt utility function
 *  
 *  @author Hawkins Peterson (hawkins03)
 *  @bug No bugs
 */

#ifndef STMT_H
#define STMT_H

#include <stdbool.h>
#include "structs.h"

/** @brief frees a statement and it's relevent substructures
 * 
 * see the function itself if you want to know what it frees
 * @param stmt the statement to free
 */
void free_stmt(struct stmt *stmt);

/** @brief prints the string interpretation of a stmt
 * 
 * @param stmt the statement to print
 */
void print_stmt(const struct stmt *stmt);


/** @brief malloc's a stmt struct
 * 
 * also sets stmt->type to STMT_EMPTY
 * can't really be malloc'd because
 * for any given file, there will
 * be n many of them.
 * 
 * @param r for errors.
 * @throw ERR_NO_MEM if it fails to malloc the stmt
 * @return the stmt struct
 */
struct stmt *init_stmt(struct lexer_ctx *lex);

/** @brief malloc's a stmt var substructure
 * 
 * also sets is_maliable and sets type to STMT_VAR
 * 
 * @param r for errors
 * @param stmt the stmt to update
 * @param is_maliable if the value can be changed later
 * @throw ERR_NO_MEM if it fails to malloc stmt->var
 */
void init_varStmt(struct lexer_ctx *lex, struct stmt *stmt, bool is_maliable);

/** @brief malloc's a stmt ifStmt substructure
 * 
 * also sets type to STMT_IF
 * 
 * @param r for errors
 * @param stmt the stmt to update
 * @throw ERR_NO_MEM if it fails to malloc stmt->ifStmt
 */
void init_ifStmt(struct lexer_ctx *lex, struct stmt *stmt);

/** @brief malloc's a stmt loop substructure
 * 
 * also sets type to STMT_LOOP
 * 
 * @param r for errors
 * @param stmt the stmt to update
 * @throw ERR_NO_MEM if it fails to malloc stmt->loop
 */
void init_loopStmt(struct lexer_ctx *lex, struct stmt *stmt);

/** @brief sets stmt->exp to exp
 * 
 * basically just for similarity
 * 
 * @param stmt the stmt to update
 * @param exp the exp to set stmt->exp to
 */
void init_expStmt(struct stmt *stmt, struct exp *exp);


/** @brief checks to see if two statements are equivalent
 * 
 * @param stmt1 the first statement to check
 * @param stmt2 the second statement to check
 * @return if the two are equal
 */
bool stmts_match(const struct stmt *stmt1, const struct stmt *stmt2);
#endif //STMT_H