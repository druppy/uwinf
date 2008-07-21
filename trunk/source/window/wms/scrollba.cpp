/***
Filename: scrollba.cpp
Description:
  Implementation of FScrollBar
Host:
  Watcom 10.6
History:
  ISJ 96-05-18 Creation
  ISJ 96-08-31 Added support for resource template
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_SCROLLBAR
#define WMSINCL_MSGS
#define WMSINCL_CPI
#include <bif.h>

#include "presparm.h"
#include <string.h>

#define TID_ONELESS     0x8001
#define TID_ONEMORE     0x8002

Bool FScrollBarHandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case SBM_QUERYPOS:
                        ev.SetResult(MRFROMUINT32(sbm_querypos()));
                        return True;
                case SBM_SETPOS:
                        ev.SetResult((WmsMRESULT)sbm_setpos((long)UINT32FROMMP(ev.mp1)));
                        return True;
                case SBM_QUERYRANGESTART:
                        ev.SetResult((WmsMRESULT)sbm_queryrangestart());
                        return True;
                case SBM_QUERYRANGEEND:
                        ev.SetResult((WmsMRESULT)sbm_queryrangeend());
                        return True;
                case SBM_SETRANGE:
                        ev.SetResult((WmsMRESULT)sbm_setrange((long)UINT32FROMMP(ev.mp1),(long)UINT32FROMMP(ev.mp2)));
                        return True;
                case SBM_SETTHUMBSIZE:
                        ev.SetResult((WmsMRESULT)sbm_setthumbsize((long)UINT32FROMMP(ev.mp1),(long)UINT32FROMMP(ev.mp2)));
                        return True;
                default:
                        return False;
        }
}

void FScrollBar::doOneLess() {
        if(current_pos>range_start) {
                current_pos--;
                WmsInvalidateRect(GetHwnd(),0,False);
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_ONELESS), MPFROMUINT32(current_pos));
        }
}
void FScrollBar::doOneMore() {
        if(current_pos<range_end) {
                current_pos++;
                WmsInvalidateRect(GetHwnd(),0,False);
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_ONEMORE), MPFROMUINT32(current_pos));
        }
}
void FScrollBar::doMuchLess() {
        if(current_pos>range_start) {
                current_pos -= (range_end-range_start)*amount_visible/total_amount;
                if(current_pos<range_start) current_pos=range_start;
                WmsInvalidateRect(GetHwnd(),0,False);
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_MUCHLESS), MPFROMUINT32(current_pos));
        }
}
void FScrollBar::doMuchMore() {
        if(current_pos<range_end) {
                current_pos += (range_end-range_start)*amount_visible/total_amount;
                if(current_pos>range_end) current_pos=range_end;
                WmsInvalidateRect(GetHwnd(),0,False);
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_MUCHMORE), MPFROMUINT32(current_pos));
        }
}

Bool FScrollBar::Create(FWnd *pParent, int resID) {
        range_start=0;
        range_end=100;
        amount_visible=10;
        total_amount=100;
        current_pos=0;
        return FControl::Create(pParent,resID);
}

Bool FScrollBar::Create(FWnd *pParent, int ID,
                        scrollbar_orientation orientation,
                        FRect *rect
                       )
{
        range_start=0;
        range_end=100;
        amount_visible=10;
        total_amount=100;
        current_pos=0;
        return FControl::Create(pParent,ID,"",orientation|WS_GROUP|WS_TABSTOP,rect);
 
}

long FScrollBar::GetScrollPos() {
        return (long)WmsSendMsg(GetHwnd(), SBM_QUERYPOS, 0,0);
}

Bool FScrollBar::SetScrollPos(long pos) {
        return (Bool)WmsSendMsg(GetHwnd(), SBM_SETPOS, MPFROMUINT32(pos), 0);
}

Bool FScrollBar::GetScrollRange(long *start, long *end) {
        if(start) *start = (long)WmsSendMsg(GetHwnd(), SBM_QUERYRANGESTART, 0,0);
        if(end) *end = (long)WmsSendMsg(GetHwnd(), SBM_QUERYRANGEEND, 0,0);
        return True;
}

Bool FScrollBar::SetScrollRange(long start, long end) {
        return (Bool)WmsSendMsg(GetHwnd(), SBM_SETRANGE, MPFROMUINT32(start), MPFROMUINT32(end));
}

Bool FScrollBar::SetThumbSize(long visible, long total) {
        return (Bool)WmsSendMsg(GetHwnd(), SBM_SETTHUMBSIZE, MPFROMUINT32(visible), MPFROMUINT32(total));
}

Bool FScrollBar::Paint() {
        WmsHDC hdc=WmsBeginPaint(GetHwnd());
        if(!hdc) return False;
        FRect rect;
        WmsQueryWindowRect(GetHwnd(),&rect);

        scrollbar_orientation orient=(scrollbar_orientation)(WmsQueryWindowStyle(GetHwnd())&0x1);

        char sbchars[7];
        WmsQueryPresParam(GetHwnd(), PP_SBCHARS, 0,0, sbchars,7);

        uint8 clr_enabledforeground, clr_enabledbackground;
        uint8 clr_disabledforeground, clr_disabledbackground;

        //paint scrollbars
        WmsQueryPresParam(GetHwnd(), PP_SBSCROLLFOREGROUNDCOLOR, 0,0,&clr_enabledforeground,1);
        WmsQueryPresParam(GetHwnd(), PP_SBSCROLLBACKGROUNDCOLOR, 0,0,&clr_enabledbackground,1);
        WmsQueryPresParam(GetHwnd(), PP_SBDISABLEDSCROLLFOREGROUNDCOLOR, 0,0,&clr_disabledforeground,1);
        WmsQueryPresParam(GetHwnd(), PP_SBDISABLEDSCROLLBACKGROUNDCOLOR, 0,0,&clr_disabledbackground,1);
        uint8 clr_enabled=clr_enabledbackground|clr_enabledforeground;
        uint8 clr_disabled=clr_disabledbackground|clr_disabledforeground;
        Bool isEnabled = (Bool)((WmsQueryWindowStyle(GetHwnd())&WS_DISABLED)==0);
        if(orient==horizontal) {
                if(isEnabled && current_pos>range_start)
                        CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yTop),  (uint16)(((uint16)clr_enabled)<<8 | ((uint16)sbchars[0])) );
                else
                        CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yTop),  (uint16)(((uint16)clr_disabled)<<8 | ((uint16)sbchars[0])) );
                if(isEnabled && current_pos<range_end)
                        CpiSetCellData(hdc, FPoint(rect.xRight-1,rect.yTop),  (uint16)(((uint16)clr_enabled)<<8 | ((uint16)sbchars[1])) );
                else
                        CpiSetCellData(hdc, FPoint(rect.xRight-1,rect.yTop),  (uint16)(((uint16)clr_disabled)<<8 | ((uint16)sbchars[1])) );
                rect.xLeft++;
                rect.xRight--;
        } else {
                if(isEnabled && current_pos>range_start)
                        CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yTop),  (uint16)(((uint16)clr_enabled)<<8 | ((uint16)sbchars[2])) );
                else
                        CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yTop),  (uint16)(((uint16)clr_disabled)<<8 | ((uint16)sbchars[2])) );
                if(isEnabled && current_pos<range_end)
                        CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yBottom-1),  (uint16)(((uint16)clr_enabled)<<8 | ((uint16)sbchars[3])) );
                else
                        CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yBottom-1),  (uint16)(((uint16)clr_disabled)<<8 | ((uint16)sbchars[3])) );
                rect.yTop++;
                rect.yBottom--;
        }

        //paint track
        uint8 clr_foreground,clr_background;
        if(isEnabled && amount_visible<total_amount) {
                WmsQueryPresParam(GetHwnd(), PP_SBTRACKFOREGROUNDCOLOR, 0,0,&clr_foreground,1);
                WmsQueryPresParam(GetHwnd(), PP_SBTRACKBACKGROUNDCOLOR, 0,0,&clr_background,1);
        } else {
                WmsQueryPresParam(GetHwnd(), PP_SBDISABLEDTRACKFOREGROUNDCOLOR, 0,0,&clr_foreground,1);
                WmsQueryPresParam(GetHwnd(), PP_SBDISABLEDTRACKBACKGROUNDCOLOR, 0,0,&clr_background,1);
        }
        CpiFillRect(hdc, rect, clr_background|clr_foreground, sbchars[4]);

        //draw thumb
        if(isEnabled && amount_visible<total_amount) {
                WmsQueryPresParam(GetHwnd(), PP_THUMBFOREGROUNDCOLOR, 0,0,&clr_foreground,1);
                WmsQueryPresParam(GetHwnd(), PP_THUMBBACKGROUNDCOLOR, 0,0,&clr_background,1);

                long z = orient==horizontal?rect.GetWidth():rect.GetHeight();
                int zz = (int)((z*current_pos)/(range_end-range_start+1));
                if(orient==horizontal)
                        CpiSetCellData(hdc, FPoint(rect.xLeft+zz,rect.yTop), (uint16)((((uint16)clr_background)<<8)|(((uint16)clr_foreground)<<8)|sbchars[5]));
                else
                        CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yTop+zz), (uint16)((((uint16)clr_background)<<8)|(((uint16)clr_foreground)<<8)|sbchars[6]));
        }

        WmsReleaseDC(hdc);

        return True;
}


Bool FScrollBar::KeyPressed(FKeyboardEvent &ev) {
        switch(ev.GetLogicalKey()) {
                case lk_home:
                        if(current_pos>range_start) {
                                current_pos=range_start;
                                WmsInvalidateRect(GetHwnd(),0,False);
                                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_START), MPFROMUINT32(current_pos));
                        }
                        return True;
                case lk_pageup:
                        doMuchLess();
                        return True;
                case lk_left:
                case lk_up:
                        doOneLess();
                        return True;
                case lk_right:
                case lk_down:
                        doOneMore();
                        return True;
                case lk_pagedown:
                        doMuchMore();
                        return True;
                case lk_end:
                        if(current_pos<range_end) {
                                current_pos=range_end;
                                WmsInvalidateRect(GetHwnd(),0,False);
                                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_END), MPFROMUINT32(current_pos));
                        }
                        return True;
                default:
                        return False;
        }
}


Bool FScrollBar::Button1Down(FButtonDownEvent &ev) {
        FRect rect;
        WmsQueryWindowRect(GetHwnd(),&rect);
        if(ev.GetX()==rect.GetLeft() && ev.GetY()==rect.GetTop()) {
                //scroll left/up
                mouse_action=dooneless;
                SetCapture();
                StartTimer(100,TID_ONELESS);
                doOneLess();
                return False;
        } else if(ev.GetX()==rect.GetRight()-1 && ev.GetY()==rect.GetBottom()-1) {
                //scroll right/down
                mouse_action=doonemore;
                SetCapture();
                StartTimer(100,TID_ONEMORE);
                doOneMore();
                return False;
        }
        scrollbar_orientation orient=(scrollbar_orientation)(WmsQueryWindowStyle(GetHwnd())&0x1);
        long z = orient==horizontal?rect.GetWidth()-2:rect.GetHeight()-2;
        z = (z*current_pos)/(range_end-range_start+1);
        if((orient==horizontal && ev.GetX()<=rect.xLeft+z) ||
           (orient==vertical   && ev.GetY()<=rect.yTop+z)) {
                doMuchLess();
        } else if((orient==horizontal && ev.GetX()>rect.xLeft+z+1) ||
                  (orient==vertical   && ev.GetY()>rect.yTop+z+1)) {
                doMuchMore();
        } else {
                mouse_action=track;
                current_trackpos=current_pos;
                SetCapture();
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_STARTTRACK), 0);
        }
        return False;
}

Bool FScrollBar::Button1Up(FButtonUpEvent &) {
        if(HasCapture()) {
                ReleaseCapture();
                StopTimer(TID_ONELESS);
                StopTimer(TID_ONEMORE);
                if(mouse_action==track) {
                        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_ENDTRACK), 0);
                        current_pos = current_trackpos;
                        WmsInvalidateRect(GetHwnd(),0,False);
                        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_SETPOS), 0);
                }
        }
        return False;
}

Bool FScrollBar::MouseMove(FMouseMoveEvent &ev) {
        if(!HasCapture()) return False;
        if(mouse_action!=track) return False;

        scrollbar_orientation orient=(scrollbar_orientation)(WmsQueryWindowStyle(GetHwnd())&0x1);
        FRect rect;
        WmsQueryWindowRect(GetHwnd(),&rect);
        sint32 newpos;
        if(orient == horizontal) {
                rect.xLeft++;
                rect.xRight--;
                if(ev.GetX()<=rect.xLeft)
                        newpos = range_start;
                else if(ev.GetX()>=rect.xRight)
                        newpos = range_end;
                else {
                        newpos = (ev.GetX()-rect.xLeft)*(range_end-range_start)/(rect.GetWidth()-1);
                        if(newpos>range_end) newpos=range_end;
                }
        } else {
                rect.yTop++;
                rect.yBottom--;
                if(ev.GetY()<=rect.yTop)
                        newpos = range_start;
                else if(ev.GetY()>=rect.yBottom)
                        newpos = range_end;
                else {
                        newpos = (ev.GetY()-rect.yTop)*(range_end-range_start)/(rect.GetHeight()-1);
                        if(newpos>range_end) newpos=range_end;
                }
        }
        if(newpos!=current_trackpos) {
                current_trackpos = newpos;
                WmsInvalidateRect(GetHwnd(),0,False);
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_CONTROL, MPFROM2UINT16(GetID(),SBN_TRACK), MPFROMUINT32(newpos));
        }
        return True;
}

Bool FScrollBar::Timer(FTimerEvent &ev) {
        switch(ev.GetTimerID()) {
                case TID_ONELESS:
                        doOneLess();
                        return True;
                case TID_ONEMORE:
                        doOneMore();
                        return True;
                default:
                        return False;
        }
}


Bool FScrollBar::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"vertical")==0) {
                        SetStyle(~(long)(horizontal|vertical),vertical);
                } else if(strcmp(name[p],"horizontal")==0) {
                        SetStyle(~(long)(horizontal|vertical),horizontal);
                }
        }
        return True;
}

long FScrollBar::sbm_querypos() {
        return (long)current_pos;
}

Bool FScrollBar::sbm_setpos(long pos) {
        if(pos<range_start) current_pos=range_start;
        else if(pos<=range_end) current_pos=pos;
        else current_pos=range_end;
        WmsInvalidateRect(GetHwnd(),0,False);
        return True;
}

long FScrollBar::sbm_queryrangestart() {
        return (long)range_start;
}

long FScrollBar::sbm_queryrangeend() {
        return (long)range_end;
}

Bool FScrollBar::sbm_setrange(long start, long end) {
        if(start>=end) return False;
        range_start=start;
        range_end=end;
        if(current_pos<range_start) current_pos=range_start;
        if(current_pos>range_end) current_pos=range_end;
        WmsInvalidateRect(GetHwnd(),0,False);
        return True;
}

Bool FScrollBar::sbm_setthumbsize(long visible, long total) {
        if(visible<0) return False;
        amount_visible = visible;
        total_amount=total;
        WmsInvalidateRect(GetHwnd(),0,False);
        return True;
}

