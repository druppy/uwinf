/***
Filename: scroll.cpp
Description:
  Implementation of client window scrolling
Host:
  Watcom 10.0a-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-10-20 Copied and modified for WMS
***/
#define BIFINCL_WINDOW
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_SCROLLHANDLER
#define BIFINCL_APPWND
#define BIFINCL_CONTROLS
#define BIFINCL_SCROLLBAR
#define WMSINCL_MSGS
#include <bif.h>

FScrollEvent::type FScrollEvent::GetType() const {
        switch(UINT162FROMMP(GetMP2())) {
                case SBN_START:
                        return start;
                case SBN_END:
                        return end;
                case SBN_ENDTRACK:
                        return endScroll;
                case SBN_ONEMORE:
                        return oneMore;
                case SBN_ONELESS:
                        return oneLess;
                case SBN_MUCHMORE:
                        return muchMore;
                case SBN_MUCHLESS:
                        return muchLess;
                case SBN_SETPOS:
                        return setPos;
                case SBN_TRACK:
                        return track;
        }
        return type(-1);        //should never be reached
}

int FScrollEvent::where() const {
        return (int)UINT32FROMMP(GetMP2());
}


FClientScrollHandler::FClientScrollHandler(FClientWindow *pwnd)
  : FHandler(pwnd), client(pwnd)
{
        SetDispatch(dispatchFunc_t(&FClientScrollHandler::Dispatch));
        hwndVScroll=hwndHScroll=0;
}

int FClientScrollHandler::GetHScrollPos() {
        return (int)UINT32FROMMR(WmsSendMsg(hwndHScroll,SBM_QUERYPOS,0,0));
}

Bool FClientScrollHandler::SetHScrollPos(int pos) {
        return (Bool)WmsSendMsg(hwndHScroll,SBM_SETPOS,MPFROMUINT32(pos),0);
}

Bool FClientScrollHandler::GetHScrollRange(int *low, int *high) {
        if(low)
                *low = (int)UINT32FROMMR(WmsSendMsg(hwndHScroll,SBM_QUERYRANGESTART,0,0));
        if(high)
                *high = (int)UINT32FROMMR(WmsSendMsg(hwndHScroll,SBM_QUERYRANGEEND,0,0));
        return True;
}

Bool FClientScrollHandler::SetHScrollThumbSize(int visible, int total) {
        return (Bool)WmsSendMsg(hwndHScroll,SBM_SETTHUMBSIZE,MPFROMUINT32(visible),MPFROMUINT32(total));
}

Bool FClientScrollHandler::SetHScrollBar(int pos, int low, int high) {
        WmsSendMsg(hwndHScroll, SBM_SETRANGE, MPFROMUINT32(low), MPFROMUINT32(high));
        WmsSendMsg(hwndHScroll, SBM_SETPOS, MPFROMUINT32(pos), 0);
        return True;
}


int FClientScrollHandler::GetVScrollPos() {
        return (int)UINT32FROMMR(WmsSendMsg(hwndVScroll,SBM_QUERYPOS,0,0));
}

Bool FClientScrollHandler::SetVScrollPos(int pos) {
        return (Bool)WmsSendMsg(hwndVScroll,SBM_SETPOS,MPFROMUINT32(pos),0);
}

Bool FClientScrollHandler::GetVScrollRange(int *low, int *high) {
        if(low)
                *low = (int)UINT32FROMMR(WmsSendMsg(hwndVScroll,SBM_QUERYRANGESTART,0,0));
        if(high)
                *high = (int)UINT32FROMMR(WmsSendMsg(hwndVScroll,SBM_QUERYRANGEEND,0,0));
        return True;
}

Bool FClientScrollHandler::SetVScrollThumbSize(int visible, int total) {
        return (Bool)WmsSendMsg(hwndVScroll,SBM_SETTHUMBSIZE,MPFROMUINT32(visible),MPFROMUINT32(total));
}

Bool FClientScrollHandler::SetVScrollBar(int pos, int low, int high) {
        WmsSendMsg(hwndVScroll, SBM_SETRANGE, MPFROMUINT32(low), MPFROMUINT32(high));
        WmsSendMsg(hwndVScroll, SBM_SETPOS, MPFROMUINT32(pos), 0);
        return True;
}


Bool FClientScrollHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==GetWndMan()->wm_created) {
                //setup hwndHScroll and hwndVScroll
                WmsHWND hwndFrame = WmsQueryWindowParent(GetWnd()->GetHwnd());
                hwndHScroll = WmsWindowFromID(hwndFrame,FID_HORZSCROLL);
                hwndVScroll = WmsWindowFromID(hwndFrame,FID_VERTSCROLL);
                return False;
        }

        if(ev.GetID()==WM_CONTROL) {
                FScrollEvent& sev=(FScrollEvent&)ev;
                if(sev.GetIdentifier()==FID_HORZSCROLL)
                        return HScroll(sev);
                if(sev.GetIdentifier()==FID_VERTSCROLL)
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