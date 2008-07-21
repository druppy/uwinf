#ifndef __BFILE_H
#define __BFILE_H

class bfPosSaver;
class BufferedFile {
	// A sequential read-only text-only line-counting file with large
	// unget() capability
	// This version just reads the entire file into memory
	char *buf;	// The entire file
	char *p;	// current position

	int line;	// line count

	void convertToText();	//convert CR-LF to LF

	int tell() { return int(p-buf); }
	void seek(int pos) { p=buf+pos; }
	friend class bfPosSaver;      
public:
	BufferedFile(const char *filename);
	~BufferedFile();
	operator int() const { return buf!=0; }

	int get();
	int unget(char c);

	int lineno() const { return line; }
	enum { Eof= -1 };
};

class bfPosSaver {
	// A class to save the current posiotion in a BufferedFile
	BufferedFile *bfp;	// the file
	int pos;		// the saved position
public:
	bfPosSaver(BufferedFile& bf) { bfp=&bf; save(); }
	void save() { pos=bfp->tell(); }
	void restore() { bfp->seek(pos); }
};

#endif
