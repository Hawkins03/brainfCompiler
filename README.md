a simple project written to compile into and interpret the language brainf.

see schema.txt for the schema.
Due to the explicit lack of any memory structure other than the stack essentially, there is no functions other than hardcoded ones (print, input and break)
if break is called outside a loop, it ends the program.
if statements are essentially just checking if the internal condition variable is equal to zero or one.
TODO: an array must have a defined sized. (it can be unknown, or to-be-assigned via the input fn, but they must be defined).

DONE: && / || / characters

TODO: unit testing
    printing out in EXP(left,op,right,etc)
    compare it to an expected string
TODO: handle double/single quotes. chars can just be ints, and can handle quotes with arrays > do with loops.


TODO: handle \'
TODO: find a way to parse functions into brainf.
TODO: split unary and regular ops
TODO: handle comments
TODO: handle true/false keywords as 1 and 0 respectivly
TODO: ensure unary ops are seperate from binary ops.