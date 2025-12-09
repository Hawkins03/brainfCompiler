#include <stdbool.h>
#ifndef MS_H
#define MS_H

struct MS_Exp;


enum atomtype{
    VAL,
    EXP
};

typedef union {
    int val;
    struct MS_Exp *exp;
    enum atomtype type;
} MS_Atom;

typedef struct MS_Exp_t { 
    MS_Atom *val;
    struct MS_Exp_t *next;
    char op;
} MS_Exp;

extern const char *MS_OPS;

void free_exp(MS_Exp *exp);
void print_exp(MS_Exp *exp);
bool isOp(char op);
MS_Exp *init_exp();
MS_Exp *parse_file(char *filename);


#endif //MS_H
