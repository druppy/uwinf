#include "statsave.h"
#include "parser.h"
#include <string.h>

void StateSaver::save() {
	if(buf) delete[] buf;
	int bl=strlen(::buf);
	buf=new char[bl+1];
	if(buf) {
		memcpy(buf,::buf,bl+1);
		tc=::tc;
                ps.save();
	}
};


void StateSaver::restore() {
	if(buf) {
		ps.restore();
		::tc = tc;
		strcpy(::buf,buf);
	}
}
