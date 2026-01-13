#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "utils.h"
#include "stmt.h"
#include "exp.h"
#include "parser.h"

struct exp  *parse_atom(struct reader *r);
struct exp  *parse_exp(int minPrio, struct reader *r);
void parse_single_stmt(struct reader *r);
void parse_stmt(struct reader *r);
struct stmt *parse_file(const char *filename);


#endif //PARSER_H