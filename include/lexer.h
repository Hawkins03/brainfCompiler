/** @file lexer.h
 *  @brief Function prototypes for reading a
 * 	series of tokens/values from a file.
 *
 *  This contains the prototypes for 
 *  - the utility functions for the
 *  	lexer_ctx struct
 *  - the functions to get a single
 * 	character from the file
 *  - the functions to convert keywords
 * 	and operators into enums
 *  - the functions to get the different
 * 	value types
 *  - the functions to fetch the current value
 *  	and to move the focus to the next value
 *  - and finally, some utility functions that
 * 	act on said values.
 *  
 *  @author Hawkins Peterson (hawkins03)
 *  @bug No known bugs.
 */


#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include "structs.h"

#define DEFAULT_LINE_CAP 256
#define MAX_LINE_LEN 4096
#define MAX_WORD_LEN 32
#define MAX_NUM_LEN 10
#define MAX_OP_LEN 3
#define DELIMS ";()[]{}'\","
#define OP_START "=+-*/%!~<>&^|"
#define ASSIGN_OP_PRIO 0

#define KEYWORDS (const char*[]) {"var", "val", "while", "for", "if", "else", "print", "input", "break", "true", "false", NULL}
#define KEYWORDS_COUNT 11
#define NUM_PRIOS 10

#define OP_STRINGS (const char *[]) 	{"+", "-", 		\
					"*", "/", "%",		\
					"~", "|", "^", "&", 	\
					"<<", ">>", 		\
					"<", "<=", ">", ">=", 	\
					"==", "!=", 		\
					"!", "&&", "||", 	\
					"=", "+=", "-=", 	\
					"*=", "/=", "%=", 	\
					"<<=", ">>=",		\
					"&=", "^=", "|=", 	\
					"++", "--"}		\

#define PREFIX_OPS (enum operator []) {OP_LOGICAL_NOT, OP_BITWISE_NOT, OP_INCREMENT, OP_DECREMENT, OP_MINUS, OP_UNKNOWN}
#define SUFFIX_OPS (enum operator []) {OP_INCREMENT, OP_DECREMENT, OP_UNKNOWN}
#define ASSIGN_OPS (enum operator []) { OP_ASSIGN, OP_PLUS_ASSIGN, OP_MINUS_ASSIGN, \
    				OP_MULTIPLY_ASSIGN, OP_DIVIDE_ASSIGN, OP_MODULO_ASSIGN, \
    				OP_LEFT_SHIFT_ASSIGN, OP_RIGHT_SHIFT_ASSIGN, \
    				OP_BITWISE_AND_ASSIGN, OP_BITWISE_XOR_ASSIGN, OP_BITWISE_OR_ASSIGN, \
				OP_UNKNOWN}

// reading from the file object

/** @brief creates the initial reader struct
 * 
 * also initializes the root statement,
 * copies the filename, and reads in
 * the first line, along with the first
 * token/value.
 * 
 * @param filename the file to read from.
 * @return the reader struct for the file
*/
struct lexer_ctx *readInFile(const char *filename);

/** @brief frees the reader struct and its contents
 * 
 * also frees the root structure,
 * the current line in focus,
 * the file pointer, and if r->val
 * is a string/name, that too.
 * 
 * @param lexer the lexer to free (in oop this would just be the overall container class)
*/
void killReader(struct lexer_ctx *lex);

// turning enums back into strings for printing

/** @brief converts an op from an enum to a string
 * 
 * basically just for printing.
 * If you want to modify it,
 * you have to copy the contents.
 * 
 * @param op the operator to return the string value of
 * @return the string version of the op
*/
const char *getOpStr(enum operator op);

/** @brief converts a keyword from an enum to a string
 * 
 * basically just for printing.
 * If you want to modify it,
 * you have to copy the contents.
 * 
 * @param key the keyword to return the string value of
 * @return the string version of the keyword
*/
const char *getKeyStr(enum key_type key);

//checking op values

/** @brief gets the priority of a given binary operation
 *  
 * check BINARY_OPS in reader.c to
 * see the list of priorities
 * 
 * lower number means lower priority, and
 * if the priority doesn't match the priority
 * of the previous operation, the previous operation
 * becomes the new left value. Otherwise the currrent
 * operation becomes the previous right operation.
 * 
 * @param op the operation to get the priority of
 * @return the integer priority of the operation (-1 if it's not a binary operation)
 */
int getPrio(const enum operator op);

/** @brief checks to see if the operation is an assign operation
 * 
 * i.e. x += y
 * 
 * see ASSIGN_OPS above to see a list of all assign operations
 * it's important because assign ops are always highest priority,
 * and the fact that they're handled differently in terms of semantics
 * 
 * @param op the possible assign operation
 * @return if op is an assign operation
 */
bool isAssignOp(const enum operator op);

/** @brief checks if the op is a non-assign binary operation
 * 
 * i.e. x * y
 * 
 * basically just checks if getPrio(op) >= 0
 * 
 * @param op the possible binary operation
 * @return if the op is a binary operation
*/
bool isBinaryOp(const enum operator op);

/** @brief checks if the op is a suffix unary operation
 * (basically just ++ and --, but see SUFFIX_OPS for if that changes)
 * 
 * is_suffix and is_prefix aren't
 * combined because they're atomic
 * @param op the possible suffix unary operation
 * @return if op is a suffix unary operation
*/
bool is_suffix_unary(enum operator op);

/** @brief checks if the op is a prefix unary operation
 * see PREFix_OPS for the full list
 * 
 * is_suffix and is_prefix aren't
 * combined because they're atomic
 * 
 * @param op the possible prefix unary operation
 * @return if op is a prefix unary operation
*/
bool is_prefix_unary(enum operator op);

/** @brief advances the focus to the next token/value
 *  
 *  since r->val is the only
 *  token handled, it doesn't return it or anything.
 * 
 *  calls a number of static inline functions for
 *  readability's sake
 *  
 *  @param lex the lexer to read from
 *  no return value, just call peakValue
 */
void nextValue(struct lexer_ctx *lex);

/** @brief basically just returns r->val or null if r isn't defined
 * 
 * it's mainly there so if r becomes undefined the program continues as written
 * 
 * @param lex the lexer to read from
 * @return r->val. It's not malloc'd, but strings inside it are
 * @throw ERR_UNEXP_CHAR if the character doesn't match a given value type
 */
struct value peekValue(struct lexer_ctx *lex);

/** @brief checks if r->val equals a specific value
 * 
 *  it's mainly for eating keywords or delimitors in parse functions
 * 
 *  @param lex the lexer to read from
 *  @param type the expected value type (either VAL_KEYWORD or VAL_DELIM)
 *  @param expected the expected value
 *  @throw ERR_INTERNAL if expected isn't defined
 *  @throw ERR_INV_VAL if there's a mismatch
 */
void acceptValue(struct lexer_ctx *lex, enum value_type type, const char *expected);

/** @brief returns r->val->str and sets it to null
 *  
 * that way it's not freed if there's an error,
 * as well as defines the ownership of the string
 * 
 * note the difference between string and name,
 * as string is a literal string, quotes and everything
 * and gets handled as an array_lit
 * 
 * @param lex the lexer to read from
 * @return r->val->str if it's a string
 * @throw ERR_INV_VAL if the value isn't a string
 */
char *stealNextString(struct lexer_ctx *lex);

/** @brief returns r->val->str and sets it to null
 * 
 * that way it's not freed if there's an error,
 * as well as defines the ownership of the string
 * 
 * note the difference between string and name,
 * as string is a literal string, quotes and everything
 * and gets handled as an array_lit. Name in this case
 * is a variable name
 * 
 * @param lex the lexer to read from
 * @return r->val->str if it's a name value
 * @throw ERR_INV_VAL if the value isn't a name
 */
char *stealNextName(struct lexer_ctx *lex);

/** @brief basically just a macro to get the next op and then move the focus of r->val
 *  
 * I may move this and the previous
 * two functions into parser.c later
 * 
 * @param lex the lexer to read from
 * @return the operator. If it's not an op, OP_UNKNOWN
 */
enum operator stealNextOp(struct lexer_ctx *lex);

#endif //LEXER_H