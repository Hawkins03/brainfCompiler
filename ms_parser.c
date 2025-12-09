#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms.h"
#include "bf.h"

const char *MS_OPS = "+-*/%";

void free_exp(MS_Exp *exp) {
    if (exp == 0) return;
    free_exp(exp->next);
    free(exp);
}

void print_exp(MS_Exp *exp) {
    if (exp == 0) return;
    if (exp == exp->next) return; // TODO: better detect loops. (i.e. don't do it recursivly and in the loop check for the origional each time) 
    printf("%d '%c' ", exp->num1, exp->op);
    print_exp(exp->next);
}

MS_Exp *parse(char *input_buff) {
    MS_Exp *init_exp(MS_Exp *exp) {
	if (exp == NULL) return exp;
	exp->num1 = 0;
	exp->op = ' ';
	exp->next = 0;
    return exp;
    }
    int len = strlen(input_buff);
    if (len <= 0)
	return 0;
    MS_Exp *ret_exp = init_exp((MS_Exp *) malloc(sizeof(MS_Exp)));
    MS_Exp *curr_exp = ret_exp;
    if (curr_exp == NULL) return 0;
    //char bf_string[BUFF_SIZE];
    for (int i = 0; i < len; i++) {
	while (isdigit(input_buff[i]) && i < strlen(input_buff)) {
	    curr_exp->num1 = (curr_exp->num1 * 10) + input_buff[i++] - '0';
	}
	if ((strchr(MS_OPS, input_buff[i]) != NULL) && (i != len)) {
	    curr_exp->op = input_buff[i];
	    //create next expression
	    curr_exp->next = init_exp((MS_Exp *) malloc(sizeof(MS_Exp)));
	    if (!curr_exp->next) return curr_exp;
	    curr_exp = curr_exp->next;
	}
    }
    return ret_exp;
}
