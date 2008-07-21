/* RC language:
 script             ::=  { item }

 item               ::=  empty-line | ppd-line | comment | block

 empty-line         ::=  <eol>

 ppd-line           ::=  '#' ..... <eol>

 comment            ::=  '/' '*' .... '*' '/'

 block              ::=  [ dialog-block | menu-block | stringtable-block | bitmap-block | icon-block | rcdata-block | cursor-block | accelarator-block ]

 dialog-block       ::=  <exp> 'DIALOG' <load-and-mem-option> size dialog-body
 size               ::=  <exp> <comma> <exp> <comma> <exp> <comma> <exp>

 menu-block         ::=  <exp> 'MENU' <load-and-mem-option> menu-body
 menu-body          ::=  'BEGIN' { menu-part } 'END'
 menu-part          ::=  [ menu-popup | menu-item | empty-line]
 menu-popup         ::=  'POPUP' <string> <eol> menu-body
 menu-item          ::=  'MENUITEM' [ menu-text-item | menu-separator-item ]
 menu-text-item     ::=  <string> <comma> exp [ <comma> exp <eol> | <eol> ]
 menu-separator-item::=  'SEPARATOR'

 stringtable-block  ::=  STRINGTABLE <load-and-mem-option> stringtable-body
 stringtable-body   ::=  'BEGIN' {stringtable-entry} 'END'
 stringtable-entry  ::=  <exp> <comma> <string> <eol>

 bitmap-block       ::=  <exp> 'BITMAP' <filename>

 icon-block         ::=  <exp> 'ICON' <filename>

 cursor-block       ::=  <exp> 'CURSOR' <filename>

 rcdata-block       ::=  NOT IMPLEMENTED

 accelarator-block  ::= <exp> 'ACCELERATORS' <load-and-mem-option> accel-body
 accel-body         ::= 'BEGIN' {accel-entry} 'END'
 accel-entry        ::= keystroke <comma> <exp> [keystroke-type] [modifier] ['NOINVERT']
*/


#include "parser.h"
#include "pmenu.h"
#include "pstrtab.h"
#include "paccel.h"
#include "pbitmap.h"
#include "picon.h"
#include "pcursor.h"
#include "pdialog.h"
#include "bfile.h"
#include "statsave.h"
#include "output.h"
#include "oops.h"

#include <string.h>
#include <stdio.h>
#include "error.h"


token_class tc;
char buf[512];

void getNext() {
	tc=getToken(fp,buf);
}



void doEmptyLine();
void doPPDLine();
void doComment();
void doCPPComment();
void doBlock();
void doExp();

void doRCDATABlock() {
	ShowError(EX_DATAERR,"RCDATA not supported yet\n");
}


void doScript() {
	//parse a .RC script
	while(tc!=tc_eof) {
		switch(tc) {
			case tc_eol:
				doEmptyLine();
				break;
			case tc_ppd:
				doPPDLine();
				break;
			case tc_commentstart:
				doComment();
				break;
			case tc_cppcomment:
				doCPPComment();
				break;
			default:
				doBlock();
		}
	}
}

void doEmptyLine() {
	output("\n");
	getNext();
}

void doPPDLine() {
	if(stricmp(buf,"#include <windows.h>")!=0) {
		output(buf);
		output("\n");
	}
	getNext();
}

void doCPPComment() {
	output(buf);
	output("\n");
	getNext();
}

void doComment() {
	int c;

	output("/*");
	for(;;) {
		c=fp->get();
		switch(c) {
			case BufferedFile::Eof:
				doUnexpected("*/");
				break;
			case '*':
				output('*');
				c=fp->get();
				if(c=='/') {
					output('/');
					getNext();
					return;
				}
			/*falltrough*/
			default:
				output(c);
		}
	}
}

void doEOL() {
	if(tc!=tc_eol)
		doUnexpected("EOL");
	output();
	getNext();
}

void doComma() {
	if(tc!=tc_comma)
		doUnexpected("comma");
	output(',');
	getNext();
}

void doString() {
	if(tc!=tc_string)
		doUnexpected("string");
	output();
	getNext();
}


void doBlock() {
	//parse a resource block

	//stringtable is special since it hasn't got a resource name/identifier
	if(tc==tc_literal && stricmp(buf,"STRINGTABLE")==0) {
		doStringtableBlock();
		return;
	}


	/* Since the resource name/identifier can be an expression we have to
	 * simulate a LALR(k) parser even though we are only a LR parser.
	 * This is done by: saving the position, disabling output, parsing an
	 * expression, and finally looking at what we have got (bitmap/dialog/...)
	 */

	StateSaver ss;
	ss.save();
	disableOutput();

	doExp();
	enableOutput();
	//we cannot restore the position now, because of possible syntax errors

	if(tc==tc_literal) {
		if(strcmp(buf,"DIALOG")==0) {
			ss.restore();
			doDialogBlock();
		} else if(strcmp(buf,"MENU")==0) {
			ss.restore();
			doMenuBlock();
		} else if(strcmp(buf,"BITMAP")==0) {
			ss.restore();
			doBitmapBlock();
		} else if(strcmp(buf,"ICON")==0) {
			ss.restore();
			doIconBlock();
		} else if(strcmp(buf,"CURSOR")==0) {
			ss.restore();
			doCursorBlock();
		} else if(strcmp(buf,"RCDATA")==0) {
			ss.restore();
			doRCDATABlock();
		} else if(strcmp(buf,"ACCELERATORS")==0) {
			ss.restore();
			doAccelBlock();
		} else
			doUnexpected();
	} else
		doUnexpected();
}



// expression parsing

inline int isOp() {
	// the the token an operator
	return tc==tc_unknown && (buf[0]=='+' ||
				  buf[0]=='-' ||
				  buf[0]=='*' ||
				  buf[0]=='/' ||
				  buf[0]=='~');
}

void doExp() {
	// Parse an expression. Note that we have to allow literals to be use
	// instead of numbers because of #define's

	// Very little is done to check the syntax
	if(tc==tc_unknown && buf[0]=='(') {
		// '(' exp ')'
		output();
		getNext();
		doExp();
		if(tc!=tc_unknown || buf[0]!=')')
			doUnexpected("')'");
		output();
		getNext();
	} else if(tc==tc_literal || tc==tc_number) {
		output();
		getNext();
	}
	if(isOp()) {
		output();
		getNext();
		doExp();
	}
}
