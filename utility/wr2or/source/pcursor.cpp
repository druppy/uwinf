#include "pcursor.h"
#include "parser.h"
#include "pcommon.h"
#include "pinline.h"
#include "output.h"

#include <string.h>

void doCursorBlock() {
	//save bitmap name in case it is inline
	char resnam[128];
	strcpy(resnam,buf);

	output("POINTER ");
	doExp();
	getNext();		// eat 'ICON'
	doLoadAndMemoryOption();

	if(tc!=tc_eol) {
		output(' ');
		doFilename(".cur",".ptr");
	} else {
		doInlineStuff(resnam,".PTR",".CUR");
	}
}
