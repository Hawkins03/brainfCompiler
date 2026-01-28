a simple project written to compile into and interpret the language brainf.

REQUIREMENTS:
 - VSCODE
 - CMAKE & ctest
 - THE CMAKE extension suite by microsoft for VScode
 - lcov
 - c and gcc

if it helps, I am using this in wsl ubuntu on a remote vscode shell running in wsl:ubuntu mode.

see schema.txt for the schema.

Due to the explicit lack of any memory structure other than the stack essentially, there is no functions other than hardcoded ones (print, input and break)

if break is called outside a loop, it raises an error (TODO)

if statements are essentially just checking if the internal condition variable is equal to zero or one.
TODO: an array must have a defined sized. (it can be unknown, or to-be-assigned via the input fn, but they must be defined) - will be done in ir.c.

TODO: add option to make error statements colored

TODO: add compiling stmts/exps into an ir
 - for ops, use multiple steps, and modify the inputted op to do so
  - eg x leftshift y exports while (y) {interp(x /= 2)} if that makes sense

TODO: add intepreting for stmts and exps