#include "parser.h"
#include "utils.h"
#include "test.h"




int main(int argc, char **argv) {
	int catch_errors = 0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--catch-errors") || !strcmp(argv[i], "-c"))
			catch_errors = 1;
	}

	run_parser_tests(catch_errors);
	run_error_tests("./tests/valid");
	run_error_tests("./tests/invalid");
	
}
