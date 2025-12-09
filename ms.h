#ifndef MS_H
#define MS_H

typedef struct MS_Exp_t {
    struct MS_Exp_t *next;
    int num1;
    char op;
} MS_Exp;

extern const char *MS_OPS;

void free_exp(MS_Exp *exp);
void print_exp(MS_Exp *exp);
MS_Exp *parse(char *input_buff);


#endif //MS_H
