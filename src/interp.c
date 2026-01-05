#include <stdio.h>
#include <string.h>
#include "interp.h"

void interp(char *input_buff) {
    int len = strlen(input_buff);
    int buff[BUFF_SIZE] = {0};
    int *curr_ptr = buff;
    int i = 0;
    

    int bracket_index = -1;
    int num_open_brackets = 0;
    int num_closed_brackets = 0;
    for (int i = 0; i < len; i++) {
	switch (input_buff[i]) {
	    case '[':
		num_open_brackets++;
		break;
	    case ']':
		num_closed_brackets++;
	}
    }
    if (num_open_brackets != num_closed_brackets) {
	return;
    }

    int bracket_pairs[2][num_open_brackets];


    int open_index = 0;
    int closed_index = 0;
    for (int i = 0; i < len; i++) {
		switch (input_buff[i]) {
			case '[':
				bracket_pairs[0][open_index++] = i;
				break;
			case ']':
				bracket_pairs[1][closed_index++] = i;
				break;
		}
    }

    
    //interpret the input
    while (i < len) {
		switch (input_buff[i]) {
			case '>':
				curr_ptr += 1;
				if (curr_ptr >= buff + BUFF_SIZE)
					curr_ptr = buff;
				break;
			case '<':
				curr_ptr -= 1;
				if (curr_ptr < buff)
					curr_ptr = buff + BUFF_SIZE - 1;
				break;
			case '+':
				*curr_ptr += 1;
				break;
			case '-':
				*curr_ptr -= 1;
				break;
			case '.':
				printf("%c", (*curr_ptr & 0x000000ff));
				break;
			case ',':
				*curr_ptr = getchar();
				break;
			case '[': 
				bracket_index++;
				if ((*curr_ptr) == 0)
					i = bracket_pairs[1][bracket_index];
				break;
			case ']':
				bracket_index--;
				if ((*curr_ptr) != 0)
					i = bracket_pairs[0][bracket_index]; 
			break;
				default:
	}
	i++;
    }

	printf("\nFinal Stack State:\n");
    printf("-1, %ld: %hhu\n", curr_ptr - buff, *curr_ptr);
    for (int i = 0; i < BUFF_SIZE; i++) {
		printf("%d[%hhu] ", i, buff[i]);
    }
    printf("\n");
}
