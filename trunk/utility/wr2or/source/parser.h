#ifndef __PARSER_H
#define __PARSER_H

#include "scanner.h"

extern token_class tc;
extern char buf[];

extern void getNext();

void doScript();

extern void doExp();
extern void doEmptyLine();
extern void doEOL();
extern void doComma();
extern void doString();

#endif
