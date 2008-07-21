/***
Filename: rbut.cpp
Description:
  Implementation of FRadioButton
Host:
  Watcom 10.5
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-05-05 Creation (migrated from PM)
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_RADIOBUTTON
#define BIFINCL_DYNMSG
#define WMSINCL_MSGS
#define WMSINCL_CPI
#include <bif.h>

#include "presparm.h"


//The FRadioButtonHandler is used to identify radiobuttons
WmsMSG FRadioButtonHandler::wm_isradiobutton=0;
FRadioButtonHandler::FRadioButtonHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        if(wm_isradiobutton==0)
                wm_isradiobutton=RegisterDynamicMessage("wm_isradiobutton");
        SetDispatch(dispatchFunc_t(FRadioButtonHandler::Dispatch));
}

Bool FRadioButtonHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==wm_isradiobutton) {
                ev.SetResult((WmsMRESULT)True);
                return True;
        } else
                return False;
}


//FRadioButton-----------------------------------------------------------------

Bool FRadioButton::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, Bool GroupStart) {
        current_state=0;
        uint32 flStyle=0;
        if(GroupStart) flStyle|= WS_GROUP;
        return FLowLevelButton::Create(pParent,ID,
                                       pszText,
                                       flStyle,rect);
}

/***
Visibility: public
Description:
        Return wether or not the readiobutton is checked (selected)
Return
        True  checked
        False not checked
***/
Bool FRadioButton::IsChecked() {
        return Bool(WmsSendMsg(GetHwnd(), BM_QUERYCHECK,0,0)!=0);
}

/***
Visibility: public
Description:
        Check (selected) the readiobutton
***/
Bool FRadioButton::Check() {
        WmsSendMsg(GetHwnd(), BM_SETCHECK, MPFROMUINT16(1),0);
        return True;
}

/***
Visibility: public
Description:
        Uncheck (deselected) the readiobutton
***/
Bool FRadioButton::UnCheck() {
        WmsSendMsg(GetHwnd(), BM_SETCHECK, MPFROMUINT16(0),0);
        return True;
}


/***
Visibility: public
Description:
        Return the checkindex of the group in which the radiobutton is
Return:
        >=0  check index
        -1   no selected radiobutton
***/
int FRadioButton::GetCheckIndex() {
        return UINT161FROMMR(WmsSendMsg(GetHwnd(), BM_QUERYCHECKINDEX, 0,0));
}


/***
Visibility: local to this module
Description:
        Return the first window in the group.
***/
static WmsHWND FindFirstInGroup(WmsHWND hwnd) {
        for(;;) {
                if(WmsQueryWindowStyle(hwnd)&WS_GROUP)
                        return hwnd;
                WmsHWND hwndPrev=hwnd;
                hwnd = WmsQueryWindow(hwnd,QW_PREVSIBLING);
                if(hwnd==0)
                        return hwndPrev;
        }
}

/***
Visibility: local to this module
Description
        Figure out wether or not a window is a radiobutton
***/
static Bool IsRadio(WmsHWND hwnd) {
        return (Bool)WmsSendMsg(hwnd, FRadioButtonHandler::wm_isradiobutton, 0,0);
}


/***
Visibility: public
Description:
        Check the i'th radiobutton in the group
***/
Bool FRadioButton::CheckIndex(int i) {
        int index=0;
        WmsHWND hwndCurrent=FindFirstInGroup(GetHwnd());
        for(;;) {
                if(IsRadio(hwndCurrent)) {
                        if(index==i) {
                                WmsSendMsg(hwndCurrent ,BM_SETCHECK, MPFROMUINT16(1),0);
                                return True;
                        } else
                                index++;
                }
                hwndCurrent=WmsQueryWindow(hwndCurrent,QW_NEXTSIBLING);
                if(hwndCurrent==0 ||                                    //no more controls
                   WmsQueryWindowStyle(hwndCurrent)&WS_GROUP)           //new group
                {
                        return False;        //radiobutton not found
                }
        }
}



Bool FRadioButton::GotFocus(FSetFocusEvent& ev) {
        //Ws have to check our selves
        WmsPostMsg(GetHwnd(), BM_SETCHECK, MPFROMUINT16(1),0);
        return FLowLevelButton::GotFocus(ev);
}

Bool FRadioButton::Paint(WmsHDC hdc, FRect *rect, const char *pszText, Bool isEnabled, Bool isDown, Bool hasFocus) {
        uint8 clr_foreground;
        uint8 clr_background;

        //erase background
        WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground,1);
        WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR, PP_DIALOGBACKGROUNDCOLOR, 0, &clr_background,1);
        CpiEraseRect(hdc, *rect, clr_foreground|clr_background);

        //draw image
        if(isEnabled) {
                if(isDown) {
                        WmsQueryPresParam(GetHwnd(), PP_HILITEFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_HILITEBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clr_background, 1);
                } else {
                        WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR, 0, 0, &clr_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
                }
        } else {
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clr_background, 1);
        }
        char image[3];
        if(current_state==0)
                WmsQueryPresParam(GetHwnd(), PP_RBUNSELECTED, 0, 0, image,3);
        else
                WmsQueryPresParam(GetHwnd(), PP_RBSELECTED, 0, 0, image,3);

        CpiDrawText(hdc, image, 3, (FRect*)rect, (color)clr_foreground, color(clr_background>>4), DT_LEFT);

        //draw label
        uint8 clrhot_foreground,clrhot_background;
        if(isEnabled) {
                if(hasFocus) {
                        WmsQueryPresParam(GetHwnd(), PP_FOCUSFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_FOCUSBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clr_background, 1);
                        clrhot_foreground=clr_foreground;
                        clrhot_background=clr_background;
                } else {
                        WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR, 0, 0, &clr_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
                        WmsQueryPresParam(GetHwnd(), PP_MNEMONICFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clrhot_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_MNEMONICBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clrhot_background, 1);
                }
        } else {
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clr_background, 1);
                clrhot_foreground=clr_foreground;
                clrhot_background=clr_background;
        }

        FRect r(*rect);
        r.SetLeft(r.GetLeft() + 4);
        CpiHotkeyTextout(hdc, r, pszText, '@', clr_foreground|clr_background, clrhot_foreground|clrhot_background);

        return True;
}

void FRadioButton::Click() {
        WmsSendMsg(GetHwnd(), BM_SETCHECK, MPFROMUINT16(1), 0);
}

unsigned FRadioButton::bm_querycheck() {
        return (unsigned)current_state;
}

unsigned FRadioButton::bm_querycheckindex() {
        int index=0;
        WmsHWND hwndCurrent=FindFirstInGroup(GetHwnd());
        for(;;) {
                if(IsRadio(hwndCurrent)) {
                        //it's a radiobutton
                        if(WmsSendMsg(hwndCurrent, BM_QUERYCHECK, 0,0)) {
                                return index;
                        } else
                                index++;
                }
                hwndCurrent=WmsQueryWindow(hwndCurrent,QW_NEXTSIBLING);
                if(hwndCurrent==0 ||                            //no more controls
                   WmsQueryWindowStyle(hwndCurrent)&WS_GROUP)   //new group
                {
                        return -1;      //no radiobutton checked
                }
        }
}

Bool FRadioButton::bm_setcheck(unsigned newstate) {
        if(newstate>1) return False;
        if(newstate==0) {
                if(current_state!=0) {
                        current_state = 0;
                        Invalidate();
                }
                return True;
        }

        //uncheck other radio (if any) first
        WmsHWND walk=GetHwnd();
        while(walk) {
                if(IsRadio(walk)) {
                        WmsSendMsg(walk, BM_SETCHECK, MPFROMUINT16(0),0);
                        if(WmsQueryWindowStyle(walk)&WS_TABSTOP) //clear tabstop
                                WmsSetWindowStyleBits(walk, 0, WS_TABSTOP);
                }
                if(WmsQueryWindowStyle(walk)&WS_GROUP) break;
                walk = WmsQueryWindow(walk,QW_PREVSIBLING);
        }
        walk = GetHwnd();
        while(walk) {
                if(IsRadio(walk)) {
                        WmsSendMsg(walk, BM_SETCHECK, MPFROMUINT16(0),0);
                        if(WmsQueryWindowStyle(walk)&WS_TABSTOP) //clear tabstop
                                WmsSetWindowStyleBits(walk, 0, WS_TABSTOP);
                }
                walk = WmsQueryWindow(walk,QW_NEXTSIBLING);
                if(walk && WmsQueryWindowStyle(walk)&WS_GROUP) break;
        }

        //now check ourselves and set a tabstop on us
        current_state = 1;
        WmsSetWindowStyleBits(GetHwnd(), WS_TABSTOP,WS_TABSTOP);
        Invalidate();

        return True;
}

