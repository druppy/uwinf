#ifndef __WR2OR_H
#define __WR2OR_H

extern int displayWarnings;
extern int displayIgnoredThings;
extern int displayNotes;

extern int warnings, ignoredThings, notes;


class BufferedFile;
extern BufferedFile *fp;

#include <stdio.h>
extern FILE *ofp;
extern FILE *efp;

#endif
