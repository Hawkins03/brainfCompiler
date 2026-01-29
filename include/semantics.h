/** @file semantics.h
 *  @brief checks the semantics of a stmt tree
 *
 *  This contains the prototypes for 
 *  - the env utility functions
 *  - the variable tracking functions
 *  - the semantic checker functions
 *  
 *  @author Hawkins Peterson (hawkins03)
 *  @bug likely bugs with how the error printer 
 * 	locates the errors in the file
 */


#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stdbool.h>
#include "structs.h"

#define STARTING_ENV_CAP 8

//env utility functions

/** @brief malloc's the variable buffer, and sets a few values
 * 
 * sets env->parent and env->root for freeing if there's an error.
 * It takes env as a parameter because as env isn't returned, when
 * handled in this manner, it doesn't have to be malloc'd
 * (other than the variable buffer, but less malloc, mo better)
 * 
 * @param env the env to initialize.
 * @throw ERR_NO_MEM if it fails to malloc env->vars
 */
void setup_env(struct env *env, struct env *parent);

/** @brief frees the env variable buffer and root stmt
 * 
 * also calls free_env(env->parent)
 * 
 * @param env the env to free.
 * @throw ERR_NO_MEM if it fails to malloc env->vars
 */
void free_env(struct env *env);

/** @brief stores a variable name and details in the env
 * 
 * array_depth is 0 if it's not an array,
 * and otherwise it's the dimension. I
 * did it this way, so that it's easier to
 * determine the arrayness of a multi-dimension
 * array.
 * 
 * i.e. for the 2d array x[y][z],
 * x[y] is an array, but x[y][z] isn't.
 * 
 * @param env the env to add the variable to.
 * @param name the name of the variable to add.
 * @param is_mutable if the variable is mutable.
 * @param array_depth the dimension of the array.
 * @return if it successfully added the variable
 */
bool define_var(struct env *env, char *name, bool is_mutable, int array_depth);

/** @brief fetches a name from the env
 * 
 * see structs for the var_data struct.
 * 
 * @param env the env to get the variable from.
 * @param name the name of the variable
 * @return the variable metadata
 */
struct var_data *get_var(const struct env *env, const char *name);

/** @brief checks the semantic of an expression.
 *  RULES:
  * 1. symbol and binding rules:
  *   1.1: a variable must be declared before it is used (eg: "x=3; var x;" is invalid)
  *   1.2: no redeclaring variables in the same scope
  *   1.3: symbols declared with val are immutable
  *   1.4: to reffer to a name as an array, it must be declared as an array.
  * 2. assignment rules:
  *   2.1: left hand side must be assigable and mutable.
  *   2.2: ints get assigned to ints, arrays get assigned to arrays
  *   2.3: for name <op>= value, name and value must be ints
  * 3. unary rules:
  *   3.1: for ++, --, and - unary operators, they must eventually refer directly to assignable values.
  * 4. binary op rules:
  *   4.1: all assignments must be ints.
  * 6: print and input take and return int values only.
 * 
 * @param env the env to use
 * @param exp the exp to check
 *
 * @throw ERR_INV_ARR if an array is used in an inpropper place (non-assign binary operations for example)
 * @throw ERR_NO_VAR if a name expression fails to find the variable in env
 * @throw ERR_INV_EXP if an operation is used in an invalid manner.
 */
void check_exp_semantics(struct env *env, struct exp *exp);

/** @brief checks the semantics of a statement.
 *  
 * RULES:
 * 1. All exps must be valid.
 * 2. control flow rules:
 *   2.1: all conds must be int type.
 * 3. declaration rules:
 *   3.1 a variable must be declared before it is used (eg: "x=3; var x;" is invalid)
 *   3.2 no redeclaring variables in the same scope
 *   3.3 symbols declared with val are immutable
 *   3.4 see assignment rules
 * 
 * @param env the env to use
 * @param exp the exp to check
 * 
 * @throw ERR_INV_ARR if an array is used in an inpropper place (non-assign binary operations for example)
 * @throw ERR_REDEF if define_variable fails.
 * @throw ERR_INV_STMT shouldn't be called, but if stmt->type is STMT_EMPTY
 */
void check_stmt_semantics(struct env *env, struct stmt *stmt);

/** @brief checks the semantics of a file.
 *  
 * runs check_stmt_semantics on parse_file_semantics(filename)
 * 
 * @param filename the file to check the semantics of.
 * @throw see check_stmt_semantics for thrown error
 */
void check_file_semantics(char *filename);

#endif //SEMANTICS_H