#include <stdio.h>
#include <string.h>
#include "interp.h"
#include "parser.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    //printf("Hello World!\n");
    // now with brainf:
    //interp(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");

    Stmt *expression;
    
    for (int i = 1; (i < argc) && (argv[i] != NULL); i++) {
	if (!argv[i])
	    raise_error("invalid string");

	//printf("before\n");
	expression = parse_file(argv[i]);
	//printf("after parse_file %p\n", expression);
    	print_stmt(expression);
	//printf("after print_full_exp\n");
	free_stmt(expression);
	//printf("after freeExp\n");
	expression = NULL;
    }
    if (argc <= 1) {
		//printf("before\n");
		expression = parse_file("example.txt");
		print_stmt(expression);
		free_stmt(expression);
		expression = NULL;
    } 
}
