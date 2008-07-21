#include "picon.h"
#include "parser.h"
#include "pcommon.h"
#include "pinline.h"
#include "output.h"

#include <string.h>

void doIconBlock() {
        //save bitmap name in case it is inline
        char resnam[128];
        strcpy(resnam,buf);

        output("ICON ");
        doExp();
        getNext();                // eat 'ICON'
        doLoadAndMemoryOption();

        if(tc!=tc_eol) {
                doFilename();
        } else {
                doInlineStuff(resnam,".ICO",".ICO");
        }
}
