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
#include "reader.h"
#include "semantics.h"

// struct reader functions
char* strdup(const char* s, struct reader *r)
{
	size_t len = strlen(s);
	char* result = calloc(len + 1, sizeof(*result));
	if (result == NULL)
		raise_syntax_error("failed to allocate space for a string", r);
	strcpy(result, s);
	return result;
}

void set_strlen(char **str, const int len, struct reader *r) {
	char *temp = realloc(*str, len);
	if (!temp) {
		free(*str);
		raise_syntax_error("ran out of memory space on the heap", r);
	}
	*str = temp;
}

void reset_strlen_if_needed(char **str, const int len, int *cap, struct reader *r) {
	if (len + 1 > *cap) {
		*cap *= 2;
		set_strlen(str, *cap, r);
	}
}

// Error Handling
void _raise_error(const char *msg, const char *func, const char *file, int line)
{
	fprintf(stderr, "ERROR in %s at %s:%d - %s\n", func, file, line, msg);
	exit(EXIT_FAILURE);
}

void _raise_syntax_error(const char *msg, const char *func, const char *file, int line, struct reader *r)
{
	if (r) {
		fprintf(stderr, "ERROR with file %s in %s at %s:%d - %s\n", r->filename, func, file, line, msg);
		killReader(r);
	} else {
		fprintf(stderr, "ERROR in %s at %s:%d - %s\n", func, file, line, msg);
	}
	_raise_error(msg, func, file, line);
	exit(EXIT_FAILURE);
}

void _raise_semantic_error(const char *msg, const char *func, const char *file, int line, struct env *env) {
	free_env(env);
	_raise_error(msg, func, file, line);
}