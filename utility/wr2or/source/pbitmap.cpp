#include "pbitmap.h"
#include "parser.h"
#include "pcommon.h"
#include "pinline.h"
#include "output.h"

#include <string.h>

void doBitmapBlock() {
	//save bitmap name in case it is inline
	char resnam[128];
	strcpy(resnam,buf);

	output("BITMAP ");
	doExp();
	getNext();		// eat 'BITMAP'
	doLoadAndMemoryOption();

	if(tc!=tc_eol) {
		doFilename();
	} else {
		doInlineStuff(resnam,".BMP",".BMP");
	}
}
