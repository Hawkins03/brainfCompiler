#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"
#include "parser.h"
#include "utils.h"
#include "test.h"

int main(int argc, char *argv[]) {
	// uncomment to run interpreter (should be hello world)
    // interp(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");

    stmt_t *expression;
    
    for (int i = 1; (i < argc) && (argv[i] != NULL); i++) {
		if (!argv[i]) {
			fprintf(stderr, "invalid argument");
			exit(EXIT_FAILURE);
		}
			

		expression = parse_file(argv[i]);
		
		print_stmt(expression);
		free_stmt(expression);
		expression = NULL;
    }
    if (argc <= 1) {
		expression = parse_file("example.txt");
		print_stmt(expression);
		free_stmt(expression);
		expression = NULL;
    } 
}
