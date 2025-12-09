#ifndef UTILS_H
#define UTILS_H

typedef struct {
    FILE *fp;
    char curr;
    bool alive;
} Reader;


//reader struct:
Reader *readInFile(char *filename);
char peek(Reader *r);
char advance(Reader *r);
int getNextNum(Reader *r);
bool isOp(char op);
bool isAlive(Reader *r);
void killReader(Reader *r);

//error printing:
void raise_error(char *error_message);
void raise_error_and_free(char *error_message, Reader *r);
#endif //READER_H
