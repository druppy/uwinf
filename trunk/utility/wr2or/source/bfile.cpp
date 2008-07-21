#include "bfile.h"
#include <io.h>
#include <fcntl.h>

BufferedFile::BufferedFile(const char *filename) {
	int fd=open(filename,O_BINARY|O_RDONLY);
	if(fd>=0) {
#ifdef __MSDOS__
		//We cannot handle more than 32Kb using MS-DOS
		if(filelength(fd)>32767) { buf=0; return; }
#endif
		int bytes=(int)filelength(fd);
		buf=new char[bytes+1];
		if(buf) {
			p=buf;
			if(read(fd,buf,bytes)==bytes) {
				buf[bytes]=0;
				convertToText();
				line=0;
			} else {
				delete[] buf;
				buf=0;
			}
		}
	} else
		buf=0;
}

BufferedFile::~BufferedFile() {
	if(buf) delete[] buf;
}


int BufferedFile::get() {
	if(*p) {
		if(*p=='\n') line++;
		return *p++;
	} else
		return Eof;
}

int BufferedFile::unget(char c) {
	if(p>buf) {
		if(c=='\n') line--;
		return *--p=c;
	} else
		return Eof;
}


void BufferedFile::convertToText() {
	//copy the text to itself skipping CR's
	char *s=p,*d=p;
	while(*s)
		if(*s=='\r')
			s++;
		else
			*d++=*s++;
	*d='\0';
}
