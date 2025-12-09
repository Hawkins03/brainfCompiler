#include <stdio.h>
#include <string.h>
#include "bf.h"
#include "ms.h"

int main(int argc, char *argv[]) {
    printf("Hello World!\n");
    // now with brainf:
    //bf_interpret(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");

    MS_Exp *exp = parse("1+1");
    print_exp(exp);
    free_exp(exp);


}
