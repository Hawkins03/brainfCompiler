#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"
#include "parser.h"
#include "utils.h"
#include "test.h"
#include "stmt.h"

int main(int argc, char *argv[]) {
	// uncomment to run interpreter (should be hello world)
    	// interp(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");

	printf("%d\n", OP_PLUS);

	if (argc <= 1)
		raise_error(ERR_NO_ARGS);

    	for (int i = 1; (i < argc) && (argv[i] != NULL); i++) {
		if (!argv[i])
			raise_error(ERR_NO_ARGS);

		struct stmt *expression = parse_file(argv[i]);
		print_stmt(expression);
		free_stmt(expression);
		expression = NULL;
    	}
}
