/***
Filename: scroll.cpp
Description:
  Implementation of client window scrolling
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_SCROLLHANDLER
#define BIFINCL_APPWND
#include <bif.h>

FScrollEvent::type FScrollEvent::GetType() const {
        switch(SHORT2FROMMP(GetMP2())) {
                //OS/2 doesn't know these two
                //case SB_BOTTOM:
                //        return end;
                //case SB_TOP:
                //        return start;
                case SB_ENDSCROLL:
                        return endScroll;
                case SB_LINEDOWN:
                        return oneMore;
                case SB_LINEUP:
                        return oneLess;
                case SB_PAGEDOWN:
                        return muchMore;
                case SB_PAGEUP:
                        return muchLess;
                case SB_SLIDERPOSITION:
                        return setPos;
                case SB_SLIDERTRACK:
                        return track;
        }
        return type(-1);        //should never be reached
}

int FScrollEvent::where() const {
        return SHORT1FROMMP(GetMP2());
}


FClientScrollHandler::FClientScrollHandler(FClientWindow *pwnd)
  : FHandler(pwnd), client(pwnd)
{
        SetDispatch(dispatchFunc_t(&FClientScrollHandler::Dispatch));
        hwndVScroll=hwndHScroll=NULL;
}

int FClientScrollHandler::GetHScrollPos() {
        return SHORT1FROMMR(WinSendMsg(hwndHScroll,SBM_QUERYPOS,0,0));
}

Bool FClientScrollHandler::SetHScrollPos(int pos) {
        return (Bool)WinSendMsg(hwndHScroll,SBM_SETPOS,MPFROMSHORT(pos),0);
}

Bool FClientScrollHandler::GetHScrollRange(int *low, int *high) {
        MRESULT rc=WinSendMsg(hwndHScroll,SBM_QUERYRANGE,0,0);
        *low=SHORT1FROMMR(rc);
        *high=SHORT2FROMMR(rc);
        return True;
}

Bool FClientScrollHandler::SetHScrollThumbSize(int visible, int total) {
        return (Bool)WinSendMsg(hwndHScroll,SBM_SETTHUMBSIZE,MPFROM2SHORT(visible,total),0);
}

Bool FClientScrollHandler::SetHScrollBar(int pos, int low, int high) {
        return (Bool)WinSendMsg(hwndHScroll,SBM_SETSCROLLBAR,MPFROMSHORT(pos),MPFROM2SHORT(low,high));
}


int FClientScrollHandler::GetVScrollPos() {
        return SHORT1FROMMR(WinSendMsg(hwndVScroll,SBM_QUERYPOS,0,0));
}

Bool FClientScrollHandler::SetVScrollPos(int pos) {
        return (Bool)WinSendMsg(hwndVScroll,SBM_SETPOS,MPFROMSHORT(pos),0);
}

Bool FClientScrollHandler::GetVScrollRange(int *low, int *high) {
        MRESULT rc=WinSendMsg(hwndVScroll,SBM_QUERYRANGE,0,0);
        *low=SHORT1FROMMR(rc);
        *high=SHORT2FROMMR(rc);
        return True;
}

Bool FClientScrollHandler::SetVScrollThumbSize(int visible, int total) {
        return (Bool)WinSendMsg(hwndVScroll,SBM_SETTHUMBSIZE,MPFROM2SHORT(visible,total),0);
}

Bool FClientScrollHandler::SetVScrollBar(int pos, int low, int high) {
        return (Bool)WinSendMsg(hwndVScroll,SBM_SETSCROLLBAR,MPFROMSHORT(pos),MPFROM2SHORT(low,high));
}


Bool FClientScrollHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==GetWndMan()->wm_created) {
                //setup hwndHScroll and hwndVScroll
                HWND hwndFrame=WinQueryWindow(GetWnd()->GetHwnd(),QW_PARENT);
                hwndHScroll=WinWindowFromID(hwndFrame,FID_HORZSCROLL);
                hwndVScroll=WinWindowFromID(hwndFrame,FID_VERTSCROLL);
                return False;
        }

        FScrollEvent& sev=(FScrollEvent&)ev;
        if(ev.GetID()==WM_HSCROLL && sev.GetIdentifier()==FID_HORZSCROLL) {
                return HScroll(sev);
        }
        if(ev.GetID()==WM_VSCROLL && sev.GetIdentifier()==FID_VERTSCROLL) {
                FScrollEvent& sev=(FScrollEvent&)ev;
                return VScroll(sev);
        }
        return False;
}


Bool FClientScrollHandler::HScroll(FScrollEvent& ev) {
        switch(ev.GetType()) {
                case FScrollEvent::start:    return HScrollStart(ev);
                case FScrollEvent::end:      return HScrollEnd(ev);
                case FScrollEvent::oneLess:  return HScrollLineLeft(ev);
                case FScrollEvent::oneMore:  return HScrollLineRight(ev);
                case FScrollEvent::muchLess: return HScrollPageLeft(ev);
                case FScrollEvent::muchMore: return HScrollPageRight(ev);
                case FScrollEvent::track:    return HScrollTrack(ev);
                case FScrollEvent::setPos:   return HScrollTo(ev);
                case FScrollEvent::endScroll:
                default:
                        return False;
        }
}

Bool FClientScrollHandler::VScroll(FScrollEvent& ev) {
        switch(ev.GetType()) {
                case FScrollEvent::start:    return VScrollTop(ev);
                case FScrollEvent::end:      return VScrollBottom(ev);
                case FScrollEvent::oneLess:  return VScrollLineUp(ev);
                case FScrollEvent::oneMore:  return VScrollLineDown(ev);
                case FScrollEvent::muchLess: return VScrollPageUp(ev);
                case FScrollEvent::muchMore: return VScrollPageDown(ev);
                case FScrollEvent::track:    return VScrollTrack(ev);
                case FScrollEvent::setPos:   return VScrollTo(ev);
                case FScrollEvent::endScroll:
                default:
                        return False;
        }
}

