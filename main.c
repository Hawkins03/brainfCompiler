#include <stdio.h>
#include <string.h>
#include "bf.h"

int main(int argc, char *argv[]) {
    printf("Hello World!\n");
    // now with brainf:
    bf_interpret(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");
}
