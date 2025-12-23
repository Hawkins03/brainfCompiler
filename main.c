#include <stdio.h>
#include <string.h>
#include "bf.h"
#include "ms.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    //printf("Hello World!\n");
    // now with brainf:
    //bf_interpret(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");

    Exp *expression;
    
    for (int i = 1; (i < argc) && (argv[i] != NULL); i++) {
	if (!argv[i])
	    raise_error("invalid string");

	printf("before\n");
	expression = parse_file(argv[i]);
	printf("after parse_file %p\n", expression);
    	print_full_exp(expression);
	printf("after print_full_exp\n");
	free_exp(expression);
	printf("after freeExp\n");
	expression = NULL;
    }
    if (argc <= 1) {
	printf("before\n");
	expression = parse_file("example.txt");
	print_full_exp(expression);
	free_exp(expression);
	expression = NULL;
    } 
}
