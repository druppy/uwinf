#ifndef __STATSAVE_H
#define __STATSAVE_H

#include "bfile.h"
#include "wr2or.h"
#include "scanner.h"

class StateSaver {
	//This class enables us to save and restore file position and token
	bfPosSaver ps;	// file position saver
	token_class tc;	// saved token class
	char *buf;	// saved token string
public:
	StateSaver() : ps(*fp) { buf=0; save(); }
	~StateSaver() { if(buf) delete[] buf; }
	void save();
	void restore();
};

#endif
