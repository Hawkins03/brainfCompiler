#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "utils.h"
#include "stmt.h"
#include "exp.h"
#include "parser.h"

exp_t *parse_atom(Reader *r);
exp_t *parse_exp(int minPrio, Reader *r);
void parse_single_stmt(Reader *r);
void parse_stmt(Reader *r);
stmt_t *parse_file(const char *filename);


#endif //PARSER_H