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

	if (argc <= 1)
		raise_error("bfCompile requires one or more files to run. Usage: ./bfCompiler <files>");

    for (int i = 1; (i < argc) && (argv[i] != NULL); i++) {
		if (!argv[i])
			raise_error("invalid argument");
	

		//test_file(argv[i], "");
		stmt_t *expression = parse_file(argv[i]);
		
		print_stmt(expression);
		free_stmt(NULL, expression);
		expression = NULL;
    }
}
