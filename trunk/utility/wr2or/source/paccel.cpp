#include "paccel.h"
#include "parser.h"
#include "pcommon.h"
#include "output.h"
#include "oops.h"

#include <string.h>


void doAccelBody();
void doAccelEntry();
const char *translateVK(const char *vk);

void doAccelBlock() {
	//current token is the resource-id
	output("ACCELTABLE ");
	doExp();
	getNext();	//eat 'accelerator'
	doLoadAndMemoryOption();
	doEOL();

	doAccelBody();
}

void doAccelBody() {
	doBEGIN(); doEOL();

	while(!isEND())
		doAccelEntry();

	doEND(); doEOL();
}

void doAccelEntry() {
	outputIndent();

	int dumpControl=0;
	if(tc==tc_string) {
		//OS/2 does not support "^A" shorthand
		if(buf[1]=='^') {
			char *s=buf, *d=buf;
			while(*s) {
				if(*s!='^')
					*d++=*s++;
				else
					s++;
			}
			*d='\0';
		}
		output();
		getNext();
	} else {
		if(buf[0]=='V' && buf[1]=='K' && buf[2]=='_') {
			//virtual key code
			output(translateVK(buf));
			getNext();
		} else
			doExp();	//keystroke
	}

	doComma();

	output('\t');

	doExp();		//accel-id / command

	if(tc==tc_comma) {
		getNext();	//eat the comma
		output('\t');
		if(dumpControl)
			output(",\tCONTROL");
		while(tc==tc_literal || tc==tc_comma) {
			if(tc==tc_comma) {
				getNext();	//eat the comma
			} else if(stricmp(buf,"ASCII")==0) {
				output(",CHAR ");
			} else if(stricmp(buf,"VIRTKEY")==0) {
				output(",VIRTUALKEY ");
			} else if(stricmp(buf,"SHIFT")==0) {
				output(",SHIFT ");
			} else if(stricmp(buf,"CONTROL")==0) {
				output(",CONTROL ");
			} else if(stricmp(buf,"ALT")==0) {
				output(",ALT ");
			} else if(stricmp(buf,"NOINVERT")==0) {
				displayIgnored("NOINVERT");
			} else
				doUnexpected();
			getNext();
		}
	} else if(dumpControl) {
		output(',');
		output('\t');
		output("CONTROL");
	}

	doEOL();
}

const char *translateVK(const char *vk) {
	// Translate a windows virtual keycode name into a OS/2 virtual keycode name

	//list of VKs that need translation
	static struct special {
		const char *winVK;
		const char *os2VK;
		const char *warntext;
	} specials[]= {
		{"VK_LBUTTON"	,"VK_BUTTON1"	,0},
		{"VK_RBUTTON"	,"VK_BUTTON2"	,0},
		{"VK_MBUTTON"	,"VK_BUTTON3"	,0},
		{"VK_CANCEL"	,"VK_BREAK"	,0},
		{"VK_BACK"      ,"VK_BACKSPACE"	,0},
		{"VK_RETURN"    ,"VK_NEWLINE"	,"VK_RETURN => VK_NEWLINE or VK_ENTER ?"},
		{"VK_CONTROL"   ,"VK_CTRL"	,0},
		{"VK_MENU"      ,"VK_ALT"	,"VK_MENU == VK_ALT or VK_MENU"},
		{"VK_CAPITAL"	,"VK_CAPSLOCK"	,0},
		{"VK_ESCAPE"	,"VK_ESC"	,0},
		{"VK_PRIOR"	,"VK_PAGEUP"	,0},
		{"VK_NEXT"	,"VK_PAGEDOWN"	,0},
		{"VK_SELECT"	,0		,"VK_SELECT has no equivalent"},
		{"VK_PRINT"	,0		,"VK_PRINT ???"},
		{"VK_EXECUTE"	,0		,"VK_EXECUTE has no equivalent"},
		{"VK_SNAPSHOT"	,"VK_PRINTSCRN"	,0},
		{"VK_HELP"	,0		,"VK_HELP has no equivalent"},
		{"VK_NUMPAD0"	,0		,"VK_NUMPAD0 has no equivalent"},
		{"VK_NUMPAD1"	,0		,"VK_NUMPAD1 has no equivalent"},
		{"VK_NUMPAD2"	,0		,"VK_NUMPAD2 has no equivalent"},
		{"VK_NUMPAD3"	,0		,"VK_NUMPAD3 has no equivalent"},
		{"VK_NUMPAD4"	,0		,"VK_NUMPAD4 has no equivalent"},
		{"VK_NUMPAD5"	,0		,"VK_NUMPAD5 has no equivalent"},
		{"VK_NUMPAD6"	,0		,"VK_NUMPAD6 has no equivalent"},
		{"VK_NUMPAD7"	,0		,"VK_NUMPAD7 has no equivalent"},
		{"VK_NUMPAD8"	,0		,"VK_NUMPAD8 has no equivalent"},
		{"VK_NUMPAD9"	,0		,"VK_NUMPAD9 has no equivalent"},
		{"VK_MULTIPLY"	,0		,"VK_MULTIPLY has no equivalent"},
		{"VK_ADD"	,0		,"VK_ADD has no equivalent"},
		{"VK_SEPARATOR"	,0		,"VK_SEPARATOR has no equivalent"},
		{"VK_SUBTRACT"	,0		,"VK_SUBTRACT has no equivalent"},
		{"VK_DECIMAL"	,0		,"VK_DECIMAL has no equivalent"},
		{"VK_DIVIDE"	,0		,"VK_DIVIDE has no equivalent"},
		{0,0,0}
	};
	special *p=specials;
	while(p->winVK) {
		if(strcmp(vk,p->winVK)==0) {
			if(p->warntext)
				displayWarning(p->warntext);
			if(p->os2VK)
				return p->os2VK;
			else
				return vk;	//leave it to the user to figure out what to do
		}
		p++;
	}
	return vk;	//windows VK and OS/2 VK is the same (or vk is unknown)
}
