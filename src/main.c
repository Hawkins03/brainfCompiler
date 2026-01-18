#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp.h"
#include "parser.h"
#include "semantics.h"
#include "utils.h"
#include "test.h"
#include "stmt.h"

int main(int argc, char *argv[]) {
	// uncomment to run interpreter (should be hello world)
    	// interp(">+++++++++[<++++++++>-]<.>+++++++[<++++>-]<+.+++++++..+++.[-]>++++++++[<++++>-] <.>+++++++++++[<++++++++>-]<-.--------.+++.------.--------.[-]>++++++++[<++++>- ]<+.[-]++++++++++.");

	bool sem = false;
	int avoid = -1;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--semantic") || !strcmp(argv[i], "-s")) {
			avoid = i;
			sem = true;
		}
	}

	if (argc <= 1)
		raise_error(ERR_NO_ARGS);

    	for (int i = 1; (i < argc) && (argv[i] != NULL); i++) {
		if (i == avoid)
			continue;
		if (!argv[i])
			raise_error(ERR_NO_ARGS);

		if (sem) {
			check_file_semantics(argv[i]);
		}
		struct stmt *expression = parse_file(argv[i]);
		print_stmt(expression);
		free_stmt(expression);
		expression = NULL;
    	}
}
