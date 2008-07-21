/***
Filename: clhndl.cpp
Description:
  Implementation of client handlers (currenty only FPaintHandler)
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
  ISJ 95-04-17 Removed EraseBkgnd() memberfunc
  ISJ 96-06-01 Implemented FCaretHandler and FTimerHandler
***/
#define BIFINCL_WINDOW
#define BIFINCL_CLIENTHANDLERS
#include <bif.h>


//paint-----------------------------------------------------------------------
FPaintHandler::FPaintHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FPaintHandler::Dispatch));
}

Bool FPaintHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_PAINT) {
                FPaintEvent& pev=(FPaintEvent&)ev;
                return Paint(pev);
        }
        // Note: WM_ERASEBKGND is not caught because Windows' split bewteen
        // background and foreground is not portable.
        // With BIF you have to paint everything including the background in
        //Paint()
        return False;
}


void FPaintHandler::Invalidate(FRect *pr, Bool eraseBackground) {
        ::InvalidateRect(GetWnd()->GetHwnd(),pr,eraseBackground);
}


//caret-----------------------------------------------------------------------
FCaretHandler::FCaretHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FCaretHandler::Dispatch));
        //set some reasonable defaults
        cx=cy=0;
        cw=ch=4;
        cshow=0;
        cs=BlockCaret;
        isCaretOwner=False;
}

Bool FCaretHandler::SetCaretPos(int x, int y) {
        if(x==cx && y==cy)
                return True;
        cx=x;
        cy=y;
        if(isCaretOwner)
                UpdateCaretPos();
        return True;
}

Bool FCaretHandler::SetCaretShape(Shape s, int w, int h) {
        if(s==cs && w==cw && h==ch)
                return True;
        cs=s;
        cw=w;
        ch=h;
        if(isCaretOwner)
                UpdateCaret();
        return True;
}

Bool FCaretHandler::ShowCaret() {
        if(cshow++==0) {
                if(isCaretOwner)
                        ::ShowCaret(GetWnd()->GetHwnd());
        }
        return True;
}

Bool FCaretHandler::HideCaret() {
        if(--cshow==0) {
                if(isCaretOwner)
                        ::HideCaret(GetWnd()->GetHwnd());
        }
        return True;
}


Bool FCaretHandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_DESTROY:
                case WM_KILLFOCUS:
                        if(isCaretOwner)
                                ::DestroyCaret();
                        return False;
                case WM_SETFOCUS:
                        UpdateCaret();
                        return False;
                default:
                        return False;
        }
}

void FCaretHandler::UpdateCaret() {
        //update the caret's shape and/or create/destroy
        if(isCaretOwner) {
                ::DestroyCaret();
                isCaretOwner=False;
        }
        int w,h;
        switch(cs) {
                case UnderscoreCaret:
                        w=cw;
                        h=0;
                        break;
                case VerticalCaret:
                        w=0;
                        h=ch;
                        break;
                case BlockCaret:
                        w=cw;
                        h=ch;
                        break;
        }
        ::CreateCaret(GetWnd()->GetHwnd(), NULL, w,h);
        isCaretOwner=True;
        UpdateCaretPos();
        if(cshow>0)
                ::ShowCaret(GetWnd()->GetHwnd());
}

void FCaretHandler::UpdateCaretPos() {
        int x=cx, y=cy;
        if(cs==UnderscoreCaret)
                y = y + ch - GetSystemMetrics(SM_CYBORDER);
        ::SetCaretPos(x,y);
}


//timer-----------------------------------------------------------------------

Bool FTimerHandler::StartTimer(int interval, int timerid) {
        UINT rc = ::SetTimer(GetWnd()->GetHwnd(), (UINT)timerid, (UINT)interval, NULL);
        if(rc!=0)
                return True;
        else
                return False;
}

Bool FTimerHandler::StopTimer(int timerid) {
        return (Bool)::KillTimer(GetWnd()->GetHwnd(), (UINT)timerid);
}

Bool FTimerHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_TIMER)
                return Timer((FTimerEvent&)ev);
        return False;
}


