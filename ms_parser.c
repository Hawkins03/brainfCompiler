#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms.h"
#include "bf.h"

const char *MS_OPS = "+-*/%";

void free_data(MS_Exp *d) {
    if (d == 0) return;
    free_data(d->next);
    free(d);
}

void print_exp(MS_Exp *exp) {
    if (exp == 0) return;
    if (exp == exp->next) return; // TODO: better detect loops. (i.e. don't do it recursivly and in the loop check for the origional each time)
    print_exp(exp->next);
    printf("%d '%c' %d\n", exp->num1, exp->op, exp->num2);
}

MS_Exp *parse(char *input_buff) {
    MS_Exp *init_exp(MS_Exp *exp) {
	if (exp == NULL) return exp;
	exp->num1 = 0;
	exp->op = ' ';
	exp->num2 = 0;
	exp->next = 0;
    return exp;
    }
    int len = strlen(input_buff);
    if (len <= 0)
	return 0;
    MS_Exp *d = init_exp((MS_Exp *) malloc(sizeof(MS_Exp)));
    if (d == NULL) return 0;
    //char bf_string[BUFF_SIZE];
    for (int i = 0; i < len; i++) {
	while (isdigit(input_buff[i]) && i < strlen(input_buff)) {
	    d->num1 = (d->num1 * 10) + input_buff[i++] - '0';
	}
	if (strchr(MS_OPS, input_buff[i]) != NULL) {
	    if (d->op != 0) {
		d->op = input_buff[i++];
		while (isdigit(input_buff[i]) && i < strlen(input_buff)) {
		    d->num2 = (d->num2 * 10) + input_buff[i++] - '0';
		}
	    }
		
	}
    }
    return d;
}
