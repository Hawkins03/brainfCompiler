#include <stdio.h>
#include <string.h>
#include "bf.h"
#include "ms.h"

int main() {
    printf("Hello World!\n");
    // now with brainf:
    //bf_interpret(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");

    Exp *expression;
    
    expression = parse_file("example.txt");
    print_full_exp(expression);
    free_exp(expression);
    expression = NULL;
}
