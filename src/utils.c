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

#define ANSI_RED     "\x1b[31m"
#define ANSI_BOLD    "\x1b[1m"
#define ANSI_RESET   "\x1b[0m"
#define ANSI_CYAN    "\x1b[36m"

static const char *ERROR_MESSAGES[] =  { \
	[ERR_OK] = "ERR_OK: no error", \
	[ERR_NO_FILE] = "ERR_NO_FILE: failed to open file", \
	[ERR_EOF] = "ERR_EOF: unexpected end of file", \
	[ERR_NO_ARGS] = "ERR_NO_ARGS: expected arguments (i.e. \"./bfCompiler tests/array1.txt)\"", \
	[ERR_NO_MEM] = "ERR_NO_MEM: memory allocation failed", \
	[ERR_REFREE] = "ERR_REFREE: Attempted to free already freed memory", \
	[ERR_UNEXP_CHAR] = "ERR_UNEXP_CHAR: unexpected character", \
	[ERR_INV_ESC] = "ERR_INV_ESC: invalid escape sequence", \
	[ERR_UNMATCHED_BRACKET] = "ERR_UNMATCHED_BRACKET: Missing closing bracket", \
	[ERR_UNMATCHED_PAREN] = "ERR_UNMATCHED_PAREN: Missing closing parenthesis", \
	[ERR_UNMATCHED_BRACE] = "ERR_UNMATCHED_BRACE: Missing closing brace", \
	[ERR_UNMATCHED_QUOTE] = "ERR_UNMATCHED_QUOTE: Missing closing quotation mark", \
	[ERR_BIG_NUM] = "ERR_BIG_NUM: number exceeds maximum length", \
	[ERR_TOO_LONG] = "ERR_TOO_LONG: string exceeds maximum length", \
	[ERR_INV_TYPE] = "ERR_INV_TYPE: invalid or unexpected type", \
	[ERR_INV_VAL] = "ERR_INV_VAR: invalid value", \
	[ERR_INV_OP] = "ERR_INV_OP: invalid operator", \
	[ERR_INV_EXP] = "ERR_INV_EXP: invalid expression", \
	[ERR_INV_STMT] = "ERR_INV_STMT: invalid statement", \
	[ERR_BAD_ELSE] = "ERR_BAD_ELSE: malformed else clause", \
	[ERR_REDEF] = "ERR_REDEF: Variable already declared in this scope", \
	[ERR_NO_VAR] = "ERR_NO_VAR: undefined variable", \
	[ERR_IMMUT] = "ERR_IMMUT: Cannot modify immutable variable (declared with 'val')", \
	[ERR_INV_ARR] = "ERR_INV_ARR: invalid use of array", \
	[ERR_INF_REC] = "ERR_INF_REC: infinite recursion detected in statement", \
	[ERR_INTERNAL] = "ERR_INTERNAL: internal compiler error" \
};

static void default_error_exit(enum err_type err_code) {
	(void)err_code; // Unused in default implementation
	exit(EXIT_FAILURE);
}

// Function pointer that can be overridden for testing
void (*error_exit_handler)(enum err_type err_code) = default_error_exit;

static const char *get_error_message(enum err_type err_code) {
    if (err_code >= 0 && err_code < sizeof(ERROR_MESSAGES)/sizeof(ERROR_MESSAGES[0])) {
        return ERROR_MESSAGES[err_code];
    }
    return "Unknown error";
}

void set_strlen(char **str, const int len, struct reader *r) {
	char *temp = realloc(*str, len);
	if (!temp) {
		free(*str);
		raise_syntax_error(ERR_NO_MEM, r);
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
void _raise_error(enum err_type err, const char *func, const char *file, int line)
{

	fprintf(stderr, "ERROR in %s at %s:%d - %s\n", func, file, line, get_error_message(err));
	error_exit_handler(err);
}


void _raise_syntax_error(enum err_type err, const char *func, const char *file, int line, struct reader *r) {
	if (r) {
		int error_start = r->val.start_pos;
		int error_end = r->line_pos;
		int error_len = error_end - error_start;
		
		if (error_len < 1) {
		error_len = 1;
		error_start = r->line_pos;
		}
		
		// Print error header
		fprintf(stderr, ANSI_BOLD "Error" ANSI_RESET " in %s" ANSI_CYAN ":%d:%d" ANSI_RESET "\n", 
			r->filename, r->line_num, error_start + 1);
		fprintf(stderr, "  " ANSI_RED "%s" ANSI_RESET "\n", get_error_message(err));
		
		// Print the source line with line number
		if (r->line_buf && r->line_buf[0]) {
		fprintf(stderr, "\n");
		
		// Print line number in cyan
		fprintf(stderr, ANSI_CYAN "%5d | " ANSI_RESET, r->line_num);
		
		// Print the line
		for (int i = 0; r->line_buf[i] && r->line_buf[i] != '\n'; i++) {
			fputc(r->line_buf[i], stderr);
		}
		fprintf(stderr, "\n");
		
		// Print underline with proper offset for line number
		fprintf(stderr, "      | ");
		for (int i = 0; i < error_start; i++) {
			fputc(r->line_buf[i] == '\t' ? '\t' : ' ', stderr);
		}
		
		// Print underline in bold red
		fprintf(stderr, ANSI_BOLD ANSI_RED);
		for (int i = 0; i < error_len; i++) {
			fputc('^', stderr);
		}
		fprintf(stderr, ANSI_RESET "\n");
		}
		
		fprintf(stderr, "\n");
		killReader(r);
	} else {
		fprintf(stderr, "ERROR in %s at %s:%d - %s\n", func, file, line, get_error_message(err));
	}
	error_exit_handler(err);
}



static char *get_line_at_pos(const char *filename, int line_num) {
    FILE *fp = fopen(filename, "r");
    if (!fp)
        return NULL;
    
    // Read the line
    char *line_buf = calloc(256 + 1, sizeof(*line_buf));
    if (!line_buf) {	
        fclose(fp);
        return NULL;
    }
    
    for (int i = 1; i <= line_num; i++) {
	if (!fgets(line_buf, 255, fp)) {
		free(line_buf);
		fclose(fp);
		return NULL;
	}
    }
    if (!line_buf) {
    	fclose(fp);
	return NULL;
    }

    line_buf[254] = '\n';
    line_buf[255] = '\0';
    

    char *tmp = realloc(line_buf, strlen(line_buf) + 2);
   
    fclose(fp);

     if (!tmp)
    	return line_buf;
    line_buf = tmp;
    size_t len = strlen(line_buf);
    line_buf[len] = '\n';
    line_buf[len + 1] = '\0';
    return line_buf;
}


static void _raise_semantic_error(enum err_type err, int line_num, int start_col, const char *func, const char *file, int line, struct env *env) {
	fprintf(stderr, "ERROR in %s at %s:%d - %s\n", func, file, line, ERROR_MESSAGES[err]);
	if (env->filename) {
		// Retrieve the source line using fpos_t
		char *source_line = get_line_at_pos(env->filename, line_num);
		size_t len = strlen(source_line);
		if (strlen(source_line) > 0) {
			fprintf(stderr, "\n  ");
			 // invalid read
			
			// Print line without trailing newline
			for (size_t i = 0; (i < len) && source_line[i] && (source_line[i] != '\n'); i++)
				fputc(source_line[i], stderr);
			fprintf(stderr, "\n  ");
			
			// Print indicator
			bool right_start_pos = start_col <= (int) strlen(source_line);
			for (int i = 0; right_start_pos && i < start_col; i++)
				fputc(source_line[i] == '\t' ? '\t' : ' ', stderr);

			fprintf(stderr, "^\n");
			
			free(source_line);
		}
		
		fprintf(stderr, "\n");
	} else {
		_raise_error(err, func, file, line);
	}
	
	free_env(env);
	error_exit_handler(err);
}

void _raise_exp_semantic_error(enum err_type err, const struct exp *exp, const char *func, const char *file, int line, struct env *env) {
	_raise_semantic_error(err, exp->line_num, exp->start_col, func, file, line, env);
}

void _raise_stmt_semantic_error(enum err_type err, const struct stmt *stmt, const char *func, const char *file, int line, struct env *env) {
	_raise_semantic_error(err, stmt->line_num, stmt->start_col, func, file, line, env);
}