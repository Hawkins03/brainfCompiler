#include "semantics.h"

 /*
  * Checks the semantics of the given statement list.
  * 1. no reassigning a value,
  * 2. no redefining a variable (i.e. no "var x;" if x already exists),
  * 3. no referencing undefined variables.
  * 4. a variable is assigned in all paths (if and loops included) before it's used.
  * 5. no obvious division by zero.
  * 6. Expressions will be expanded, such that they can't be internally nested.
  * 7. if a variable is assigned in only one branch of an if statement, it is considered unassigned after the if statement.
  * 8. Integers don't go out of bounds (32 bit signed).
  */
void check_semantics(Stmt *stmt) {

}