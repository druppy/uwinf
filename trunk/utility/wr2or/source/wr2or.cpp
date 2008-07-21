#include "wr2or.h"
#include "parser.h"
#include "output.h"

#include "bfile.h"

#include "default.h"

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include "error.h"


#define PROGNAME "wr2or"

void showBanner() {
	printf("\n%s v1.1 - Windows .RC to OS/2 .RC converter\n",PROGNAME);
}

void showUsage() {
	showBanner();
	printf("usage: %s <inputfile> <outputfile> [options]\n",PROGNAME);
	printf(" <inputfile>      Windows RC type file\n");
	printf(" <outputfile>     OS/2 RC output file\n");
	printf(" -eo <errorfile>  Put errors/warnings/notes into <errorfile>\n");
	printf(" -q               Quiet mode\n");
	printf(" -v               Verbose mode\n");
	printf(" -w0              Warning level 0. Only Errors\n");
	printf(" -w1              Warning level 1. errors+warnings\n");
	printf(" -w3              Warning level 3. errors+warnings+ignored\n");
	printf(" -w4              Warning level 4. errors+warnings+ignored+notes\n");
	printf(" -usage           Display this text\n");
	exit(EX_USAGE);
}


//----------- global variables ----------------------------
//toggles
int displayWarnings=1;
int displayIgnoredThings=1;
int displayNotes=1;

int warnings=0, ignoredThings=0, notes=0;	//count


BufferedFile *fp=0;
FILE *ofp=0;
FILE *efp=0;


static const char *inputFilename=0;
static const char *outputFilename=0;
static const char *errorFilename=0;
static int verbose=1;

void parseArguments(int argc, char *argv[]) {
	if(argc<2) showUsage();

	for(int i=1; i<argc; i++) {
		const char *arg=argv[i];
		if(arg[0]=='-' || arg[0]=='/') {
			const char *option=arg+1;
			if(strcmp(option,"usage")==0) {
				showUsage();
			} else if(strcmp(option,"eo")==0) {
				if(i>=argc-1)
					ShowError(EX_USAGE,"%s: -eo is missing a file argument\n",PROGNAME);
				i++;
				errorFilename=argv[i];
			} else if(strcmp(option,"q")==0) {
				verbose=0;
			} else if(strcmp(option,"v")==0) {
				verbose=1;
			} else if(strcmp(option,"w0")==0) {
				displayWarnings=displayIgnoredThings=displayNotes=0;
			} else if(strcmp(option,"w1")==0) {
				displayIgnoredThings=displayNotes=0;
				displayWarnings=1;
			} else if(strcmp(option,"w2")==0) {
				displayWarnings=displayIgnoredThings=1;
				displayNotes=0;
			} else if(strcmp(option,"v")==0) {
				displayWarnings=displayIgnoredThings=displayNotes=1;
			} else
				ShowError(EX_USAGE,"%s: unknown option '%s'\n",PROGNAME,arg);
		} else {
			if(inputFilename==0)
				inputFilename=arg;
			else if(outputFilename==0)
				outputFilename=arg;
			else
				ShowError(EX_USAGE,"%s: what is '%s' supposed to do?\n",PROGNAME,arg);
		}
	}
	if(inputFilename==0)
		ShowError(EX_USAGE,"%s: No input file specified\n",PROGNAME);
	if(outputFilename==0)
		ShowError(EX_USAGE,"%s: No output file specified\n",PROGNAME);
}


static int nargc;
static char *nargv[30];


void doDefaultArgThing(int oargc, char *oargv[]) {
	for(int i=0; i<=oargc; i++)
		nargv[i]=oargv[i];
	nargc=oargc-1;
	if(scanDefaultsFile(PROGNAME,&nargc,nargv+1,30)!=1)
		ShowError(EX_MEM,"%s: Out of memory while scanning defaults option file\n",PROGNAME);
	nargc++;
}


int main(int oargc, char *oargv[]) {
	//argument stuff

	doDefaultArgThing(oargc,oargv);

	if(nargc<3) showUsage();

	parseArguments(nargc,nargv);

	if(verbose) showBanner();


	// file setup

	fp= new BufferedFile(inputFilename);
	if(!fp)
		ShowError(EX_MEM,"%s: out of memory\n",PROGNAME);
	if(!*fp)
		ShowError(EX_NOINPUT,"%s: '%s' could not be opened\n",PROGNAME,inputFilename);

	ofp=fopen(outputFilename,"wt");
	if(ofp==0)
		ShowError(EX_CANTCREAT,"%s: '%s' could not be created\n",PROGNAME,outputFilename);

	if(errorFilename) {
		efp=fopen(errorFilename,"wt");
		if(efp==0)
			ShowError(EX_CANTCREAT,"%s: '%s' could not be created\n",PROGNAME,errorFilename);
	}


	//do the translation
	output("#include <pm.h>\n");

	getNext();
	doScript();


	//clean up
	delete fp; fp=0;

	if(fclose(ofp)==EOF)
		ShowError(EX_IOERR,"%s: could not close '%s'\n",PROGNAME,outputFilename);

	if(efp) {
		if(fclose(efp)==EOF)
			ShowError(EX_IOERR,"%s: could not close '%s'\n",PROGNAME,errorFilename);
		//delete error output file if nothing has been written to it
		if(warnings+ignoredThings+notes==0)
			unlink(errorFilename);
	}


	//show some statistics
	if(verbose) {
		printf("Warnings: %d  Ignored: %d  Notes %d\n",warnings,ignoredThings,notes);
	}

	return 0;
}
