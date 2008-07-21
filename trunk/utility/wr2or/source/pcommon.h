#ifndef __PCOMMON_H
#define __PCOMMON_H

void outputIndent();

void doBEGIN();
void doEND();
int isBEGIN();
int isEND();
void outputBEGIN();
void outputEND();


void doLoadAndMemoryOption();

void doFilename();
void doFilename(const char *oldext, const char *newext);

#endif
