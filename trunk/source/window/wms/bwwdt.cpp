/***
Filename: bwwdt.cpp
Description:
  Implementation of a desktop for WMS
Host:
  Watcom 10.5-10.6
History:
  ISJ 96-04-21 Created
  ISJ 96-09-21 Added support for the switchlist
***/
#define BIFINCL_WINDOW
#define BIFINCL_CLIENTHANDLERS
#define BIFINCL_MOUSEHANDLERS
#define WMSINCL_DESKTOP
#define WMSINCL_BW
#define WMSINCL_CPI
#define WMSINCL_SYSVALUE
#define WMSINCL_MSGS
#define WMSINCL_FOCUS

#include <bif.h>

#include "presparm.h"

int FDesktop::GetWidth() {
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        return r.GetWidth();
}

int FDesktop::GetHeight() {
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        return r.GetHeight();
}



class ActualDesktopHandler_internal : public FHandler {
public:
        ActualDesktopHandler_internal(FWnd *wnd)
          : FHandler(wnd)
          { SetDispatch(dispatchFunc_t(ActualDesktopHandler_internal::Dispatch)); }
        Bool Dispatch(FEvent &ev);
};

Bool ActualDesktopHandler_internal::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_DESTROY: {
                        WmsSetDesktop(0);
                        return False;
                }
        }
        return False;
};

class ActualDesktopClass : public FDesktop,
                           public FPaintHandler,
                           public FMouseHandler
{
        ActualDesktopHandler_internal aeh;
public:
        ActualDesktopClass()
          : FDesktop(),
            aeh(this),
            FPaintHandler(this),
            FMouseHandler(this)
            {}

        Bool Create();

        Bool Paint(FPaintEvent&);

        Bool ButtonDown(FButtonDownEvent &ev);
};

Bool ActualDesktopClass::Create() {
        FRect r;
        r.Set(0,0,0,0);
        r.SetWidth((int)WmsQuerySysValue(0,SV_CXSCREEN));
        r.SetHeight((int)WmsQuerySysValue(0,SV_CYSCREEN));
        if(!FWnd::Create(0,                             //no parent
                         0,                             //no name
                         WS_VISIBLE|WS_CLIPCHILDREN,    //style
                         &r,
                         0,                             //no owner
                         HWND_TOP,
                         0                              //no ID
                        ))
                return False;

        //Tell WMS that we are the desktop
        WmsSetDesktop(GetHwnd());

        //Setup presentation parameters (lots of them)
        WmsHWND hwnd=GetHwnd();
        unsigned clr;
        if((CpiQueryCapabilities()&CPICAP_ATTRIBUTEFORMAT)==CPICAP_ATTRIBUTECOLOR1616) {
                clr=clr_lightgray;
                WmsSetPresParam(hwnd, PP_DESKTOPFOREGROUNDCOLOR,&clr,1);
                clr=clr_darkgray<<4;
                WmsSetPresParam(hwnd, PP_DESKTOPBACKGROUNDCOLOR,&clr,1);
                WmsSetPresParam(hwnd, PP_DESKTOPFILLCHAR," ",1);
        } else {
                clr=clr_darkgray;
                WmsSetPresParam(hwnd, PP_DESKTOPFOREGROUNDCOLOR,&clr,1);
                clr=clr_lightgray<<4;
                WmsSetPresParam(hwnd, PP_DESKTOPBACKGROUNDCOLOR,&clr,1);
                WmsSetPresParam(hwnd, PP_DESKTOPFILLCHAR,"Û",1);
        }

        clr=clr_blue<<4|clr_white;
        WmsSetPresParam(hwnd, PP_ACTIVEBORDERCOLOR,&clr,1);
        clr=clr_blue<<4|clr_lightgray;
        WmsSetPresParam(hwnd, PP_INACTIVEBORDERCOLOR,&clr,1);
        clr=clr_white;
        WmsSetPresParam(hwnd, PP_ACTIVETEXTFOREGROUNDCOLOR,&clr,1);
        clr=clr_cyan<<4;
        WmsSetPresParam(hwnd, PP_ACTIVETEXTBACKGROUNDCOLOR,&clr,1);
        clr=clr_lightgray;
        WmsSetPresParam(hwnd, PP_INACTIVETEXTFOREGROUNDCOLOR,&clr,1);
        clr=clr_blue<<4;
        WmsSetPresParam(hwnd, PP_INACTIVETEXTBACKGROUNDCOLOR,&clr,1);
        clr=clr_lightgreen;
        WmsSetPresParam(hwnd, PP_TITLEBUTTONFOREGROUNDCOLOR,&clr,1);
        clr=clr_blue<<4;
        WmsSetPresParam(hwnd, PP_TITLEBUTTONBACKGROUNDCOLOR,&clr,1);
        //WmsSetPresParam(hwnd, PP_THINBORDERCHARS, "\332\304\277\263\263\300\304\331",8);
        //WmsSetPresParam(hwnd, PP_SIZEBORDERCHARS, "\331\315\273\272\272\310\315\274",8);
        //WmsSetPresParam(hwnd, PP_MODALBORDERCHARS,"\333\333\333\333\333\333\333\333",8);
        WmsSetPresParam(hwnd, PP_THINBORDERCHARS, "ÚÄ¿³³ÀÄÙ",8);
        WmsSetPresParam(hwnd, PP_SIZEBORDERCHARS, "ÉÍ»ººÈÍ¼",8);
        WmsSetPresParam(hwnd, PP_MODALBORDERCHARS,"ÛßÛÝÞÛÜÛ",8);
        WmsSetPresParam(hwnd, PP_TITLEBUTTONMAXCHARS,"[]",3);
        WmsSetPresParam(hwnd, PP_TITLEBUTTONMINCHARS,"[]",3);
        WmsSetPresParam(hwnd, PP_TITLEBUTTONHIDECHARS,"[#]",3);
        WmsSetPresParam(hwnd, PP_TITLEBUTTONRESTORECHARS,"[]",3);
        WmsSetPresParam(hwnd, PP_TITLEBUTTONSYSMENUCHARS, "[þ]",3);
        clr=clr_black;
        WmsSetPresParam(hwnd, PP_DEFAULTFOREGROUNDCOLOR,&clr,1);
        clr=clr_lightgray<<4;
        WmsSetPresParam(hwnd, PP_DEFAULTBACKGROUNDCOLOR,&clr,1);
        clr=clr_darkgray;
        WmsSetPresParam(hwnd, PP_DISABLEDFOREGROUNDCOLOR,&clr,1);
        clr=clr_lightgray<<4;
        WmsSetPresParam(hwnd, PP_DISABLEDBACKGROUNDCOLOR,&clr,1);
        clr=clr_white;
        WmsSetPresParam(hwnd, PP_FOCUSFOREGROUNDCOLOR,&clr,1);
        clr=clr_lightgray<<4;
        WmsSetPresParam(hwnd, PP_FOCUSBACKGROUNDCOLOR,&clr,1);
        clr=clr_lightcyan;
        WmsSetPresParam(hwnd, PP_HILITEFOREGROUNDCOLOR,&clr,1);
        clr=clr_lightgray<<4;
        WmsSetPresParam(hwnd, PP_HILITEBACKGROUNDCOLOR,&clr,1);
        clr=clr_yellow;
        WmsSetPresParam(hwnd, PP_MNEMONICFOREGROUNDCOLOR,&clr,1);
        //PP_MNEMONICBACKGROUNDCOLOR not set, defaults to PP_DEFAULTBACKGROUNDCOLOR

        //PP_BUTTONMIDDLEFOREGROUNDCOLOR not set. defaults to default/hilite/disabled
        clr=clr_green<<4;
        WmsSetPresParam(hwnd, PP_BUTTONMIDDLEBACKGROUNDCOLOR,&clr,1);
        //PP_BUTTONSHADOWBACKGROUNDCOLOR not set. defaults to dlgbackground
        clr=clr_darkgray;
        WmsSetPresParam(hwnd, PP_BUTTONSHADOWFOREGROUNDCOLOR,&clr,1);
        WmsSetPresParam(hwnd, PP_BUTTONSHADOWCHARS,"Üßß",3);
        clr=clr_lightblue;
        WmsSetPresParam(hwnd, PP_BUTTONDEFAULTFOREGROUNDCOLOR,&clr,1);
        //PP_BUTTONDEFAULTBACKGROUNDCOLOR not set. defaults to middelbackground
        WmsSetPresParam(hwnd, PP_BUTTONDEFAULTLEFTCHAR,"",1);

        WmsSetPresParam(hwnd, PP_BUTTONDEFAULTRIGHTCHAR,"",1);
        clr=clr_yellow;
        WmsSetPresParam(hwnd, PP_BUTTONMNEMONICFOREGROUNDCOLOR,&clr,1);
        clr=clr_green<<4;
        WmsSetPresParam(hwnd, PP_BUTTONMNEMONICBACKGROUNDCOLOR,&clr,1);

        clr=clr_lightgray<<4;
        WmsSetPresParam(hwnd, PP_DIALOGBACKGROUNDCOLOR,&clr,1);

        WmsSetPresParam(hwnd, PP_CBUNCHECKED,   "[ ]",3);
        WmsSetPresParam(hwnd, PP_CBCHECKED,     "[x]",3);
        WmsSetPresParam(hwnd, PP_CBUNDETERMINED,"[?]",3);
        WmsSetPresParam(hwnd, PP_RBUNSELECTED,  "( )",3);
        WmsSetPresParam(hwnd, PP_RBSELECTED,    "()",3);

        WmsSetPresParam(hwnd, PP_GBBORDERCHARS, "ÚÄ¿³³ÀÄÙ",8);

        clr=clr_black;
        WmsSetPresParam(hwnd, PP_LBNORMALFOREGROUNDCOLOR,&clr,1);
        clr=clr_cyan<<4;
        WmsSetPresParam(hwnd, PP_LBNORMALBACKGROUNDCOLOR,&clr,1);
        clr=clr_yellow;
        WmsSetPresParam(hwnd, PP_LBSELECTEDFOREGROUNDCOLOR,&clr,1);
        clr=clr_cyan<<4;
        WmsSetPresParam(hwnd, PP_LBSELECTEDBACKGROUNDCOLOR,&clr,1);
        clr=clr_white;
        WmsSetPresParam(hwnd, PP_LBFOCUSFOREGROUNDCOLOR,&clr,1);
        clr=clr_green<<4;
        WmsSetPresParam(hwnd, PP_LBFOCUSBACKGROUNDCOLOR,&clr,1);
        clr=clr_yellow;
        WmsSetPresParam(hwnd, PP_LBSELECTEDFOCUSFOREGROUNDCOLOR,&clr,1);
        clr=clr_green<<4;
        WmsSetPresParam(hwnd, PP_LBSELECTEDFOCUSBACKGROUNDCOLOR,&clr,1);

        clr=clr_yellow;
        WmsSetPresParam(hwnd, PP_LENORMALFOREGROUNDCOLOR, &clr,1);
        clr=clr_blue<<4;
        WmsSetPresParam(hwnd, PP_LENORMALBACKGROUNDCOLOR, &clr,1);
        clr=clr_white;
        WmsSetPresParam(hwnd, PP_LESELECTIONFOREGROUNDCOLOR, &clr,1);
        clr=clr_green<<4;
        WmsSetPresParam(hwnd, PP_LESELECTIONBACKGROUNDCOLOR, &clr,1);
        //PP_LEFOCUSFOREGROUNDCOLOR not set, default to normal color
        //PP_LEFOCUSBACKGROUNDCOLOR ditto
        clr=clr_lightgray;
        WmsSetPresParam(hwnd, PP_LEDISABLEDFOREGROUNDCOLOR, &clr,1);
        clr=clr_blue<<4;
        WmsSetPresParam(hwnd, PP_LEDISABLEDBACKGROUNDCOLOR, &clr,1);
        clr=clr_lightgreen;
        WmsSetPresParam(hwnd, PP_SLESCROLLFOREGROUNDCOLOR, &clr,1);
        //PP_SLESCROLLBACKGROUNDCOLOR not set, defaults to normal color
        WmsSetPresParam(hwnd, PP_SLESCROLLCHARS, "",2);

        WmsSetPresParam(hwnd, PP_SBCHARS, "±", 7);
        clr=clr_cyan;
        WmsSetPresParam(hwnd, PP_SBSCROLLFOREGROUNDCOLOR, &clr,1);
        clr=clr_blue<<4;
        WmsSetPresParam(hwnd, PP_SBSCROLLBACKGROUNDCOLOR, &clr,1);
        clr=clr_blue;
        WmsSetPresParam(hwnd, PP_SBTRACKFOREGROUNDCOLOR, &clr,1);
        clr=clr_cyan<<4;
        WmsSetPresParam(hwnd, PP_SBTRACKBACKGROUNDCOLOR, &clr,1);
        clr=clr_cyan;
        WmsSetPresParam(hwnd, PP_THUMBFOREGROUNDCOLOR, &clr,1);
        clr=clr_blue<<4;
        WmsSetPresParam(hwnd, PP_THUMBBACKGROUNDCOLOR, &clr,1);
        clr=clr_darkgray;
        WmsSetPresParam(hwnd, PP_SBDISABLEDSCROLLFOREGROUNDCOLOR, &clr,1);
        clr=clr_blue<<4;
        WmsSetPresParam(hwnd, PP_SBDISABLEDSCROLLBACKGROUNDCOLOR, &clr,1);
        clr=clr_lightgray;
        WmsSetPresParam(hwnd, PP_SBDISABLEDTRACKFOREGROUNDCOLOR, &clr,1);
        clr=clr_cyan;
        WmsSetPresParam(hwnd, PP_SBDISABLEDTRACKBACKGROUNDCOLOR, &clr,1);
        //combobox
        clr = clr_white;
        WmsSetPresParam(hwnd, PP_CBBUTTONFOREGROUNDCOLOR, &clr,1);
        clr = clr_blue<<4;
        WmsSetPresParam(hwnd, PP_CBBUTTONBACKGROUNDCOLOR, &clr,1);
        WmsSetPresParam(hwnd, PP_CBDROPDOWNCHARS, "", 2);
        WmsSetPresParam(hwnd, PP_CBROLLUPCHARS, "", 2);
        //menu
        WmsSetPresParam(hwnd, PP_MBORDERCHARS, "ÚÄ¿³³ÀÄÙ",8);
        WmsSetPresParam(hwnd, PP_MSUBMENUCHAR, "",1);
        WmsSetPresParam(hwnd, PP_MCONDITIONALCASCADECHAR, "",1);
        if(CpiQueryCapabilities()&CPICAP_FULLBOXDRAW)
                WmsSetPresParam(hwnd, PP_MCHECKCHAR, "û",1);
        else
                WmsSetPresParam(hwnd, PP_MCHECKCHAR, "Ï",1);
        WmsSetPresParam(hwnd, PP_MSEPARATORCHAR, "Ä",1);
        clr=clr_lightgray<<4;
        WmsSetPresParam(hwnd, PP_MBACKGROUNDCOLOR, &clr,1);
        clr=clr_black;
        WmsSetPresParam(hwnd, PP_MFOREGROUNDCOLOR, &clr,1);
        //PP_MBORDERFOREGROUNDCOLOR, PP_MBORDERBACKGROUNDCOLOR,
        //PP_MNORMALFOREGROUNDCOLOR, PP_MNORMALBACKGROUNDCOLOR defaults to the 
        //two above
        clr=clr_red;
        WmsSetPresParam(hwnd, PP_MHOTKEYFOREGROUNDCOLOR, &clr,1);
        //PP_MHOTKEYBACKGROUNDCOLOR defaults to PP_MBACKGROUNDCOLOR
        clr=clr_darkgray;
        WmsSetPresParam(hwnd, PP_MDISABLEDFOREGROUNDCOLOR, &clr,1);
        //PP_MDISABLEDBACKGROUNDCOLOR defaults to PP_MBACKGROUNDCOLOR
        clr=clr_black;
        WmsSetPresParam(hwnd, PP_MSELECTEDFOREGROUNDCOLOR, &clr,1);
        clr=clr_green<<4;
        WmsSetPresParam(hwnd, PP_MSELECTEDBACKGROUNDCOLOR, &clr,1);
        //PP_MSTATICFOREGROUNDCOLOR and PP_MSTATICBACKGROUNDCOLOR defaults to
        //default menu background

        //Set focus to the desktop
        WmsChangeFocus(hwnd,hwnd,0);
        return True;
}


Bool ActualDesktopClass::Paint(FPaintEvent&) {
        FRect pr;
        WmsHDC hdc=WmsBeginPaint(GetHwnd(),&pr);
        if(hdc) {
                //FRect r;
                //WmsQueryWindowRect(GetHwnd(),&r);
                uint8 fg_clr,bg_clr;
                WmsQueryPresParam(GetHwnd(),PP_DESKTOPFOREGROUNDCOLOR,0,0,&fg_clr,1);
                WmsQueryPresParam(GetHwnd(),PP_DESKTOPBACKGROUNDCOLOR,0,0,&bg_clr,1);
                uint8 fc;
                WmsQueryPresParam(GetHwnd(),PP_DESKTOPFILLCHAR,0,0,&fc,1);
                CpiFillRect(hdc,pr,fg_clr|bg_clr,fc);
                WmsEndPaint(hdc);
                return True;
        } else
                return False;
}

Bool ActualDesktopClass::ButtonDown(FButtonDownEvent &ev) {
        if((ev.GetButton()==1 && ev.IsButtonDown(2)) ||
           (ev.GetButton()==2 && ev.IsButtonDown(1))) {
                WmsPostMsg(GetHwnd(), WM_SHOWSWITCHLIST, MPFROMUINT16(True), 0);
                return True;
        } else
                return False;
}

static FDesktop *theDesktop;

Bool BWWInitializeDesktop() {
        //assumption: FWndMan has initialized enough
        ActualDesktopClass *d=new ActualDesktopClass;
        if(d) {
                if(d->Create()) {
                        theDesktop=d;
                        return True;
                } else {
                        delete d;
                        return False;
                }
        } else
                return False;
}

void BWWTerminateDesktop() {
        delete theDesktop;
        theDesktop=0;
}

FDesktop * BIFFUNCTION GetDesktop() {
        return theDesktop;
}


//todo: handle menu -> desktop menu

