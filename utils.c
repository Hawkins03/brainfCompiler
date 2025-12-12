#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bf.h"
#include "ms.h"
#include "utils.h"


Reader *readInFile(char *filename) {
    if (filename == NULL)
	raise_error("Error, bad filename");
    Reader *r = (Reader *) malloc(sizeof(Reader));
    if (!r)
	raise_error("Error, bad Reader malloc");
    r->fp = fopen(filename, "r");
    if (!r->fp) {
	free(r);
	raise_error("Error, bad file");
    } else if (feof(r->fp))
	raise_error_and_free("Error, EOF on file open", r);
    fseek(r->fp, 0L, SEEK_END);
    r->chars_left = ftell(r->fp);
    fseek(r->fp, 0L, SEEK_SET);
    r->curr = fgetc(r->fp);
    return r;
}

char peek(Reader *r) {
    if (r->curr == EOF) raise_error("ERROR, peek read EOF");
    return r->curr;
}
char advance(Reader *r) {
    if (r->chars_left-- <= 1) {
    	//killReader(r);//raise_error("ERROR, advance when Reader died");
	return '\0';
    }
    char out = r->curr;
    //printf("out '%c', %p\n", out, r->fp);
    if (r->fp && !feof(r->fp)) {
	r->curr = fgetc(r->fp);
    }
    if (!r->fp)
	killReader(r);
    if (feof(r->fp))
	killReader(r);
    return out;
}

void accept(char ch1, char ch2, char *message) {
    if (ch1 != ch2) raise_error(message);
}

void skip_spaces(Reader *r) {
    if (!isAlive(r)) return;
    while (r->fp && !feof(r->fp)) {
	//printf("skipping a space\n");
	if (!isspace(peek(r)))
	    return;
	if (advance(r) == '\0') return;
    }
}

int getNextNum(Reader *r) {
    if (!isAlive(r)) raise_error("Error, getting num when Reader died");
    int num = 0;
    while (isdigit(peek(r))) {
	num = (num * 10) + advance(r) - '0';
    }
    //printf("num = %d\n", num);
    return num;
}

char *getNextWord(Reader *r) {
    if (!isAlive(r)) raise_error("Error, getting word when Reader died");
     
    skip_spaces(r);
    //printf("%c\n", peek(r));

    if (!isalpha(peek(r))) raise_error("Invallid first letter in word (first letter must be in the range 'a-zA-Z')");
    char chars[BUFF_SIZE] = {0};
    int len = 0;

    for (int i = 0; i < BUFF_SIZE; i++) {
	if (isalnum(peek(r)) || (peek(r) == '_'))
	    chars[len++] = peek(r);
	else
	    break;

	advance(r);
    }

    char *word = (char *) calloc(len, sizeof(char));
    strncpy(word, chars, len);
    return word;
}

bool isAlive(Reader *r) {
    return r->chars_left > 0;
}

void killReader(Reader *r) {
    //printf("killing now\n");
    if (!r || (r->chars_left > 0)) raise_error("Error, double freeing reader");
    r->chars_left = 0;
    fclose(r->fp);
    r->fp = NULL;
    free(r);
}

void raise_error(char *error_message) {
    perror(error_message);
    exit(EXIT_FAILURE);
}

void raise_error_and_free(char *error_message, Reader *r) {
    killReader(r);
    raise_error(error_message);
}
