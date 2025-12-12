a simple project written to compile into and interpret the language brainf.

the source language:

<atom>::= int | '(' <exp> ')'
<op>::= any one of these characters "+-*/%"
<exp>::= <atom> (<op> <atom>)* | <atom>(<exp>)

<var>::= ["let"|"var"] name = <exp>   <-- TODO

TODO: update the source to accept parenthesis
TODO: update reader.h to read into a buffer? (Nope, it already does that)
TODO: update to accept variables
TODO: write a parser to work with the ir.
TODO: find a way to parse functions into brainf.
