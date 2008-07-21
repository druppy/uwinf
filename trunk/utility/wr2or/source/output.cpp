#include "output.h"
#include "parser.h"
#include "wr2or.h"

#include <stdio.h>
#include "error.h"

// This module handles output to the OS/2 resource script file

static int outputDisabled=0;
void disableOutput() {
	outputDisabled++;
}
void enableOutput() {
	outputDisabled--;
}


void output(const char *s) {
	if(outputDisabled) return;
	if(fprintf(ofp,"%s",s)==EOF) {
		ShowError(EX_IOERR,"Write error\n");
	}
}

void output(char c) {
	if(outputDisabled) return;
	if(putc(c,ofp)==EOF) {
		ShowError(EX_IOERR,"Write error\n");
	}
}

void output(int i) {
	if(outputDisabled) return;
	if(fprintf(ofp,"%d",i)==EOF) {
		ShowError(EX_IOERR,"Write error\n");
	}
}

void output() {
	//output current token
	if(outputDisabled) return;
	output(buf);
}
