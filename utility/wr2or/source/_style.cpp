#include "_win.h"
#include "_style.h"
#include "parser.h"
#include "oops.h"

#include <string.h>
#include <stdlib.h>


struct translateEntry {
	const char *string;
	unsigned long style;
} entries[]= {
	{"SS_LEFT",                      winSS_LEFT},
        {"SS_CENTER",                    winSS_CENTER},
        {"SS_RIGHT",                     winSS_RIGHT},
        {"SS_ICON",                      winSS_ICON},
        {"SS_BLACKRECT",                 winSS_BLACKRECT},
        {"SS_GRAYRECT",                  winSS_GRAYRECT},
        {"SS_WHITERECT",                 winSS_WHITERECT},
        {"SS_BLACKFRAME",                winSS_BLACKFRAME},
        {"SS_GRAYFRAME",                 winSS_GRAYFRAME},
        {"SS_WHITEFRAME",                winSS_WHITEFRAME},
        {"SS_SIMPLE",                    winSS_SIMPLE},
        {"SS_LEFTNOWORDWRAP",            winSS_LEFTNOWORDWRAP},
        {"SS_NOPREFIX",                  winSS_NOPREFIX},
        {"BS_PUSHBUTTON",                winBS_PUSHBUTTON},
        {"BS_DEFPUSHBUTTON",             winBS_DEFPUSHBUTTON},
        {"BS_CHECKBOX",                  winBS_CHECKBOX},
        {"BS_AUTOCHECKBOX",              winBS_AUTOCHECKBOX},
        {"BS_RADIOBUTTON",               winBS_RADIOBUTTON},
        {"BS_3STATE",                    winBS_3STATE},
        {"BS_AUTO3STATE",                winBS_AUTO3STATE},
        {"BS_GROUPBOX",                  winBS_GROUPBOX},
        {"BS_USERBUTTON",                winBS_USERBUTTON},
        {"BS_AUTORADIOBUTTON",           winBS_AUTORADIOBUTTON},
        {"BS_OWNERDRAW",                 winBS_OWNERDRAW},
	{"BS_LEFTTEXT",                  winBS_LEFTTEXT},
        {"ES_LEFT",                      winES_LEFT},
        {"ES_CENTER",                    winES_CENTER},
        {"ES_RIGHT",                     winES_RIGHT},
        {"ES_MULTILINE",                 winES_MULTILINE},
        {"ES_UPPERCASE",                 winES_UPPERCASE},
        {"ES_LOWERCASE",                 winES_LOWERCASE},
        {"ES_PASSWORD",                  winES_PASSWORD},
        {"ES_AUTOVSCROLL",               winES_AUTOVSCROLL},
        {"ES_AUTOHSCROLL",               winES_AUTOHSCROLL},
        {"ES_NOHIDESEL",                 winES_NOHIDESEL},
        {"ES_OEMCONVERT",                winES_OEMCONVERT},
        {"ES_READONLY",                  winES_READONLY},
        {"ES_WANTRETURN",                winES_WANTRETURN},
        {"SBS_HORZ",                     winSBS_HORZ},
        {"SBS_VERT",                     winSBS_VERT},
        {"SBS_TOPALIGN",                 winSBS_TOPALIGN},
        {"SBS_LEFTALIGN",                winSBS_LEFTALIGN},
	{"SBS_BOTTOMALIGN",              winSBS_BOTTOMALIGN},
        {"SBS_RIGHTALIGN",               winSBS_RIGHTALIGN},
        {"SBS_SIZEBOXTOPLEFTALIGN",      winSBS_SIZEBOXTOPLEFTALIGN},
        {"SBS_SIZEBOXBOTTOMRIGHTALIGN",  winSBS_SIZEBOXBOTTOMRIGHTALIGN},
        {"SBS_SIZEBOX",                  winSBS_SIZEBOX},
        {"LBS_NOTIFY",                   winLBS_NOTIFY},
        {"LBS_SORT",                     winLBS_SORT},
        {"LBS_NOREDRAW",                 winLBS_NOREDRAW},
        {"LBS_MULTIPLESEL",              winLBS_MULTIPLESEL},
	{"LBS_OWNERDRAWFIXED",           winLBS_OWNERDRAWFIXED},
        {"LBS_OWNERDRAWVARIABLE",        winLBS_OWNERDRAWVARIABLE},
        {"LBS_HASSTRINGS",               winLBS_HASSTRINGS},
	{"LBS_USETABSTOPS",              winLBS_USETABSTOPS},
        {"LBS_NOINTEGRALHEIGHT",         winLBS_NOINTEGRALHEIGHT},
        {"LBS_MULTICOLUMN",              winLBS_MULTICOLUMN},
        {"LBS_WANTKEYBOARDINPUT",        winLBS_WANTKEYBOARDINPUT},
        {"LBS_EXTENDEDSEL",              winLBS_EXTENDEDSEL},
        {"LBS_DISABLENOSCROLL",          winLBS_DISABLENOSCROLL},
        {"LBS_STANDARD",                 winLBS_STANDARD},
        {"CBS_SIMPLE",                   winCBS_SIMPLE},
	{"CBS_DROPDOWN",                 winCBS_DROPDOWN},
	{"CBS_DROPDOWNLIST",             winCBS_DROPDOWNLIST},
	{"CBS_OWNERDRAWFIXED",           winCBS_OWNERDRAWFIXED},
	{"CBS_OWNERDRAWVARIABLE",        winCBS_OWNERDRAWVARIABLE},
	{"CBS_AUTOHSCROLL",              winCBS_AUTOHSCROLL},
	{"CBS_OEMCONVERT",               winCBS_OEMCONVERT},
	{"CBS_SORT",                     winCBS_SORT},
	{"CBS_HASSTRINGS",               winCBS_HASSTRINGS},
	{"CBS_NOINTEGRALHEIGHT",         winCBS_NOINTEGRALHEIGHT},
	{"CBS_DISABLENOSCROLL",          winCBS_DISABLENOSCROLL},
	/* common window styles */
	{"WS_OVERLAPPED",                winWS_OVERLAPPED},
	{"WS_POPUP",                     winWS_POPUP},
	{"WS_CHILD",                     winWS_CHILD},
	{"WS_CLIPSIBLINGS",              winWS_CLIPSIBLINGS},
	{"WS_CLIPCHILDREN",              winWS_CLIPCHILDREN},
	{"WS_VISIBLE",                   winWS_VISIBLE},
	{"WS_DISABLED",                  winWS_DISABLED},
	{"WS_MINIMIZE",                  winWS_MINIMIZE},
	{"WS_MAXIMIZE",                  winWS_MAXIMIZE},
	{"WS_CAPTION",                   winWS_CAPTION},
	{"WS_BORDER",                    winWS_BORDER},
	{"WS_DLGFRAME",                  winWS_DLGFRAME},
	{"WS_VSCROLL",                   winWS_VSCROLL},
	{"WS_HSCROLL",                   winWS_HSCROLL},
	{"WS_SYSMENU",                   winWS_SYSMENU},
	{"WS_THICKFRAME",                winWS_THICKFRAME},
	{"WS_MINIMIZEBOX",               winWS_MINIMIZEBOX},
	{"WS_MAXIMIZEBOX",               winWS_MAXIMIZEBOX},
	{"WS_GROUP",                     winWS_GROUP},
	{"WS_TABSTOP",                   winWS_TABSTOP},
	{"WS_OVERLAPPEDWINDOW",          winWS_OVERLAPPEDWINDOW},
	{"WS_POPUPWINDOW",               winWS_POPUPWINDOW},
	{"WS_CHILDWINDOW",               winWS_CHILDWINDOW},
	/* dialog styles */
	{"DS_ABSALIGN",                  winDS_ABSALIGN},
	{"DS_SYSMODAL",                  winDS_SYSMODAL},
	{"DS_LOCALEDIT",                 winDS_LOCALEDIT},
	{"DS_SETFONT",                   winDS_SETFONT},
	{"DS_MODALFRAME",                winDS_MODALFRAME},
	{"DS_NOIDLEMSG",                 winDS_NOIDLEMSG},
	{0,0}
};

static unsigned long getStyleValue(const char *s) {
	translateEntry *p=entries;
	while(p->string) {
		if(stricmp(s,p->string)==0) return p->style;
		p++;
	}
	return (unsigned long)-1;	//not found
}

unsigned long translateStyleString(const char *_s, unsigned long defstyle) {
	unsigned long style=defstyle;
	char s[256]; strcpy(s,_s);
	char *p=strtok(s," \t");
	enum toDo { orIt, andIt, xorIt, removeIt } td=orIt;
	while(p) {
		if(strcmp(p,"|")==0) {
			td=orIt;
		} else if(strcmp(p,"&")==0) {
			td=andIt;
		} else if(strcmp(p,"^")==0) {
			td=xorIt;
		} else if(strcmp(p,"NOT")==0) {
			td=removeIt;
		} else {
			unsigned long ss=getStyleValue(p);
			if(ss==(unsigned long)-1) {
				//non-standard style or hardcoded style
				char *endptr;
				long l=strtol(p,&endptr,0);
				if(*endptr)
					doUnexpected(p);
				else
					style |= l;	//hardcoded
			}
			switch(td) {
				case orIt:
					style |= ss;
					break;
				case andIt:
					style &= ss;
					break;
				case xorIt:
					style ^= ss;
					break;
				case removeIt:
					style &= ~ss;
					break;
			}
		}
		p=strtok(0," \t");
	}
	return style;
}
