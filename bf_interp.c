#include <stdio.h>
#include <string.h>
#include "bf.h"

void bf_interpret(char *input_buff) {
    if (strlen(input_buff) == -1) return;
    int len = strlen(input_buff);
    char buff[BUFF_SIZE] = {0};
    int bracket_ptrs[BUFF_SIZE/2];
    int bracket_index = -1;
    char *curr_ptr = buff;
    int i = 0;
    while (i < len) {
	//printf("%d, %c, %ld: %hhu\n", i, input_buff[i], curr_ptr - buff, *curr_ptr); // %hhu is the proper way to print a char as an integer, being respectful of it's size and all.
	switch (input_buff[i]) {
	    case '>':
		curr_ptr += 1;
		if (curr_ptr >= buff + BUFF_SIZE) {
		    curr_ptr = buff;
		}
		break;
	    case '<':
		curr_ptr -= 1;
		if (curr_ptr < buff) {
		    curr_ptr = buff + BUFF_SIZE - 1;
		}
		break;
	    case '+':
		*curr_ptr += 1;
		break;
	    case '-':
		*curr_ptr -= 1;
		break;
	    case '.':
		printf("%c", *curr_ptr);
		break;
	    case ',':
		*curr_ptr = getchar();
	    case '[': 
		bracket_ptrs[++bracket_index] = i - 1;
		if ((*curr_ptr) == 0) {
		    //printf("swapping!\n"); 
		    curr_ptr = strchr(curr_ptr, ']') + 1;
		}
		break;
	    case ']':
		if ((*curr_ptr) != 0) {
		    //printf("bracket index: %d\n", bracket_index);
		    if (bracket_index >= 0) { //TODO: throw an error if false
			//printf("moving back\n");
			i = bracket_ptrs[bracket_index--];
		    }
		}
		break;
	    default:
		
	}
	i++;
    }
    printf("-1, %ld: %hhu\n", curr_ptr - buff, *curr_ptr);
    /*for (int i = 0; i < BUFF_SIZE; i++) {
	printf("%d[%hhu] ", i, buff[i]);
    }*/
    printf("\n");
}
