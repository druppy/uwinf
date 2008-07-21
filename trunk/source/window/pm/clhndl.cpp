/***
Filename: clhndl.cpp
Description:
  Implementation of client handlers (currenty only FPaintHandler)
Host:
  Watcom 10.0a-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 95-02-15 Removed EraseBkgnd()
  ISJ 96-05-30 Implemented FCaretHandler and FTimerHandler
***/
#define BIFINCL_WINDOW
#define BIFINCL_CLIENTHANDLERS
#define INCL_WINCURSORS
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
        } else
                return False;
}


/***
Visibility: public
Description:
        Invalidate an area of the window
Parameters:
        pr                   pointer to the rectangle to be invalidated (NULL for whole window)
        includeDescendands   See WinInvalidateRect()
***/
void FPaintHandler::Invalidate(FRect *pr, Bool includeDescendands) {
        ::WinInvalidateRect(GetWnd()->GetHwnd(),pr,includeDescendands);
}


//caret-----------------------------------------------------------------------
FCaretHandler::FCaretHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(FCaretHandler::Dispatch));
        //set some "reasonable" defaults
        cx=cy=0;
        cw=ch=4;
        cs=InsertCaret;
        cshow=0;        //initially hidden
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
                        ::WinShowCursor(GetWnd()->GetHwnd(),TRUE);
        }
        return True;
}

Bool FCaretHandler::HideCaret() {
        if(--cshow==0) {
                if(isCaretOwner)
                        ::WinShowCursor(GetWnd()->GetHwnd(),FALSE);
        }
        return True;
}

Bool FCaretHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_SETFOCUS) {
                if(SHORT1FROMMP(ev.GetMP2())) {
                        //gaining focus
                        isCaretOwner=True;
                        UpdateCaret();
                } else {
                        //losing focus
                        if(isCaretOwner) {
                                isCaretOwner=False;
                                ::WinDestroyCursor(GetWnd()->GetHwnd());
                        }
                }
        } else if(ev.GetID()==WM_DESTROY) {
                if(isCaretOwner) {
                        isCaretOwner=False;
                        ::WinDestroyCursor(GetWnd()->GetHwnd());
                }
        }
        return False;
}

void FCaretHandler::UpdateCaret() {
        if(isCaretOwner) {
                ::WinDestroyCursor(GetWnd()->GetHwnd());
                isCaretOwner=False;
        }
        LONG lcx,lcy;
        switch(cs) {
                case UnderscoreCaret:
                        lcx = cw;
                        lcy = 0;
                        break;
                case VerticalCaret:
                        lcx = 0;
                        lcy = ch;
                        break;
                case BlockCaret:
                        lcx = cw;
                        lcy = ch;
                        break;
        }
        ::WinCreateCursor(GetWnd()->GetHwnd(), cx,cy, lcx,lcy, CURSOR_SOLID|CURSOR_FLASH, NULL);
        isCaretOwner=True;
        UpdateCaretPos();
        if(cshow>0)
                ::WinShowCursor(GetWnd()->GetHwnd(), TRUE);
}

void FCaretHandler::UpdateCaretPos() {
        if(isCaretOwner)
                ::WinCreateCursor(GetWnd()->GetHwnd(), cx,cy, 0,0, CURSOR_SETPOS, NULL);
}


//FTimerHandler---------------------------------------------------------------
Bool FTimerHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_TIMER)
                return Timer((FTimerEvent&)ev);
        else
                return False;
}

Bool FTimerHandler::StartTimer(int interval, int timerid) {
        return (Bool)WinStartTimer(GetWndMan()->GetHAB(0), GetWnd()->GetHwnd(), timerid, interval);
}

Bool FTimerHandler::StopTimer(int timerid) {
        return (Bool)WinStopTimer(GetWndMan()->GetHAB(0), GetWnd()->GetHwnd(), timerid);
}

