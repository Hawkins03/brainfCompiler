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
    r->alive = true;
    r->fp = fopen(filename, "r");
    if (!r->fp) {
	free(r);
	raise_error("Error, bad file");
    } else if (feof(r->fp))
	raise_error_and_free("Error, EOF on file open", r);
    r->curr = fgetc(r->fp);
    return r;
}

char peek(Reader *r) {
    if (r->curr == EOF) raise_error("ERROR, peek read EOF");
    return r->curr;
}
char advance(Reader *r) {
    if (!isAlive(r)) raise_error("ERROR, advance when Reader dead");
    char out = r->curr;
    r->curr = fgetc(r->fp);
    if (feof(r->fp)) {
	killReader(r);
    }
    return out;
}

void accept(char ch1, char ch2, char *message) {
    if (ch1 != ch2) raise_error(message);
}

void skip_spaces(Reader *r) {
    char *spaces = " \n\t";
    if (strchr(spaces, peek(r)))
	advance(r);
}

int getNextNum(Reader *r) {
    if (!isAlive(r)) raise_error("Error, getting num when Reader dead");
    int num = 0;
    while (isdigit(peek(r))) {
	num = (num * 10) + advance(r) - '0';
    }
    return num;
}

void killReader(Reader *r) {
    if (!r || !r->alive) raise_error("Error, double freeing reader");
    r->alive = false;
    fclose(r->fp);
    r->fp = NULL;
    free(r);
}
bool isAlive(Reader *r) {
    return r->alive;
}

void raise_error(char *error_message) {
    perror(error_message);
    fprintf(stderr, "%s at %s:%d\n", strerror(errno), __FILE__, __LINE__);
    exit(EXIT_FAILURE);
}

void raise_error_and_free(char *error_message, Reader *r) {
    killReader(r);
    raise_error(error_message);
}
