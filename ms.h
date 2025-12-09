#ifndef MS_H
#define MS_H

typedef struct MS_Exp_t {
    struct MS_Exp_t *next;
    int val;
    char op;
} MS_Exp;

extern const char *MS_OPS;

void free_exp(MS_Exp *exp);
void print_exp(MS_Exp *exp);
MS_Exp *init_exp();
MS_Exp *parse_file(char *filename);


#endif //MS_H
