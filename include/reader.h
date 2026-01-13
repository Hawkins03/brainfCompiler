#ifndef READER_H
#define READER_H

#include "structs.h"

bool readerIsAlive(Reader *r);
Reader *readInFile(const char *filename);
void killReader(Reader *r);

int peek(Reader *r);
int advance(Reader *r);
void skip_spaces(Reader *r);

int getNextNum(Reader *r);
char *getNextWord(Reader *r);
key_t getKeyType(char *keyword);
const char *getKeyStr(key_t key);
char *getNextOp(Reader *r);
int getCharacterValue(Reader *r); // i.e. for the input " 'x'; " it returns the ascii value of x.
char getNextDelim(Reader *r);
value_t *getRawToken(Reader *r);

value_t *getToken(Reader *r);
value_t *peekToken(Reader *r);
void acceptToken(Reader *r, value_type_t type, const char *expected);

bool hasNextStmt(Reader *r);
bool atSemicolon(Reader *r);

#endif //READER_H