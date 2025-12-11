#include <stdbool.h>
#include "utils.h"
#ifndef MS_H
#define MS_H

struct MS_Exp;


/*typedef enum {
    EMPTY,
    VAL,
    EXP
} Type;*/

typedef struct { //note, holds a val or an exp. (TODO: make into a union w/ type)
    int val;
    struct MS_Exp *exp;
} MS_Atom;

typedef struct MS_Exp_t { 
    MS_Atom *left;
    MS_Atom *right;
    char op;
    struct MS_Exp_t *prev; //needed to fix w*x*y+z issue (should be ((w*x)*y) + z, not w*(x*(y+z)))
} MS_Exp;

extern const char *MS_OPS;

void free_atom(MS_Atom *atom);
void free_exp(MS_Exp *exp);
void print_full_exp(MS_Atom *atom);
void print_atom(MS_Atom *atom);
void print_exp(MS_Exp *exp);
bool isOp(char op);
MS_Exp *init_exp();
MS_Atom *parse_atom(Reader *r);
MS_Atom *parse_exp(int minPrio, Reader *r);
MS_Atom *parse_file(char *filename);


#endif //MS_H
