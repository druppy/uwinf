#ifndef __OOPS_H
#define __OOPS_H

extern void doUnexpected();
extern void doUnexpected(const char *what);
extern void displayWarning(const char *w);
extern void displayIgnored(const char *i);
extern void displayNote(const char *n);

#endif
