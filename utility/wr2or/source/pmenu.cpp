#include "pmenu.h"
#include "parser.h"
#include "pcommon.h"
#include "output.h"
#include "oops.h"

#include <string.h>
#include <stdlib.h>

/* Syntax:
 menu-resource        ::= <res-id> 'MENU' <load&mem-options> menu-body
 menu-body            ::= 'BEGIN' {menu-item} 'END'
 menu-part            ::= menu-text-item | menu-separator-item | menu-submenu-item
 menu-text-item       ::= 'MENUITEM' <string> [<comma>] <id> [<comma> menu-options
 menu-separator-item  ::= 'MENUITEM' 'SEPARATOR'
 menu-submenu-item    ::= 'POPUP' <string> [<comma> menu-options] menu-body
*/

static int SubMenuID;


void doMenuBody();
void doMenuPart();
void doMenuPopup();
void doMenuItem();
void doMenuTextItem();
void doMenuSeparatorItem();
void doMenuOptions();
void doMenuString();

void doMenuBlock() {
	//current token is the menu name/identifier
	output("MENU ");
	doExp();
	getNext();	//eat the 'MENU' token
	doLoadAndMemoryOption();
	doEOL();

	SubMenuID=9000;
	doMenuBody();
}

void doMenuBody() {
	doBEGIN();
	while(!isEND()) doMenuPart();
	doEND();
}

void doMenuPart() {
	if(tc==tc_literal) {
		if(strcmp(buf,"POPUP")==0) {
			doMenuPopup();
			return;
		} else if(strcmp(buf,"MENUITEM")==0) {
			doMenuItem();
			return;
		}
	} else if(tc==tc_eol) {
		doEmptyLine();
		return;
	}
	doUnexpected("POPUP or MENUITEM");
}

void doMenuPopup() {
	//current token is 'POPUP'
	outputIndent();
	output("SUBMENU ");
	getNext();

	if(tc!=tc_string)
		doUnexpected("A string");
	doMenuString();

	//add a submenu ID
	char s[20];
	itoa(SubMenuID++,s,10);
	output(",");
	output(s);

	if(tc!=tc_eol) {
		doComma();
		doMenuOptions();
	}

	doEOL();

	doMenuBody();

}

void doMenuItem() {
	//current token is 'MENUITEM'
	outputIndent();
	output("MENUITEM ");
	getNext();

	if(tc==tc_string)
		doMenuTextItem();
	else if(tc==tc_literal && strcmp(buf,"SEPARATOR")==0)
		doMenuSeparatorItem();
	else
		doUnexpected();
}

void doMenuSeparatorItem() {
	//current token is 'SEPARATOR'
	output();
	getNext();
	doEOL();
}

void doMenuTextItem() {
	//current token is a string
	doMenuString();

	output('\t');
	if(tc==tc_comma) doComma();
	doExp();
	if(tc!=tc_eol) {
		//we have to translate 'checked'->'MIA_CHECKED' aso.
		doComma();

		doMenuOptions();
	}
	doEOL();
}



/** menu option translation *****************************************/
static char style[80];
static char attr[80];

static void addStyle(const char *s) {
	if(style[0])
		strcat(style,"|");
	strcat(style,s);
}
static void addAttr(const char *a) {
	if(attr[0])
		strcat(attr,"|");
	strcat(attr,a);
}


void doMenuOptions() {
	style[0]='\0';
	attr[0]='\0';
	while(tc!=tc_eof && tc!=tc_eol) {
		if(tc==tc_literal) {
			if(stricmp(buf,"CHECKED")==0) {
				addAttr("MIA_CHECKED");
			} else if(stricmp(buf,"GRAYED")==0) {
				addAttr("MIA_DISABLED");	//OS/2 does not have a grayed&not disabled concept
			} else if(stricmp(buf,"HELP")==0) {
				//eat it
			} else if(stricmp(buf,"INACTIVE")==0) {
				addAttr("MIA_DISABLED");
			} else if(stricmp(buf,"MENUBARBREAK")==0) {
				addStyle("MIS_BREAK");
			} else if(stricmp(buf,"MENUBREAK")==0) {
				addStyle("MIS_BREAK");
			} else {
				doUnexpected();
			}
		} else {
			//eat it
		}
		getNext();
	}
	output(style);
	if(attr[0]) {
		output(" ,");
		output(attr);
	}
}


/****** menu string translation  (& -> ~ ) **************************/
void doMenuString() {
	int i;
	for(i=0; buf[i];) {
		switch(buf[i]) {
			case '~':
				output('~');
				output('~');
				i++;
				break;
			case '&':
				output('~');
				i++;
				if(buf[i]=='&') {
					output('&');
					break;
				}
			/*fallthrough*/
			default:
				output(buf[i]);
				i++;
		}
	}
	getNext();
}
