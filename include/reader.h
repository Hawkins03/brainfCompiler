#ifndef READER_H
#define READER_H

#include "structs.h"

struct reader *readInFile(const char *filename);
void killreader_t(struct reader *r);

bool readerIsAlive(struct reader *r);
bool hasNextStmt(struct reader *r);
bool atSemicolon(struct reader *r);

int peek(struct reader *r);
int advance(struct reader *r);
void skip_spaces(struct reader *r);

char *stealTokString(struct value *tok);
char *stealNextString(struct reader *r);
char getNextDelim(struct reader *r);
char *getNextOp(struct reader *r);
int getNextNum(struct reader *r);
char *getNextWord(struct reader *r);
enum key_type getKeyType(char *keyword);
const char *getKeyStr(enum key_type key);
int getCharacterValue(struct reader *r); // i.e. for the input " 'x'; " it returns the ascii value of x.
char *getNextString(struct reader *r);

#endif //READER_H