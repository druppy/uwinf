/***
Filename: lowlbut.cpp
Description:
  Implementation of FLowLevelButton, a basic button class intended as 
  superclass for radiobuttons, checkboxes and pushbuttons
Host:
  Watcom 10.5
History:
  ISJ 96-05-05 Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_LOWLEVELBUTTON
#define WMSINCL_CPI
#define WMSINCL_MSGS
#include <bif.h>
#include <string.h>
#include <ctype.h>


Bool FLowLevelButtonHandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case BM_QUERYCHECK:
                        ev.SetResult(MRFROMUINT16(bm_querycheck()));
                        return True;
                case BM_QUERYCHECKINDEX:
                        ev.SetResult(MRFROMUINT16(bm_querycheckindex()));
                        return True;
                case BM_SETCHECK:
                        ev.SetResult((WmsMRESULT)bm_setcheck(UINT161FROMMP(ev.GetMP1())));
                        return True;
                case BM_CLICK:
                        bm_click();
                        ev.SetResult((WmsMRESULT)True);
                        return True;
                case BM_QUERYDEFAULT:
                        ev.SetResult(MRFROMUINT16(bm_querydefault()));
                        return True;
                case BM_SETDEFAULT:
                        ev.SetResult((WmsMRESULT)bm_setdefault((Bool)UINT161FROMMP(ev.GetMP1())));
                        return True;
                default:
                        return False;
        }
}

Bool FLowLevelButton::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"text")==0 && value[p]) {
                        SetText(value[p]);
                }
        }
        return True;
}


Bool FLowLevelButton::Paint() {
        WmsHDC hdc=WmsBeginPaint(GetHwnd(),0);
        if(hdc) {
                FRect rect;
                WmsQueryWindowRect(GetHwnd(), &rect);
                char text[160];
                GetText(text,160);
                Bool isEnabled = (Bool)((WmsQueryWindowStyle(GetHwnd())&WS_DISABLED)==0);
                Bool b=Paint(hdc, &rect, text, isEnabled, isDown, HasFocus());
                WmsEndPaint(hdc);
                return b;
        } else
                return False;
}

//character interface
Bool FLowLevelButton::CharInput(FKeyboardEvent& ev) {
        if(ev.GetChar()==' ') {
                Click();
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),BN_CLICK), 0);
                return True;
        } else
                return False;
}

//mouse interface
Bool FLowLevelButton::Button1Down(FButtonDownEvent &ev) {
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        GetDownRect(&r);
        if(r.Contains(ev.GetPoint())) {
                if(SetCapture()) {
                        isDown=True;
                        Invalidate();
                }
        }
        return False; //we don't mind other getting this message
}

Bool FLowLevelButton::Button1Up(FButtonUpEvent&) {
        if(HasCapture())
                ReleaseCapture();
        if(isDown) {
                isDown=False;
                SetFocus();
                Invalidate();
                Click();
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),BN_CLICK), 0);
        }
        return False; //we don't mind other getting this message
}

Bool FLowLevelButton::MouseMove(FMouseMoveEvent& ev) {
        if(HasCapture()) {
                FRect r;
                WmsQueryWindowRect(GetHwnd(),&r);
                GetDownRect(&r);
                if(r.Contains(ev.GetPoint())) {
                        if(!isDown) {
                                isDown=True;
                                Invalidate();
                        }
                } else {
                        if(isDown) {
                                isDown=False;
                                Invalidate();
                        }
                }
        }
        return False; //we don't mind other getting this message
}

//focus invalidation
Bool FLowLevelButton::LostFocus(FKillFocusEvent&) {
        //notify owner
        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),BN_GOTFOCUS), 0);
        //focus should be repainted
        Invalidate();
        return False;
}

Bool FLowLevelButton::GotFocus(FSetFocusEvent&) {
        //notify owner
        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),BN_LOSTFOCUS), 0);
        //focus should be repainted
        Invalidate();
        return False;
}

void FLowLevelButton::bm_click() {
        Click();
        //notify owner
        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),BN_CLICK), 0);
}

Bool FLowLevelButton::MatchMnemonic(char c) {
        char text[256];
        GetText(text,256);
        char *p=strchr(text,'@');
        if(p)
                return (Bool)(tolower(p[1])==tolower(c));
        else
                return False;
}


