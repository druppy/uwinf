/***
Filename: scroll.cpp
Description:
  Implementation of client window scrolling
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_SCROLLHANDLER
#define BIFINCL_APPWND
#include <bif.h>

FScrollEvent::type FScrollEvent::GetType() const {
#if BIFOS_ == BIFOS_WIN32_
        switch(LOWORD(wParam)) {
#else
        switch(wParam) {
#endif
                case SB_BOTTOM:
                        return end;
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
                case SB_THUMBPOSITION:
                        return setPos;
                case SB_THUMBTRACK:
                        return track;
                case SB_TOP:
                        return start;
        }
        return type(-1);        //should never be reached
}

int FScrollEvent::where() const {
#if BIFOS_ == BIFOS_WIN32_
        return HIWORD(wParam);
#else
        return LOWORD(lParam);
#endif
}


FClientScrollHandler::FClientScrollHandler(FClientWindow *pwnd)
  : FHandler(pwnd), client(pwnd)
{
        SetDispatch(dispatchFunc_t(&FClientScrollHandler::Dispatch));
}

int FClientScrollHandler::GetHScrollPos() {
        return ::GetScrollPos(client->GetAppWindow()->GetHwnd(),SB_HORZ);
}

Bool FClientScrollHandler::SetHScrollPos(int pos) {
        ::SetScrollPos(client->GetAppWindow()->GetHwnd(),SB_HORZ,pos,TRUE);
        UpdateHArrows();
        return True;
}

Bool FClientScrollHandler::GetHScrollRange(int *low, int *high) {
        ::GetScrollRange(client->GetAppWindow()->GetHwnd(),SB_HORZ,low,high);
        return True;
}

Bool FClientScrollHandler::SetHScrollThumbSize(int /*visible*/, int /*total*/) {
        //Windows doesn't support thumb sizes
        return True;
}

Bool FClientScrollHandler::SetHScrollBar(int pos, int low, int high) {
        if(low<0) low=0;
        if(high<low) high=low;
        if(pos<low) pos=low;
        if(pos>high) pos=high;
        ::SetScrollRange(client->GetAppWindow()->GetHwnd(),SB_HORZ,low,high,TRUE);
        ::SetScrollPos(client->GetAppWindow()->GetHwnd(),SB_HORZ,pos,TRUE);
        UpdateHArrows();
        return True;
}


int FClientScrollHandler::GetVScrollPos() {
        return GetScrollPos(client->GetAppWindow()->GetHwnd(),SB_VERT);
}

Bool FClientScrollHandler::SetVScrollPos(int pos) {
        ::SetScrollPos(client->GetAppWindow()->GetHwnd(),SB_VERT,pos,TRUE);
        UpdateVArrows();
        return True;
}

Bool FClientScrollHandler::GetVScrollRange(int *low, int *high) {
        ::GetScrollRange(client->GetAppWindow()->GetHwnd(),SB_VERT,low,high);
        return True;
}

Bool FClientScrollHandler::SetVScrollThumbSize(int /*visible*/, int /*total*/) {
        //Windows doesn't support thumb sizes
        return True;
}

Bool FClientScrollHandler::SetVScrollBar(int pos, int low, int high) {
        if(low<0) low=0;
        if(high<low) high=low;
        if(pos<low) pos=low;
        if(pos>high) pos=high;
        ::SetScrollRange(client->GetAppWindow()->GetHwnd(),SB_VERT,low,high,TRUE);
        ::SetScrollPos(client->GetAppWindow()->GetHwnd(),SB_VERT,pos,TRUE);
        UpdateVArrows();
        return True;
}

void FClientScrollHandler::UpdateHArrows() {
        int pos=GetHScrollPos();
        int low,high;
        GetHScrollRange(&low,&high);
        if(pos>low || pos<high)
                ::EnableScrollBar(client->GetAppWindow()->GetHwnd(),SB_HORZ,ESB_ENABLE_BOTH);
        if(pos<=low)
                ::EnableScrollBar(client->GetAppWindow()->GetHwnd(),SB_HORZ,ESB_DISABLE_LTUP);
        if(pos>=high)
                ::EnableScrollBar(client->GetAppWindow()->GetHwnd(),SB_HORZ,ESB_DISABLE_RTDN);
}

void FClientScrollHandler::UpdateVArrows() {
        int pos=GetVScrollPos();
        int low,high;
        GetVScrollRange(&low,&high);
        if(pos>low || pos<high)
                ::EnableScrollBar(client->GetAppWindow()->GetHwnd(),SB_VERT,ESB_ENABLE_BOTH);
        if(pos<=low)
                ::EnableScrollBar(client->GetAppWindow()->GetHwnd(),SB_VERT,ESB_DISABLE_LTUP);
        if(pos>=high)
                ::EnableScrollBar(client->GetAppWindow()->GetHwnd(),SB_VERT,ESB_DISABLE_RTDN);
}

Bool FClientScrollHandler::Dispatch(FEvent& ev) {
        if(HIWORD(ev.lParam)) return False;
        if(ev.GetID()==WM_HSCROLL) {
                FScrollEvent& sev=(FScrollEvent&)ev;
                Bool b=HScroll(sev);
                if(sev.GetType()==FScrollEvent::endScroll)
                        UpdateHArrows();        //fix win 3.x repaint bug
                return b;
        }
        if(ev.GetID()==WM_VSCROLL) {
                FScrollEvent& sev=(FScrollEvent&)ev;
                Bool b=VScroll(sev);
                if(sev.GetType()==FScrollEvent::endScroll)
                        UpdateVArrows();        //fix win 3.x repaint bug
                return b;
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

