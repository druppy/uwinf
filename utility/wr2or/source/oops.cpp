#include "oops.h"
#include "wr2or.h"
#include "bfile.h"
#include "parser.h"

#include <stdio.h>
#include "error.h"

// This module handles output of warnings, notes and error messages

void doUnexpected() {
	doUnexpected(0);
}

static void doUnexpected(FILE *efp, const char *what) {
	fprintf(efp,"Line %d: ",fp->lineno());
	switch(tc) {
		case tc_unknown:
			fprintf(efp,"<unknown,%s>",buf);
			break;
		case tc_string:
			fprintf(efp,"<string,%s>",buf);
			break;
		case tc_comma:
			fprintf(efp,"<comma,%s>",buf);
			break;
		case tc_ppd:
			fprintf(efp,"<ppd,%s>",buf);
			break;
		case tc_commentstart:
			fprintf(efp,"<comment start,%s>",buf);
			break;
		case tc_number:
			fprintf(efp,"<number,%s>",buf);
			break;
		case tc_literal:
			fprintf(efp,"<literal,%s>",buf);
			break;
		case tc_eol:
			fprintf(efp,"<EOL>");
			break;
		case tc_eof:
			fprintf(efp,"<EOF>");
			break;
	}
	fprintf(efp," was found\n");
	if(what)
		fprintf(efp,"%s was expected\n",what);
}

void doUnexpected(const char *what) {
	doUnexpected(stderr,what);
	if(efp) doUnexpected(efp,what);
	ShowError(EX_DATAERR,"");
}



static void displayWarning(FILE *efp,const char *w) {
	fprintf(efp,"Warning. (line %d): %s\n",fp->lineno(),w);
}

void displayWarning(const char *w) {
	if(displayWarnings)
		displayWarning(stderr,w);
	if(efp) displayWarning(efp,w);
	warnings++;
}



static void displayIgnored(FILE *efp, const char *i) {
	fprintf(efp,"Ignored. (line %d): %s\n",fp->lineno(),i);
}

void displayIgnored(const char *i) {
	if(displayIgnoredThings)
		displayIgnored(stderr,i);
	if(efp) displayIgnored(efp,i);
	ignoredThings++;
}


static void displayNote(FILE *efp, const char *n) {
	fprintf(efp,"note (line %d): %s\n",fp->lineno(),n);
}

void displayNote(const char *n) {
	if(displayNotes)
		displayNote(stderr,n);
	if(efp) displayNote(efp,n);
	notes++;
}
