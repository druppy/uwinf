#include "pstrtab.h"
#include "parser.h"
#include "pcommon.h"
#include "output.h"

/* Windows and OS/2 formats for stringtables are alomost identical except
 * that windows can have string-id and string-expression separated
 * by a comma and OS/2 does not
 */

void doStringtableBody();
void doStringtableEntry();

void doStringtableBlock() {
        //current token is 'STRINGTABLE'
        output();
        getNext();
        doLoadAndMemoryOption();
        doEOL();

        doStringtableBody();
}

void doStringtableBody() {
        doBEGIN(); doEOL();

        while(!isEND()) {
                if(tc==tc_eol) {
                        output();
                        getNext();
                } else if(tc==tc_cppcomment) {
                        output();
                        output("\n");
                        getNext();
                } else
                        doStringtableEntry();
        }

        doEND(); doEOL();

}

void doStringtableEntry() {
        outputIndent();

        doExp();

        if(tc==tc_comma) {
                disableOutput();
                doComma();
                enableOutput();
        }

        output('\t');

        doString();

        doEOL();
}
