#include <stdio.h>
#include <string.h>
#include "bf.h"
#include "ms.h"

int main(int argc, char *argv[]) {
    printf("Hello World!\n");
    // now with brainf:
    //bf_interpret(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");

    MS_Atom *expression;
    
    expression = parse_file("example.txt");
    print_full_exp(expression);
    free_atom(expression);
    expression = NULL;
}
