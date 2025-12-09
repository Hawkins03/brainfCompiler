#ifndef MS_H
#define MS_H

typedef struct MS_Exp_t {
    int num1;
    int num2;
    char op;
    struct MS_Exp_t *next;
} MS_Exp;

extern const char *MS_OPS;

void free_data(MS_Exp *exp);
void print_exp(MS_Exp *exp);
MS_Exp *parse(char *input_buff);


#endif //MS_H
