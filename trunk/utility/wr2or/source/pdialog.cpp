#include "pdialog.h"
#include "parser.h"
#include "pcommon.h"
#include "_win.h"
#include "_style.h"
#include "output.h"
#include "oops.h"
#include "wr2or.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*
The syntax of a dialog resource is not described here. It is just too complex.

A few notes on how the translation is done:
First, all controls in the dialog are transformed into a generic control by
the doXXXXX functions.
Then an attempt is made to classify the control (button/text/edittext/...)
Then the control translation is passed on to the appropriate generateXXXXXX
function which knows how translate win-styles into OS/2-styles.

The transformation of the controls into generic controls is due to two reasons:
 1: We have to handle generic controls anyway
 2: We can handle silly statements such as a DEFPUSHBUTTON with the
    BS_DEFPUSHBUTTON style removed (this is legal!)
*/


#define MAXSTYLELEN        256
#define MAXTEXTLEN        128
#define MAXIDLEN        80

void doDialogEntry();
void outputDialogFooter();
void outputDialogHeader(int x, int y, int width, int height,
                        char *style,
                        const char *caption,
                        const char *id,
                        const char *menuname,
                        const char *classname,
                        const char *fontname,
                        int fontsize);
void doDialogStyleStuff(char *winstyle, char *pmstyle, char *pmframe);

void do3State();
void doAuto3State();
void doAutoCheckbox();
void doAutoRadioButton();
void doCheckbox();
void doCombobox();
void doControl();
void doCtext();
void doDefpushbutton();
void doEdittext();
void doGroupbox();
void doIcon();
void doListBox();
void doLtext();
void doPushButton();
void doRadioButton();
void doRtext();
void doScrollbar();
void handleGenericControl(const char *ctrl_text,
                          const char *ctrl_id,
                          const char *ctrl_class,
                          const char *ctrl_style,
                          int x,int y,int w,int h,
                          unsigned long defstyle
                         );
int generateCommonControlStyle(unsigned long s);
void outputTranslatedString(const char *s);
void generateGenericControlStatement(const char *ctrl_text,
                                     const char *ctrl_id,
                                     const char *ctrl_class,
                                     unsigned long style,
                                     int x,int y,int w,int h
                                    );
void generateStaticControlStatement(const char *ctrl_text,
                                    const char *ctrl_id,
                                    unsigned long style,
                                    int x,int y,int w,int h
                                   );
void generateTextControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateIconControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateRectangleFrameControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateButtonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateListboxControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateComboboxControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateEditControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateScrollbarControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateSLEControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateMLEControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h);


inline int parseNumber() {
        if(tc!=tc_number) doUnexpected("a number (only numbers are supported)");
        int i=atoi(buf);
        getNext();
        return i;
}


static int __dlg_h;
static void translateDialogXYWH(int &, int &, int &, int &h) {
        __dlg_h = h;
}
static void translateControlXYWH(int &, int &y, int &, int &h) {
        y = __dlg_h- (y+h);
}


void doDialogBlock() {
        output("DLGTEMPLATE ");

        //We have to save the dialog-id so we can use it in both
        //the DLGTEMPLATE staement and the DIALOG statement
        char dlg_id[MAXIDLEN]="";
        while(tc!=tc_literal || stricmp(buf,"DIALOG")!=0) {
                strcat(dlg_id,buf);
                getNext();
        }
        output(dlg_id); output(' ');

        getNext();        //eat 'DIALOG'
        doLoadAndMemoryOption();
        output('\n');

        disableOutput();

        int dlg_x, dlg_y, dlg_width, dlg_height;
        char        dlg_style[MAXSTYLELEN]="";
        char        dlg_caption[MAXTEXTLEN]="";
        char        dlg_menuname[80]="";
        char        dlg_class[80]="";
        char        dlg_fontname[80]="";
        int        dlg_fontsize=0;

        dlg_x=parseNumber();

        doComma();

        dlg_y=parseNumber();

        doComma();

        dlg_width=parseNumber();

        doComma();

        dlg_height=parseNumber();

        doEOL();

        translateDialogXYWH(dlg_x,dlg_y,dlg_width,dlg_height);

        while(!isBEGIN()) {
                if(tc!=tc_literal) doUnexpected("STYLE/CAPTION/MENU/CLASS/FONT");
                if(stricmp(buf,"STYLE")==0) {
                        getNext();
                        while(tc!=tc_eol && tc!=tc_eof) {
                                strcat(dlg_style," ");
                                strcat(dlg_style,buf);
                                getNext();
                                if(tc==tc_eol && dlg_style[strlen(dlg_style)-1]=='|')
                                        getNext(); //continued on next line
                        }
                } else if(stricmp(buf,"CAPTION")==0) {
                        getNext();
                        if(tc!=tc_string) doUnexpected("a string");
                        strcpy(dlg_caption,buf);
                        getNext();
                        strcat(dlg_style," | WS_CAPTION");        //documented behavior of MS RC
                } else if(stricmp(buf,"MENU")==0) {
                        getNext();
                        strcpy(dlg_menuname,buf);
                        getNext();
                } else if(stricmp(buf,"CLASS")==0) {
                        getNext();
                        strcpy(dlg_class,buf);
                        getNext();
                } else if(stricmp(buf,"FONT")==0) {
                        getNext();
                        if(tc!=tc_number) doUnexpected("a number");
                        dlg_fontsize=atoi(buf);
                        getNext();
                        doComma();
                        if(tc!=tc_string) doUnexpected("a string");
                        strcpy(dlg_fontname,buf);
                        getNext();
                } else {
                        doUnexpected("STYLE/CAPTION/MENU/CLASS/FONT");
                }
                doEOL();
        }
        enableOutput();

        doBEGIN(); doEOL();
        outputDialogHeader(dlg_x,dlg_y,dlg_width,dlg_height,dlg_style,dlg_caption,dlg_id,dlg_menuname,dlg_class,dlg_fontname,dlg_fontsize);
        while(!isEND()) doDialogEntry();
        outputDialogFooter();
        doEND(); doEOL();
}

void outputDialogHeader(int x, int y, int width, int height,
                              char *style,
                        const char *caption,
                        const char *id,
                        const char *menuname,
                        const char *classname,
                        const char *fontname,
                        int            fontsize)
{
        outputIndent();
        output("DIALOG ");
        if(caption[0])
                output(caption);
        else
                output("\"\"");
        output(", ");
        output(id); output(", ");
        output(x); output(','); output(y); output(',');
        output(width); output(','); output(height);
        if(style[0]) {
                output(", ");
                char pmstyle[MAXSTYLELEN],pmframestyle[MAXSTYLELEN];
                doDialogStyleStuff(style,pmstyle,pmframestyle);
                if(pmstyle[0] || pmframestyle[0]) {
                        output(pmstyle);
                        if(pmframestyle[0]) {
                                output(", ");
                                output(pmframestyle);
                        }
                }
        }
        output('\n');
        if(*fontname) {
                outputIndent();
                output("PRESPARAMS PP_FONTNAMESIZE, \"");
                if(fontsize>0)
                        output(fontsize);
                else
                        output(10);
                output('.');
                while(*fontname) {
                        if(*fontname!='"') output(*fontname);
                        fontname++;
                }
                output("\"\n");
        }
        if(menuname[0])
                displayWarning("You have to manually insert a CTLDATA MENU statement for the dialog box");
        if(classname[0])
                displayWarning("You have to manually change the DIALOG statement into a WINDOW statement in order to use a different window class");
        outputBEGIN();
}

void outputDialogFooter() {
        outputEND();
}


inline void addStyle(char *s, const char *a) {
        if(s[0]) strcat(s,"|");
        strcat(s,a);
}

void doDialogStyleStuff(char *winstyle, char *pmstyle, char *pmframe) {
        /* Translate the windows dialog style into style and framestyle.
         * NOTE: This is not easy and this implementation is quite buggy.
         */

        unsigned long style=translateStyleString(winstyle,0);

        pmstyle[0]='\0';
        pmframe[0]='\0';

        //window styles
        addStyle(pmstyle,"WS_VISIBLE");
        if(style&winWS_CAPTION)
                addStyle(pmframe,"FCF_TITLEBAR");
        if(style&winWS_BORDER && (style&winDS_MODALFRAME)==0) {
                addStyle(pmframe,"FCF_BORDER");
        }
        if(style&winWS_DLGFRAME)
                addStyle(pmstyle,"FS_DLGBORDER");
        if(style&winWS_VSCROLL)
                addStyle(pmframe,"FCF_VERTSCROLL");
        if(style&winWS_HSCROLL)
                addStyle(pmframe,"FCF_HORZSCROLL");
        if(style&winWS_SYSMENU)
                addStyle(pmframe,"FCF_SYSMENU");
        if(style&winWS_THICKFRAME) {
                addStyle(pmstyle,"NOT FS_DLGBORDER");
                addStyle(pmstyle,"FS_SIZEBORDER");
        }
        if(style&winWS_MINIMIZEBOX)
                addStyle(pmframe,"FCF_MINBUTTON");
        if(style&winWS_MAXIMIZEBOX)
                addStyle(pmframe,"FCF_MAXBUTTON");
        //dialog styles
        if(style&winDS_ABSALIGN)
                addStyle(pmframe,"FCF_SCREENALIGN");
        if(style&winDS_SYSMODAL)
                addStyle(pmframe,"FCF_SYSMODAL");
        if(style&winDS_LOCALEDIT)
                displayIgnored("DS_LOCALEDIT is not supported");
        //if(style&winDS_SETFONT)
        if(style&winDS_MODALFRAME)
                addStyle(pmframe,"FCF_DLGBORDER");
        if(style&winDS_NOIDLEMSG)
                displayIgnored("DS_NOIDLEMSG is not supported");
}

void doDialogEntry() {
        if(tc!=tc_literal) doUnexpected("CHECKBOX/.../SCROLLBAR");
        if(stricmp(buf,"AUTO3STATE")==0) {
                doAuto3State();
        } else if(stricmp(buf,"AUTOCHECKBOX")==0) {
                doAutoCheckbox();
        } else if(stricmp(buf,"AUTORADIOBUTTON")==0) {
                doAutoRadioButton();
        } else if(stricmp(buf,"CHECKBOX")==0) {
                doCheckbox();
        } else if(stricmp(buf,"COMBOBOX")==0) {
                doCombobox();
        } else if(stricmp(buf,"CONTROL")==0) {
                doControl();
        } else if(stricmp(buf,"CTEXT")==0) {
                doCtext();
        } else if(stricmp(buf,"DEFPUSHBUTTON")==0) {
                doDefpushbutton();
        } else if(stricmp(buf,"EDITTEXT")==0) {
                doEdittext();
        } else if(stricmp(buf,"GROUPBOX")==0) {
                doGroupbox();
        } else if(stricmp(buf,"ICON")==0) {
                doIcon();
        } else if(stricmp(buf,"LISTBOX")==0) {
                doListBox();
        } else if(stricmp(buf,"LTEXT")==0) {
                doLtext();
        } else if(stricmp(buf,"PUSHBUTTON")==0) {
                doPushButton();
        } else if(stricmp(buf,"RADIOBUTTON")==0) {
                doRadioButton();
        } else if(stricmp(buf,"RTEXT")==0) {
                doRtext();
        } else if(stricmp(buf,"SCROLLBAR")==0) {
                doScrollbar();
        } else if(stricmp(buf,"STATE3")==0) {
                do3State();
        } else {
                doUnexpected("Some sort of control statement");
        }
}



/******************* control parsing ***************************************/

//control parsing helper functions ***************************************
static void skipEOL() {
        if(tc==tc_eol) doEOL();
}

const char *translateControlID(const char *winID) {
        struct special {
                const char *winID;
                const char *pmID;
        };
        static special specials[] = {
                {"IDOK",        "DID_OK"},
                {"IDCANCEL",        "DID_CANCEL"},
                {"-1",                "0xffff"},
                {0,0}
        };
        special *p=specials;
        while(p->winID) {
                if(strcmp(winID,p->winID)==0)
                        return p->pmID;
                p++;
        }
        return winID;
}

inline int doNumber() {
        if(tc!=tc_number) doUnexpected("a number/value");
        int v=atoi(buf);
        getNext();
        return v;
}

static void ctrlStyle(char *s) {
        s[0]='\0';
        while(tc!=tc_eol && tc!=tc_comma && tc!=tc_eof) {
                strcat(s," ");
                strcat(s,buf);
                if(tc==tc_unknown && strcmp(buf,"|")==0) {
                        getNext();
                        skipEOL();
                } else
                        getNext();
        }
}

static void ctrlText(char *s) {
        /* Since a control text can be a value (or even a #define) it is a
         * bit more complicated than just a string
         */
        if(tc!=tc_string) {
                if(tc==tc_comma) {
                        //empty text
                        *s='\0';
                } else {
                        //assume an #define expression
                        strcpy(s,buf);
                        getNext();
                }
        } else {
                strcpy(s,buf);
                getNext();
        }
}

static void ctrlClass(char *s) {
        if(tc!=tc_string) {
                if(tc==tc_comma) {
                        //empty text
                        *s='\0';
                } else {
                        doUnexpected("a string");
                }
        } else {
                strcpy(s,buf);
                getNext();
        }
}

static void ctrlID(char *s) {
        //The id can be any kind of expression
        *s='\0';
        while(tc!=tc_comma && tc!=tc_eol && tc!=tc_eof) {
                strcat(s,buf);
                getNext();
        }
}

inline int ctrlX() {
        return doNumber();
}

inline int ctrlY() {
        return doNumber();
}

inline int ctrlW() {
        return doNumber();
}

inline int ctrlH() {
        return doNumber();
}


// control parsing functions ***********************************************
static void doCheckbox_common(unsigned long bs);

void doAuto3State() {
        doCheckbox_common(winBS_AUTO3STATE);
}
void do3State() {
        doCheckbox_common(winBS_3STATE);
}
void doAutoCheckbox() {
        doCheckbox_common(winBS_AUTOCHECKBOX);
}

void doCheckbox() {
        doCheckbox_common(winBS_CHECKBOX);
}

static void doCheckbox_common(unsigned long bs) {
        getNext();        //eat 'CHECKBOX'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"button\"",style,x,y,width,height, winWS_VISIBLE|winWS_TABSTOP|bs);
}

void doCombobox() {
        getNext();        //eat 'COMBOBOX'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"combobox\"",style,x,y,width,height, winWS_VISIBLE|winWS_TABSTOP);
}


void doControl() {
        getNext();        //eat 'CONTROL'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="", cclass[80]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        ctrlClass(cclass); doComma(); skipEOL();
        ctrlStyle(style); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        doEOL();
        enableOutput();

        handleGenericControl(text,id,cclass,style,x,y,width,height, winWS_VISIBLE);
}


void doCtext() {
        getNext();        //eat 'CTEXT'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"static\"",style,x,y,width,height, winWS_VISIBLE|winWS_GROUP|(style[0]?0:winSS_CENTER));
}


void doEdittext() {
        getNext();        //eat 'EDITTEXT'
        char id[MAXTEXTLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl("",id,"\"edit\"",style,x,y,width,height, winWS_VISIBLE|winWS_TABSTOP|winWS_BORDER|winES_LEFT);
}



void doGroupbox() {
        getNext();        //eat 'GROUPBOX'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"button\"",style,x,y,width,height, winWS_VISIBLE|winBS_GROUPBOX);
}


void doIcon() {
        getNext();        //eat 'ICON'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width=-1,height=-1;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                width=ctrlW();
                doComma(); skipEOL();
                height=ctrlH();
                if(tc==tc_comma) {
                        doComma(); skipEOL();
                        ctrlStyle(style);
                }
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"static\"",style,x,y,width,height, winWS_VISIBLE|winSS_ICON);
}


void doListBox() {
        getNext();        //eat 'LISTBOX'
        char id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl("",id,"\"listbox\"",style,x,y,width,height, winWS_VISIBLE|winWS_BORDER|winLBS_NOTIFY);
}



void doLtext() {
        getNext();        //eat 'LTEXT'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"static\"",style,x,y,width,height, winWS_VISIBLE|winWS_GROUP|(style[0]?0:winSS_LEFT));
}


static void doPushButton_common(unsigned long bs);
void doDefPushButton() {
        doPushButton_common(winBS_DEFPUSHBUTTON);
}
void doPushButton() {
        doPushButton_common(winBS_PUSHBUTTON);
}

static void doPushButton_common(unsigned long bs) {
        getNext();        //eat 'PUSHBUTTON'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"button\"",style,x,y,width,height, winWS_VISIBLE|winWS_TABSTOP|bs);
}

static void doRadioButton_common(unsigned long bs);
void doAutoRadioButton() {
        doRadioButton_common(winBS_AUTORADIOBUTTON);
}
void doRadioButton() {
        doRadioButton_common(winBS_RADIOBUTTON);
}
static void doRadioButton_common(unsigned long bs) {
        getNext();        //eat 'RADIOBUTTON'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"button\"",style,x,y,width,height, winWS_VISIBLE|winWS_TABSTOP|bs);
}

void doRtext() {
        getNext();        //eat 'RTEXT'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"static\"",style,x,y,width,height, winWS_VISIBLE|winWS_GROUP|(style[0]?0:winSS_RIGHT));
}

void doScrollbar() {
        getNext();        //eat 'SCROLLBAR'
        char text[MAXTEXTLEN]="",id[MAXIDLEN]="";
        int x,y,width,height;
        char style[MAXSTYLELEN]="";

        disableOutput();

        ctrlText(text); doComma(); skipEOL();
        ctrlID(id); doComma(); skipEOL();
        x=ctrlX(); doComma(); skipEOL();
        y=ctrlY(); doComma(); skipEOL();
        width=ctrlW(); doComma(); skipEOL();
        height=ctrlH();
        if(tc==tc_comma) {
                doComma(); skipEOL();
                ctrlStyle(style);
        }
        doEOL();
        enableOutput();

        handleGenericControl(text,id,"\"scrollbar\"",style,x,y,width,height, winWS_VISIBLE|winSBS_HORZ);
}



void handleGenericControl(const char *ctrl_text,
                          const char *ctrl_id,
                          const char *ctrl_class,
                          const char *ctrl_style,
                          int x,int y,int w,int h,
                          unsigned long defstyle
                         )
{
        unsigned long style=translateStyleString(ctrl_style,defstyle);
        translateControlXYWH(x,y,w,h);
        ctrl_id=translateControlID(ctrl_id);
        if(stricmp(ctrl_class,"\"static\"")==0) {
                generateStaticControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
        } else if(stricmp(ctrl_class,"\"button\"")==0) {
                generateButtonControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
        } else if(stricmp(ctrl_class,"\"listbox\"")==0) {
                generateListboxControlStatement(ctrl_id,style,x,y,w,h);
        } else if(stricmp(ctrl_class,"\"combobox\"")==0) {
                generateComboboxControlStatement(ctrl_id,style,x,y,w,h);
        } else if(stricmp(ctrl_class,"\"edit\"")==0) {
                generateEditControlStatement(ctrl_id,style,x,y,w,h);
        } else if(stricmp(ctrl_class,"\"scrollbar\"")==0) {
                generateScrollbarControlStatement(ctrl_id,style,x,y,w,h);
        } else {
                displayNote("Non-standard control class. May not be available under PM");
                generateGenericControlStatement(ctrl_text,ctrl_id,ctrl_class,style,x,y,w,h);
        }
}

/***************************************************************************/
/**** Control statement generation */


// generation helper functions

int generateCommonControlStyle(unsigned long s) {
        //generate styles from the upper 16 bit of a window style
        int g=0;

        if(!(s&winWS_VISIBLE)) {
                //by default controls are visible unless the opposite is specified
                if(g++) output('|');
                output("NOT WS_VISIBLE");
        }
        if(s&winWS_TABSTOP) {
                if(g++) output('|');
                output("WS_TABSTOP");
        }
        if(s&winWS_DISABLED) {
                if(g++) output('|');
                output("WS_DISABLED");
        }
        if(s&winWS_GROUP) {
                if(g++) output('|');
                output("WS_GROUP");
        }
        return g!=0;
}

void generateXYWH(int x, int y, int w, int h) {
        output(x); output(',');
        output(y); output(',');
        output(w); output(',');
        output(h);
}

void generateTIXYWH(const char *text, const char *id, int x, int y, int w, int h, int translate=0) {
        if(!translate)
                output(text);
        else
                outputTranslatedString(text);
        output(", ");
        output(id); output(", ");
        generateXYWH(x,y,w,h);
}

void outputTranslatedString(const char *s) {
        //translate '&' to '~'
        while(*s) {
                switch(*s) {
                        case '~':
                                output('~');
                                output('~');
                                s++;
                                break;
                        case '&':
                                output('~');
                                s++;
                                if(*s=='&') {
                                        output('&');
                                        break;
                                }
                        /*fallthrough*/
                        default:
                                output(*s);
                                s++;
                }
        }
}


// generation

void generateGenericControlStatement(const char *ctrl_text,
                                     const char *ctrl_id,
                                     const char *ctrl_class,
                                     unsigned long style,
                                     int x,int y,int w,int h
                                    )
{
        outputIndent();
        output("CONTROL ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h); output(',');
        output(ctrl_class); output(',');
        if(generateCommonControlStyle(style)) {
                if(style&0xffff) {
                        char t[10];
                        sprintf(t,"|0x%04X",int(style&0xfff));
                        output(t);
                }
        }
        output('\n');
}


void generateStaticControlStatement(const char *ctrl_text,
                                    const char *ctrl_id,
                                    unsigned long style,
                                    int x,int y,int w,int h
                                   )
{
        switch(style&0xf) {
                case winSS_LEFT:
                case winSS_LEFTNOWORDWRAP:
                case winSS_SIMPLE:
                case winSS_CENTER:
                case winSS_RIGHT:
                        generateTextControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winSS_ICON:
                        generateIconControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                case winSS_BLACKRECT:
                case winSS_GRAYRECT:
                case winSS_WHITERECT:
                case winSS_BLACKFRAME:
                case winSS_GRAYFRAME:
                case winSS_WHITEFRAME:
                        generateRectangleFrameControlStatement(ctrl_id,style,x,y,w,h);
                        break;
                default:
                        displayNote("Unknown static control style found");
                        generateGenericControlStatement(ctrl_text,ctrl_id,"\"static\"",style,x,y,w,h);
                        break;
        }
}


void generateTextControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h)
{
        outputIndent();
        switch(style&0xf) {
                case winSS_CENTER:
                        output("LTEXT ");
                        break;
                case winSS_RIGHT:
                        output("RTEXT ");
                        break;
                default:
                        output("LTEXT ");
                        break;
        }

        if(style&winSS_NOPREFIX)
                generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h);
        else
                generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1);
        output(',');

        int g=generateCommonControlStyle(style);
        if((style&winSS_NOPREFIX)==0 && strchr(ctrl_text,'&')) {
                if(g++) output('|');
                output("DT_MNEMONIC");
        }
        if((style&0xf)!=winSS_LEFTNOWORDWRAP) {
                if(g++) output('|');
                output("DT_WORDBREAK");
        }
        output('\n');
}

void generateIconControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h)
{
        outputIndent();
        output("ICON ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h); output(',');
        generateCommonControlStyle(style);
        output('\n');
}

void generateRectangleFrameControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("CONTROL ");
        generateTIXYWH("\"\"",ctrl_id,x,y,w,h); output(',');
        output("WC_STATIC, ");
        if(generateCommonControlStyle(style)) output('|');
        switch(style&0xf) {
                case winSS_BLACKRECT:
                        output("SS_FGNDRECT");
                        break;
                case winSS_GRAYRECT:
                        output("SS_HALFTONERECT");
                        break;
                case winSS_WHITERECT:
                        output("SS_BKGNDRECT");
                        break;
                case winSS_BLACKFRAME:
                        output("SS_FGNDFRAME");
                        break;
                case winSS_GRAYFRAME:
                        output("SS_HALFTONEFRAME");
                        break;
                case winSS_WHITEFRAME:
                        output("SS_BKGNDFRAME");
                        break;
        }
        output('\n');

        displayNote("Rectangle/frame translation isn't very good");
}



void generateEditControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        if(style&winES_MULTILINE)
                generateMLEControlStatement(ctrl_id,style,x,y,w,h);
        else
                generateSLEControlStatement(ctrl_id,style,x,y,w,h);
}

void generateSLEControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("EDITTEXT ");
        if(style&winWS_BORDER && h==12) h = 8; //exclude border
        generateTIXYWH("\"\"",ctrl_id,x,y,w,h); output(',');

        int g=generateCommonControlStyle(style);
        switch(style&0x3) {
                case winES_LEFT:
                        if(g++) output('|');
                        output("ES_LEFT");
                        break;
                case winES_CENTER:
                        if(g++) output('|');
                        output("ES_CENTER");
                        break;
                case winES_RIGHT:
                        if(g++) output('|');
                        output("ES_RIGHT");
                        break;
        }
        if(style&(winES_UPPERCASE|winES_LOWERCASE))
                displayWarning("OS/2 does not support ES_UPPERCASE/ES_LOWERCASE");
        if(style&winES_PASSWORD) {
                if(g++) output('|');
                output("ES_UNREADABLE");
        }
        if(style&winES_AUTOHSCROLL) {
                if(g++) output('|');
                output("ES_AUTOSCROLL");
        }
        if(style&winES_READONLY) {
                if(g++) output('|');
                output("ES_READONLY");
        }
        if(style&winES_NOHIDESEL)
                displayWarning("ES_NOHIDESEL is not supported");
        if(style&winES_OEMCONVERT)
                displayIgnored("ES_OEMCONVERT is not a problem");
        // ES_NOHIDESEL is not supported,
        // ES_OEMCONVERT is not a problem
        if(style&winWS_BORDER) {
                if(g++) output('|');
                output("ES_MARGIN");
        }

        output('\n');
}


void generateMLEControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("MLE ");
        generateTIXYWH("\"\"",ctrl_id,x,y,w,h); output(',');

        int g=generateCommonControlStyle(style);
        if((style&0x3)!=winES_LEFT)                                  
                displayWarning("ES_CENTER/ES_RIGHT is not supported in MLEs");
        if(style&(winES_UPPERCASE|winES_LOWERCASE))
                displayWarning("OS/2 does not support ES_UPPERCASE/ES_LOWERCASE");
        if(style&winES_PASSWORD)
                displayWarning("ES_PASSWORD is not supported in MLEs");
        if(style&winES_AUTOHSCROLL) {
                if(g++) output('|');
                output("MLS_HSCROLL");
        }
        if(style&winES_AUTOVSCROLL) {
                if(g++) output('|');
                output("MLS_VSCROLL");
        }
        if(style&winES_READONLY) {
                if(g++) output('|');
                output("MLS_READONLY");
        }
        if(style&winES_NOHIDESEL)
                displayWarning("ES_NOHIDESEL is not supported");
        if(style&winES_OEMCONVERT)
                displayIgnored("ES_OEMCONVERT is not a problem");
        if(style&winES_WANTRETURN)
                displayIgnored("ES_WANTRETURN is default");
        if(style&winWS_BORDER) {
                if(g++) output('|');
                output("MLS_BORDER");
        }

        output('\n');
}

void generateComboboxControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("COMBOBOX ");
        generateTIXYWH("\"\"",ctrl_id,x,y,w,h); output(',');

        int g=generateCommonControlStyle(style);
        switch(style&0xf) {
                case winCBS_DROPDOWN:
                        if(g++) output('|');
                        output("CBS_DROPDOWN");
                        break;
                case winCBS_DROPDOWNLIST:
                        if(g++) output('|');
                        output("CBS_DROPDOWNLIST");
                        break;
                case winCBS_SIMPLE:
                default:
                        if(g++) output('|');
                        output("CBS_SIMPLE");
                        break;
        }
        if(style&winCBS_AUTOHSCROLL) {
                if(g++) output('|');
                output("LS_HORZSCROLL");
        }

        if(style&winCBS_OWNERDRAWFIXED)
                displayWarning("CBS_OWNERDRAWFIXED cannot be specified in the resources");
        if(style&winCBS_OWNERDRAWVARIABLE)
                displayWarning("CBS_OWNERDRAWVARIABLE is not supported by OS/2");
        if(style&winCBS_OEMCONVERT)
                displayIgnored("CBS_OEMCONVERT is not a problem");
        if(style&winCBS_SORT)
                displayWarning("(CBS_SORT) Sorting is specified at runtime");
        if(style&winCBS_HASSTRINGS)
                displayIgnored("CBS_HASSTRINGS");
        if(style&winCBS_NOINTEGRALHEIGHT) {
                displayWarning("CBS_NOINTEGRALHEIGHT cannot be specified in the resources");
                displayNote("Add the LS_NOADJUSTPOS style at runtime in order to achieve a simular effect");
        }
        if(style&winCBS_DISABLENOSCROLL)
                displayIgnored("CBS_DISABLENOSCROLL is not needed");
        output('\n');
}


void generateListboxControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("LISTBOX ");
        output(ctrl_id); output(',');
        generateXYWH(x,y,w,h); output(',');

        int g=generateCommonControlStyle(style);
        if(style&winLBS_NOTIFY)
                displayIgnored("LBS_NOTIFY is ignored");
        if(style&winLBS_SORT)
                displayWarning("(LBS_SORT) Sorting is specified at runtime");
        if(style&winLBS_NOREDRAW)
                displayWarning("LBS_NOREDRAW is not supported");
        if(style&winLBS_MULTIPLESEL) {
                if(g++) output('|');
                output("LS_MULTIPLESEL");
        }
        if(style&winLBS_OWNERDRAWFIXED) {
                if(g++) output('|');
                output("LS_OWNERDRAW");
        }
        if(style&winLBS_OWNERDRAWVARIABLE) {
                displayWarning("Only LBS_OWNERDRAWFIXED is supported");
                displayNote("winLBS_OWNERDRAWVARIABLE was converted to LBS_OWNERDRAWFIXED");
                if(g++) output('|');
                output("LS_OWNERDRAW");
        }
        if(style&winLBS_HASSTRINGS)
                displayIgnored("LBS_HASSTRINGS");
        if(style&winLBS_USETABSTOPS)
                displayWarning("LBS_USETABSTOPS is not directly translatable");
        if(style&winLBS_NOINTEGRALHEIGHT) {
                if(g++) output('|');
                output("LS_NOADJUSTPOS");
        }
        if(style&winLBS_MULTICOLUMN) {
                if(g++) output('|');
                output("LS_HORZSCROLL");
        }
        if(style&winLBS_WANTKEYBOARDINPUT)
                displayWarning("LBS_WANTKEYBOARDINPUT is not supported");
        if(style&winLBS_EXTENDEDSEL) {
                if(g++) output('|');
                output("LS_EXTENDEDSEL");
        }
        if(style&winLBS_DISABLENOSCROLL)
                displayIgnored("LBS_DISABLENOSCROLL is default");

        output('\n');
}


void generateScrollbarControlStatement(const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        /* NOTE: scrollbars are translated into slider controls because it is
         * very rare that scrollbars in dialogs are used for scrolling.
         * Usually they are used for adjusting a value.
         */
        if(style&winSBS_SIZEBOX) {
                displayWarning("SBS_SIZEBOX is not suported by OS/2 and there are no equivalent");
                outputIndent();
                output("/* sizebox was here */\n");
                return;
        }

        outputIndent();
        output("SLIDER ");
        output(ctrl_id); output(',');
        generateXYWH(x,y,w,h); output(',');

        int g=generateCommonControlStyle(style);
        int ishorz=(style&0x1)==winSBS_HORZ;

        //orientation
        if(ishorz) {
                if(g++) output('|');
                output("SLS_HORIZONTAL");
        } else {
                if(g++) output('|');
                output("SLS_VERTICAL");
        }

        //alignment
        if(g++) output('|');
        if(ishorz) {
                if(style&winSBS_TOPALIGN)
                        output("SLS_TOP");
                else if(style&winSBS_BOTTOMALIGN)
                        output("SLS_BOTTOM");
                else output("SLS_CENTER");
                output("|SLS_BOTTOMS_RIGHT");
        } else {
                if(style&winSBS_LEFTALIGN)
                        output("SLS_LEFT");
                else if(style&winSBS_RIGHTALIGN)
                        output("SLS_RIGHT");
                else
                        output("SLS_CENTER");
                output("|SLS_BOTTOMS_TOP");
        }

        output('\n');
}



void generatePushbuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateDefpushbuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateCheckboxControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateAutocheckboxControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateRadiobuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generate3stateControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateAuto3stateControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateGroupboxControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateAutoradiobuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);
void generateUserbuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h);


void generateButtonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        switch(style&0xf) {
                case winBS_PUSHBUTTON:
                        generatePushbuttonControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_DEFPUSHBUTTON:
                        generateDefpushbuttonControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_CHECKBOX:
                        generateCheckboxControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_AUTOCHECKBOX:
                        generateAutocheckboxControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_RADIOBUTTON:
                        generateRadiobuttonControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_3STATE:
                        generate3stateControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_AUTO3STATE:
                        generateAuto3stateControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_GROUPBOX:
                        generateGroupboxControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_USERBUTTON:
                        generateUserbuttonControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_AUTORADIOBUTTON:
                        generateAutoradiobuttonControlStatement(ctrl_text,ctrl_id,style,x,y,w,h);
                        break;
                case winBS_OWNERDRAW:
                        displayWarning("BS_OWNERDRAW is not supported. Use subclassing or a BS_USERBUTTON");
                        break;
                default:
                        displayWarning("Unknown button type found");
        }
        if(style&winBS_LEFTTEXT)
                displayIgnored("BS_LEFTTEXT is not supported");

}


void generatePushbuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("PUSHBUTTON ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        generateCommonControlStyle(style);
        output('\n');
}

void generateDefpushbuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("DEFPUSHBUTTON ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        generateCommonControlStyle(style);
        output('\n');
}

void generateCheckboxControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("CHECKBOX ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        generateCommonControlStyle(style);
        output('\n');
}

void generateAutocheckboxControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("AUTOCHECKBOX ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        generateCommonControlStyle(style);
        output('\n');
}


void generateRadiobuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("RADIOBUTTON ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        generateCommonControlStyle(style);
        output('\n');
}

void generateAutoradiobuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("AUTORADIOBUTTON ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        generateCommonControlStyle(style);
        output('\n');
}

void generate3stateControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("CHECKBOX ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        int g=generateCommonControlStyle(style);
        if(g) output('|');
        output("BS_3STATE");
        output('\n');
}

void generateAuto3stateControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("AUTOCHECKBOX ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        int g=generateCommonControlStyle(style);
        if(g) output('|');
        output("BS_AUTO3STATE");

        output('\n');
}

void generateGroupboxControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        outputIndent();
        output("GROUPBOX ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        int g=generateCommonControlStyle(style);
        if(g) output('|');
        output("DT_MNEMONIC");

        output('\n');
}

void generateUserbuttonControlStatement(const char *ctrl_text, const char *ctrl_id, unsigned long style, int x, int y, int w, int h) {
        displayWarning("BS_USERBUTTON found");
        outputIndent();

        output("CONTROL ");
        generateTIXYWH(ctrl_text,ctrl_id,x,y,w,h,1); output(',');
        output("WC_BUTTON,");
        int g=generateCommonControlStyle(style);
        if(g) output('|');
        output("BS_USERBUTTON");

        output('\n');
}
