#include "pcommon.h"
#include "parser.h"
#include "output.h"
#include "oops.h"

#include <string.h>

/* This module contains parsing shared by several modules */

/***********************************************************************/
// begin/end/{/}/indentation stuff
// We allow both BEGIN/END blocks as well as brace blocks

static int bracesUsed=0;
static int indent=0;

void outputIndent() {
        int i=indent*2;
        while(i--) output(' ');
}

void doBEGIN() {
        if(tc!=tc_literal || strcmp(buf,"BEGIN")!=0) {
                if(tc==tc_unknown && buf[0]=='{')
                        bracesUsed=1;
                else
                        doUnexpected("BEGIN");
        }

        outputIndent();
        output("BEGIN");
        getNext();
        indent++;
}

void doEND() {
        if(tc!=tc_literal || strcmp(buf,"END")!=0) {
                if(!bracesUsed || (tc!=tc_unknown || buf[0]!='}'))
                        doUnexpected("END");
        }

        indent--;
        outputIndent();
        output("END");
        getNext();
}

int isBEGIN() {
        if(tc==tc_literal && strcmp(buf,"BEGIN")==0) return 1;
        if(tc==tc_unknown && buf[0]=='{') {
                bracesUsed=1;
                return 1;
        }
        return 0;
}

int isEND() {
        if(tc==tc_literal && strcmp(buf,"END")==0) return 1;
        if(bracesUsed && tc==tc_unknown && buf[0]=='}') return 1;
        return 0;
}


void outputBEGIN() {
        outputIndent();
        output("BEGIN\n");
        indent++;
}

void outputEND() {
        indent--;
        outputIndent();
        output("END\n");
}

/************ loadtype and memory options ********************************/
void doLoadAndMemoryOption() {
        int first=1;
        while(tc==tc_literal) {
                if(first) {
                        output(' ');
                        first=0;
                }
                if(stricmp(buf,"DISCARDABLE")==0) {
                        output("DISCARDABLE ");
                } else if(stricmp(buf,"FIXED")==0) {
                        output("FIXED ");
                } else if(stricmp(buf,"IMPURE")==0) {
                        displayWarning("IMPURE is not supported by OS/2");
                } else if(stricmp(buf,"MOVEABLE")==0) {
                        output("MOVEABLE ");
                } else if(stricmp(buf,"NONDISCARDABLE")==0) {
                        displayIgnored("NONDISCARDABLE is ignored");
                } else if(stricmp(buf,"PURE")==0) {
                        displayIgnored("PURE is ignored");
                } else if(stricmp(buf,"PRELOAD")==0) {
                        output("PRELOAD ");
                } else if(stricmp(buf,"LOADONCALL")==0) {
                        output("LOADONCALL ");
                } else
                        break;
                getNext();
        }
}


/********** filename parsing *******************************************/

void doFilename() {
        if(tc==tc_string) {
                //string name -> name
                output(' ');
                for(char *p=buf; *p; p++) {
                        if(*p!='"') output(*p);
                }
                getNext();
        } else {
                output();
                getNext();
                if(tc==tc_unknown && buf[0]=='.') {
                        //handle file name extension
                        output();       //output .
                        getNext();      //get extension
                        output();       //put extension
                        getNext();      
                }
        }
        doEOL();
}

void doFilename(const char *oldext, const char *newext) {
        //handle a filename replacing old extension with new extension
        char name[80];
        char *d=name;

        if(tc==tc_string) {
                //string name -> name
                for(char *p=buf; *p; p++) {
                        if(*p!='"') *d++=*p;
                }
                *d='\0';
                getNext();
        } else {
                strcpy(name,buf);
                getNext();
                if(tc==tc_unknown && buf[0]=='.') {
                        //handle file name extension
                        strcat(name,buf);
                        getNext();
                        strcat(name,buf);
                }
        }

        char *dp=strchr(name,'.');
        if(dp) {
                if(stricmp(dp,oldext)==0)
                        strcpy(dp,newext);
        }

        output(name);
        doEOL();
}
