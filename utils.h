#ifndef UTILS_H
#define UTILS_H
#define MAX_WORD_LEN 32


typedef struct {
    FILE *fp;
    char curr;
    long chars_left;
} Reader;

//reader struct:
Reader *readInFile(char *filename);
char peek(Reader *r);
char advance(Reader *r);
void accept(char ch1, char ch2, char *message);
void skip_spaces(Reader *r);
int getNextNum(Reader *r);
char *getNextWord(Reader *r);
void killReader(Reader *r);
bool isAlive(Reader *r);

//error printing:
void raise_error(char *error_message);
void raise_error_and_free(char *error_message, Reader *r);
#endif //READER_H
