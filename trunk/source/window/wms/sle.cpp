/***
Filename: sle.cpp
Description:
  Implementation of FSLE (Single-Line-Edit)
Host:
  Watcom 10.6
History:
  ISJ 96-05-12 Creation
  ISJ 96-08-31 Added support for resource template and sle_password
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_SLE
#define BIFINCL_DYNMSG
#define WMSINCL_CPI
#define WMSINCL_MSGS
#define WMSINCL_LOGKEYS
#define WMSINCL_DESKTOP
#include <bif.h>
#include "bwclpbrd.h"
#include "presparm.h"
#include <string.h>


static WmsMSG em_textchanged=0;

FSLEHandler::FSLEHandler(FWnd *pwnd)
  : FHandler(pwnd)
{ 
        SetDispatch(dispatchFunc_t(FSLEHandler::Dispatch));
        if(em_textchanged==0)
                em_textchanged = RegisterDynamicMessage("em_textchanged");
}

Bool FSLEHandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_CUT:
                        ev.SetResult((WmsMRESULT)wm_cut());
                        return True;
                case WM_COPY:
                        ev.SetResult((WmsMRESULT)wm_copy());
                        return True;
                case WM_CLEAR:
                        ev.SetResult((WmsMRESULT)wm_clear());
                        return True;
                case WM_PASTE:
                        ev.SetResult((WmsMRESULT)wm_paste());
                        return True;
                case EM_SETTEXTLIMIT:
                        ev.SetResult((WmsMRESULT)em_settextlimit(UINT161FROMMP(ev.mp1)));
                        return True;
                case EM_QUERYFIRSTCHAR:
                        ev.SetResult((WmsMRESULT)em_queryfirstchar());
                        return True;
                case EM_SETFIRSTCHAR:
                        ev.SetResult((WmsMRESULT)em_setfirstchar(UINT161FROMMP(ev.mp1)));
                        return True;
                case EM_QUERYREADONLY:
                        ev.SetResult((WmsMRESULT)em_queryreadonly());
                        return True;
                case EM_SETREADONLY:
                        ev.SetResult((WmsMRESULT)em_setreadonly((Bool)UINT161FROMMP(ev.mp1)));
                        return True;
                case EM_QUERYSELECTION: {
                        int start;
                        int end;
                        em_queryselection(&start,&end);
                        ev.SetResult(MRFROM2UINT16(start,end));
                        return True;
                }
                case EM_SETSELECTION: {
                        ev.SetResult((WmsMRESULT)em_setselection(UINT161FROMMP(ev.mp1),UINT162FROMMP(ev.mp1)));
                        return True;
                }
                case WM_SETTEXT: {
                        WmsPostMsg(GetWnd()->GetHwnd(),em_textchanged,0,0);
                        return False;
                }
                default:
                        if(ev.GetID()==em_textchanged) {
                                text_changed();
                                return True;
                        } else
                                return False;
        }
}

Bool FSLE::Create(FWnd *pParent, int resID) {
        display_start=0;
        caret_pos=0;
        sel_start=sel_end=-1;
        textlimit=256;
        return FControl::Create(pParent,resID);
}

Bool FSLE::Create(FWnd *pParent, int ID, FRect *rect, long style, const char *pszText)
{
        uint32 flStyle=WS_VISIBLE;
        if(style&sle_password)
                flStyle|= sle_password;
        if(style&sle_autohscroll)
                flStyle|= sle_autohscroll;
        if(style&sle_readonly)
                flStyle|= sle_readonly;
        if(style&sle_border)
                flStyle|= sle_border;

        display_start=0;
        caret_pos=0;
        sel_start=sel_end=-1;
        textlimit=256;
        return FControl::Create(pParent,ID,
                                pszText?pszText:"",
                                flStyle,rect);
}

//clipboard operations
Bool FSLE::Cut() {
        return (Bool)WmsSendMsg(GetHwnd(),WM_CUT,0,0);
}

Bool FSLE::Copy() {
        return (Bool)WmsSendMsg(GetHwnd(),WM_COPY,0,0);
}
Bool FSLE::Clear() {
        return (Bool)WmsSendMsg(GetHwnd(),WM_CLEAR,0,0);
}

Bool FSLE::Paste() {
        return (Bool)WmsSendMsg(GetHwnd(),WM_PASTE,0,0);
}

//text operations
Bool FSLE::LimitText(int limit) {
        return (Bool)WmsSendMsg(GetHwnd(), EM_SETTEXTLIMIT, MPFROMUINT16(limit),0);
}


//readonly
Bool FSLE::IsReadonly() {
        return (Bool)WmsSendMsg(GetHwnd(), EM_QUERYREADONLY, 0,0);
}

Bool FSLE::SetReadonly(Bool f) {
        WmsSendMsg(GetHwnd(), EM_SETREADONLY, MPFROMUINT16(f),0);
        return True;
}


//selection
Bool FSLE::GetSelection(int *start, int *end) {
        WmsMRESULT rc=WmsSendMsg(GetHwnd(), EM_QUERYSELECTION, 0,0);
        *start = UINT161FROMMR(rc);
        *end = UINT162FROMMR(rc);
        return True;
}

Bool FSLE::SetSelection(int start, int end) {
        return (Bool)WmsSendMsg(GetHwnd(), EM_SETSELECTION, MPFROM2UINT16(start,end),0);
}


//---actual implementation---

void FSLE::updateCaretPos() {
        //scroll text so that the caret is visible
        //and set caret pos (if we have the focus)
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        if(WmsQueryWindowStyle(GetHwnd())&sle_autohscroll) {
                if(caret_pos<display_start) {
                        if(caret_pos==display_start-1)
                                display_start--;
                        else
                                display_start = caret_pos - (((r.GetWidth() -2) / 3) + 1);
                        if((int)display_start<0) display_start=0;
                        Invalidate();
                } else if(caret_pos >= display_start + r.GetWidth() - 2) {
                        if(caret_pos == display_start + r.GetWidth() - 2)
                                display_start++;
                        else
                                display_start = caret_pos - (((r.GetWidth() -2)*2 / 3) + 1);
                        if(display_start>GetTextLen())
                                display_start=GetTextLen();
                        Invalidate();
                }
        }

        if(HasFocus()) {
                //set caret pos
                if(caret_pos<display_start)
                        SetCaretPos(1,0);
                else if(caret_pos-display_start < r.GetWidth()-2)
                        SetCaretPos(1+caret_pos-display_start,0);
                else
                        SetCaretPos(1+r.GetWidth()-2,0);
        }
}

Bool FSLE::Paint(FPaintEvent &) {
        WmsHDC hdc=WmsBeginPaint(GetHwnd());
        if(!hdc) return False;

        FRect rect;
        WmsQueryWindowRect(GetHwnd(),&rect);
        char buf[256];
        GetText(buf,256);
        if((WmsQueryWindowStyle(GetHwnd())&sle_password) != 0)
                memset(buf,'*',strlen(buf));

        Bool isEnabled = (Bool)((WmsQueryWindowStyle(GetHwnd())&WS_DISABLED)==0);

        uint8 clr_foreground,clr_background;
        if(!isEnabled) {
                WmsQueryPresParam(GetHwnd(), PP_LEDISABLEDFOREGROUNDCOLOR, PP_LENORMALFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_LEDISABLEDBACKGROUNDCOLOR, PP_LENORMALBACKGROUNDCOLOR, 0, &clr_background, 1);
        } else if(HasFocus()) {
                WmsQueryPresParam(GetHwnd(), PP_LEFOCUSFOREGROUNDCOLOR, PP_LENORMALFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_LEFOCUSBACKGROUNDCOLOR, PP_LENORMALBACKGROUNDCOLOR, 0, &clr_background, 1);
        } else {
                WmsQueryPresParam(GetHwnd(), PP_LENORMALFOREGROUNDCOLOR, 0, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_LENORMALBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
        }

        //erase background
        CpiEraseRect(hdc, rect, clr_background|clr_foreground);

        //draw text
        FRect r=rect;
        r.xLeft++;
        r.xRight--;
        CpiDrawText(hdc, buf+display_start, -1, &r, color(clr_foreground), color(clr_background>>4), DT_LEFT);

        if(isEnabled && sel_start>=0) {
                //draw selected text
                WmsQueryPresParam(GetHwnd(), PP_LESELECTIONFOREGROUNDCOLOR, PP_LENORMALFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_LESELECTIONBACKGROUNDCOLOR, PP_LENORMALBACKGROUNDCOLOR, 0, &clr_background, 1);
                for(int p=sel_start; p<sel_end; p++) {
                        int x= 1+p-display_start;
                        if(x>=r.xLeft && x<r.xRight)
                                CpiSetCellAttr(hdc, FPoint(x,r.yTop), clr_foreground|clr_background);
                }
        }

        //draw scroll chars
        WmsQueryPresParam(GetHwnd(), PP_SLESCROLLFOREGROUNDCOLOR, PP_LENORMALFOREGROUNDCOLOR, 0, &clr_foreground, 1);
        WmsQueryPresParam(GetHwnd(), PP_SLESCROLLBACKGROUNDCOLOR, PP_LENORMALBACKGROUNDCOLOR, 0, &clr_background, 1);
        char scrollchars[2];
        WmsQueryPresParam(GetHwnd(), PP_SLESCROLLCHARS, 0, 0, scrollchars, 2);
        if(display_start>0)
                CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yTop), (uint16)(((uint16)(clr_background|clr_foreground))<<8) | scrollchars[0]);
        else
                CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yTop), (uint16)(((uint16)(clr_background|clr_foreground))<<8) | ' ');
        if(strlen(buf) - display_start > rect.GetWidth() - 2)
                CpiSetCellData(hdc, FPoint(rect.xRight-1,rect.yTop), (uint16)(((uint16)(clr_background|clr_foreground))<<8) | scrollchars[1]);
        else
                CpiSetCellData(hdc, FPoint(rect.xRight-1,rect.yTop), (uint16)(((uint16)(clr_background|clr_foreground))<<8) | ' ');

        WmsReleaseDC(hdc);

        return True;
}


Bool FSLE::LostFocus(FKillFocusEvent&) {
        HideCaret();
        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),EN_LOSTFOCUS), 0);
        return False;
}

Bool FSLE::GotFocus(FSetFocusEvent&) {
        insertMode=True;
        updateCaretPos();
        //todo: set caret shape?
        ShowCaret();
        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),EN_GOTFOCUS), 0);
        return False;
}

Bool FSLE::CharInput(FKeyboardEvent &ev) {
        if(WmsQueryWindowStyle(GetHwnd())&sle_readonly)
                return False;
        if(ev.GetChar()<32 && ev.IsCtrlDown()) {
                //ctrl-something
                //note: alt-nnn is inserted like real characters
                switch(ev.GetChar()) {
#ifndef CUA
                        case 'T'-'@': {
                                //delete word
                                char buf[256];
                                GetText(buf,256);
                                char *p=buf+caret_pos;
                                if(*p==' ') {
                                        while(*p==' ') p++;
                                } else {
                                        while(*p && *p!=' ') p++;
                                }
                                memmove(buf+caret_pos,p,strlen(p)+1);
                                if(sel_start>caret_pos) sel_start -= (p-buf)-caret_pos;
                                if(sel_end>caret_pos) sel_end -= (p-buf)-caret_pos;
                                SetText(buf);
                                return True;
                        }
                        case 'Y'-'@': //delete all
                                SetText("");
                                return True;
#else
                        case 'Z'-'@': //cut
                                wm_cut();
                                return True;
                        case 'X'-'@': //clear
                                wm_clear();
                                return True;
                        case 'C'-'@': //copy
                                wm_copy();
                                return True;
                        case 'V'-'@': //paste
                                wm_paste();
                                return True;
#endif
                        default:
                                return False;
                }
        } else {
#ifdef CUA
                wm_clear();
#endif
                char buf[256];
                GetText(buf,256);
                if(insertMode || caret_pos==strlen(buf)) {
                        if(strlen(buf)<textlimit) {
                                memmove(buf+caret_pos+1,buf+caret_pos,strlen(buf+caret_pos)+1);
                        } else
                                return True;
                }
                buf[caret_pos] = ev.GetChar();
                SetText(buf);
                caret_pos++;
                updateCaretPos();
                return True;
        }
}

Bool FSLE::KeyPressed(FKeyboardEvent &ev) {
        int org_caret_pos=caret_pos;
        switch(ev.GetLogicalKey()) {
                case lk_backspace: {
                        if(WmsQueryWindowStyle(GetHwnd())&sle_readonly) return False;
                        if(ev.IsCtrlDown() || ev.IsAltDown() || ev.IsShiftDown()) return False;
                        char buf[256];
                        GetText(buf,256);
                        if(caret_pos>0) {
                                memmove(buf+caret_pos-1,buf+caret_pos,strlen(buf+caret_pos)+1);
                                SetText(buf);
                                if(sel_start>=caret_pos) sel_start--;
                                if(sel_end>=caret_pos) sel_end--;
                                caret_pos--;
                                updateCaretPos();
                        }
                        return True;
                }
                case lk_delete: {
                        if(WmsQueryWindowStyle(GetHwnd())&sle_readonly) return False;
                        if(ev.IsAltDown()) return False;
                        if(ev.IsShiftDown()) {
                                wm_cut();
                        } else if(ev.IsCtrlDown()) {
                                wm_clear();
                        } else {
                                char buf[256];
                                GetText(buf,256);
                                if(caret_pos<strlen(buf)) {
                                        memmove(buf+caret_pos,buf+caret_pos+1,strlen(buf+caret_pos+1)+1);
                                        SetText(buf);
                                        if(sel_start>=caret_pos) sel_start--;
                                        if(sel_end>=caret_pos) sel_end--;
                                }
                        }
                        return True;
                }
                case lk_insert: {
                        if(ev.IsCtrlDown()) {
                                //copy
                                if(WmsQueryWindowStyle(GetHwnd())&sle_readonly) return False;
                                wm_copy();
                        } else if(ev.IsShiftDown()) {
                                //paste
                                if(WmsQueryWindowStyle(GetHwnd())&sle_readonly) return False;
                                wm_paste();
                        } else {
                                //toggle insert mode
                                insertMode = (Bool)!insertMode;
                                if(insertMode)
                                        SetCaretShape(InsertCaret);
                                else
                                        SetCaretShape(OverwriteCaret);
                        }
                        return True;
                }
                case lk_home: {
                        caret_pos = 0;
                        updateCaretPos();
                        break;
                }
                case lk_end: {
                        caret_pos = GetTextLen();
                        updateCaretPos();
                        break;
                }
                case lk_left: {
                        if(caret_pos>0) {
                                if(ev.IsCtrlDown()) {
                                        char buf[256];
                                        GetText(buf,256);
                                        char *p=buf+caret_pos;
                                        if(*p==' ') {
                                                while(p>buf && *p==' ') p--;
                                                while(p>buf && p[-1]!=' ') p--;
                                        } else {
                                                p--;
                                                while(p>buf && p[-1]!=' ') p--;
                                        }
                                        caret_pos = p-buf;
                                } else
                                        caret_pos--;
                                updateCaretPos();
                        }
                        break;
                }
                case lk_right: {
                        if(caret_pos<GetTextLen()) {
                                if(ev.IsCtrlDown()) {
                                        char buf[256];
                                        GetText(buf,256);
                                        char *p=buf+caret_pos;
                                        if(*p==' ') {
                                                while(*p==' ') p++;
                                        } else {
                                                while(*p && *p!=' ') p++;
                                                while(*p && *p==' ') p++;
                                        }
                                        caret_pos = p-buf;
                                } else
                                        caret_pos++;
                                updateCaretPos();
                        }
                        break;
                }
                default:
                        return False;
        }
        if(ev.IsShiftDown()) {
                //create selection/extend selection
                if(sel_start>=0) {
                        //extend selection
                        if(org_caret_pos==sel_start) {
                                sel_start=caret_pos;
                        } else if(org_caret_pos==sel_end) {
                                sel_end=caret_pos;
                        } else {
                                if(org_caret_pos<caret_pos)
                                        sel_start=org_caret_pos, sel_end=caret_pos;
                                else
                                        sel_end=org_caret_pos, sel_start=caret_pos;
                        }
                        if(sel_start==sel_end)
                                sel_start=-1,sel_end=-1;
                } else {
                        //create selection
                        if(org_caret_pos<caret_pos)
                                sel_start=org_caret_pos, sel_end=caret_pos;
                        else
                                sel_end=org_caret_pos, sel_start=caret_pos;
                }
                Invalidate();
        } else {
#ifdef CUA
                sel_start=sel_end=-1;
                Invalidate();
#endif
        }
        return True;
}

Bool FSLE::Enabled(FEnableEvent&) {
        Invalidate();
        return False;
}
Bool FSLE::Disabled(FEnableEvent&) {
        Invalidate();
        return False;
}

#define SLETIMER_LEFTSCROLL     0x8001
#define SLETIMER_RIGHTSCROLL    0x8002

Bool FSLE::Button1Down(FButtonDownEvent &ev) {
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        if(ev.GetX()==0 && ev.GetY()==0) {
                if(SetCapture())
                        StartTimer(100, SLETIMER_LEFTSCROLL);
                return True;
        } else if(ev.GetX()==r.GetRight()-1 && ev.GetY()==0) {
                if(SetCapture())
                        StartTimer(100, SLETIMER_RIGHTSCROLL);
                return True;
        } else {
                caret_pos=display_start + ev.GetX() - 1;
                if(caret_pos>GetTextLen()) caret_pos=GetTextLen();
                if(!HasFocus())
                        SetFocus();
                else
                        updateCaretPos();
                return False; //we don't mind other handlers getting this
        }
}

Bool FSLE::Button1Up(FButtonUpEvent &) {
        if(HasCapture()) {
                ReleaseCapture();
                StopTimer(SLETIMER_LEFTSCROLL);
                StopTimer(SLETIMER_RIGHTSCROLL);
                return True;
        } else
                return False;
}

Bool FSLE::Timer(FTimerEvent &ev) {
        if(ev.GetTimerID()==SLETIMER_LEFTSCROLL) {
                FPoint p=ev.pt;
                WmsMapWindowPoints(HWND_DESKTOP,GetHwnd(),&p,1);
                if(p.x==0 && p.y==0) {
                        if(caret_pos>0) {
                                caret_pos--;
                                updateCaretPos();
                        }
                }
                return True;
        } else if(ev.GetTimerID()==SLETIMER_RIGHTSCROLL) {
                FPoint p=ev.pt;
                WmsMapWindowPoints(HWND_DESKTOP,GetHwnd(),&p,1);
                FRect r;
                WmsQueryWindowRect(GetHwnd(),&r);
                if(p.y==0 && p.x==r.xRight-1) {
                        if(caret_pos<GetTextLen()) {
                                caret_pos++;
                                updateCaretPos();
                        }
                }
                return True;
        } else
                return False;
}

Bool FSLE::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"password")==0) {
                        SetStyle(~(long)sle_password,sle_password);
                } else if(strcmp(name[p],"autohscroll")==0) {
                        SetStyle(~(long)sle_autohscroll,sle_autohscroll);
                } else if(strcmp(name[p],"readonly")==0) {
                        SetStyle(~(long)sle_readonly,sle_readonly);
                } else if(strcmp(name[p],"border")==0) {
                        SetStyle(~(long)sle_border,sle_border);
                } else if(strcmp(name[p],"text")==0 && value[p]) {
                        SetText(value[p]);
                }
        }
        return True;
}

Bool FSLE::wm_cut() {
        if(wm_copy())
                return wm_clear();
        else
                return False;
}

Bool FSLE::wm_copy() {
        if(sel_start>=0) {
                char buf[256];
                if(GetText(buf,256)) {
                        if(WmsClipboard.SetText(buf+sel_start, sel_end-sel_start) == 0)
                                return True;
                }
        }
        return False;
}

Bool FSLE::wm_clear() {
        if(sel_start>=0) {
                char buf[256];
                if(GetText(buf,256)) {
                        //remove selected text
                        memmove(buf+sel_start, buf+sel_end, strlen(buf+sel_end)+1);
                        SetText(buf);
                        if(caret_pos>sel_start) {
                                if(caret_pos>sel_end)
                                        caret_pos -= sel_end-sel_start;
                                else
                                        caret_pos = sel_start;
                                updateCaretPos();
                        }
                        sel_start = sel_end = -1;
                        return True;
                }
        }
        return False;
}

Bool FSLE::wm_paste() {
        char clipbuf[256];
        unsigned clipchars;
        if(WmsClipboard.GetText(clipbuf,256,&clipchars) !=0)
                return False; //nothing in clipboard
        char buf[256];
        GetText(buf,256);
#ifdef CUA
        if(strlen(buf) - (sel_end-sel_start) + clipchars > textlimit)
                return False;   //text would exceed textlimit
        if(sel_start>=0) {
                //remove selected text
                memmove(buf+sel_start, buf+sel_end, strlen(buf+sel_end)+1);
                if(caret_pos>sel_start) {
                        if(caret_pos>sel_end)
                                caret_pos -= sel_end-sel_start;
                        else
                                caret_pos = sel_start;
                }
                sel_start = sel_end = -1;
        }
#else
        if(strlen(buf) + clipchars > textlimit)
                return False;   //text would exceed textlimit
#endif
        //insert text at caret_pos
        memmove(buf+caret_pos+clipchars, buf+caret_pos, strlen(buf+caret_pos)+1);
        memcpy(buf+caret_pos,clipbuf,clipchars);
        caret_pos += clipchars;
        SetText(buf);

        updateCaretPos();

        return True;
}

Bool FSLE::em_settextlimit(unsigned limit) {
        if(limit>0) {
                textlimit = limit;
                return True;
        } else
                return False;
}

unsigned FSLE::em_queryfirstchar() {
        return display_start;
}

Bool FSLE::em_setfirstchar(unsigned pos) {
        char buf[256];
        GetText(buf,256);
        if(pos>strlen(buf)+1)
                return False;
        display_start = pos;
        caret_pos = pos;
        updateCaretPos();
        Invalidate();
        return True;
}

Bool FSLE::em_queryreadonly() {
        return (Bool)((WmsQueryWindowStyle(GetHwnd())&sle_readonly)!=0);
}

Bool FSLE::em_setreadonly(Bool f) {
        if(f)
                WmsSetWindowStyleBits(GetHwnd(),sle_readonly,sle_readonly);
        else
                WmsSetWindowStyleBits(GetHwnd(),0,sle_readonly);
        return True;
}

void FSLE::em_queryselection(int *start, int *end) {
        *start = sel_start;
        * end = sel_end;
}

Bool FSLE::em_setselection(int start, int end) {
        if(start<-1) return False;
        if(start==-1) {
                sel_start=sel_end=-1;
                Invalidate();
                return True;
        }
        char buf[256];
        GetText(buf,256);
        if(start>strlen(buf))
                return False;
        if(start>end && end!=-1)
                return False;
        sel_start = start;
        if(end==-1) 
                sel_end = strlen(buf);
        else
                sel_end = end;
#ifdef CUA
        caret_pos = sel_end;
#endif
        updateCaretPos();
        return True;
}

void FSLE::text_changed() {
        int l=GetTextLen();
        if(caret_pos>l) caret_pos = 0;
        sel_start=sel_end=-1;
        if(display_start>l) display_start=0;
        updateCaretPos();
        Invalidate();
        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),EN_CHANGE), 0);
}

