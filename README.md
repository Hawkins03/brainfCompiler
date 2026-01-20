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



TODO: add a preprocessor
TODO: add more error checking

TODO: use cmake, and use the (suprise!) tool that was built for error testing, either instead or alongside my own tool to do the same. Also use coverage testing to ensure that the test suite gets total coverage and that if there's an error, the code can diagnose it. Also use this to figure out where I need more error testing.