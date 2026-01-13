#include <ctype.h>
#include <errno.h>
#include <execinfo.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "value.h"
#include "reader.h"
#include "semantics.h"



// Reader functions
char* strdup(const char* s, Reader *r) {
    size_t len = strlen(s);
    char* result = calloc(len + 1, sizeof(*result));
    if (result == NULL)
		raise_syntax_error("failed to allocate space for a string", r);
    strcpy(result, s);
    return result;
}



// Error Handling
void _raise_error(const char *msg, const char *func, const char *file, int line) {
	fprintf(stderr, "ERROR in %s at %s:%d - %s\n", func, file, line, msg);
	exit(EXIT_FAILURE);
}

void _raise_syntax_error(const char *msg, const char *func, const char *file, int line, Reader *r) {
	if (r)
		killReader(r);
	_raise_error(msg, func, file, line);
}

void _raise_semantic_error(const char *msg, const char *func, const char *file, int line, env_t *env) {
	if (env)
		free_env(env);
	_raise_error(msg, func, file, line);
}