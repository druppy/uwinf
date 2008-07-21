/***
Filename: clhndl.cpp
Description:
  Implementation of client handlers (currenty only FPaintHandler)
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 95-02-15 Removed EraseBkgnd()
  ISJ 96-04-28 (Creation) Copied from OS/2 and modified for WMS
***/
#define BIFINCL_WINDOW
#define BIFINCL_CLIENTHANDLERS
#define WMSINCL_BW
#define WMSINCL_MSGS
#define WMSINCL_FOCUS
#define WMSINCL_CPI
#define WMSINCL_TIMER
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
        ::WmsInvalidateRect(GetWnd()->GetHwnd(),pr,includeDescendands);
}


//FCaretHandler----------------------------------------------------------------
FCaretHandler::FCaretHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FCaretHandler::Dispatch));
        //set some "reasonable" defaults
        cx=cy=0;
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

Bool FCaretHandler::SetCaretShape(Shape s) {
        if(s==cs)
                return True;
        cs=s;
        if(isCaretOwner)
                UpdateCaret();
        return True;
}

Bool FCaretHandler::ShowCaret() {
        if(cshow++==0) {
                if(isCaretOwner)
                        ::_CpiShowCursor();
        }
        return True;
}

Bool FCaretHandler::HideCaret() {
        if(--cshow==0) {
                if(isCaretOwner)
                        ::_CpiShowCursor();
        }
        return True;
}

Bool FCaretHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_SETFOCUS) {
                if(UINT161FROMMP(ev.GetMP2())) {
                        //gaining focus
                        UpdateCaret();
                } else {
                        //losing focus
                        if(isCaretOwner) {
                                isCaretOwner=False;
                                ::_CpiHideCursor();
                        }
                }
        } else if(ev.GetID()==WM_DESTROY) {
                if(isCaretOwner) {
                        isCaretOwner=False;
                        ::_CpiHideCursor();
                }
        }
        return False;
}

void FCaretHandler::UpdateCaret() {
        //caret is visible
        switch(cs) {
                case UnderscoreCaret:
                        ::_CpiSetCursorShape(90,100);
                        break;
                case HalfCaret:
                        ::_CpiSetCursorShape(50,100);
                        break;
                case BlockCaret:
                        ::_CpiSetCursorShape(0,100);
                        break;
        }
        isCaretOwner=True;
        UpdateCaretPos();
        if(cshow>0)
                ::_CpiShowCursor();
}

void FCaretHandler::UpdateCaretPos() {
        if(isCaretOwner) {
                ::_CpiSetCursorPos(GetWnd()->GetHwnd(),cx,cy);
        }
}


//FTimerHandler---------------------------------------------------------------
Bool FTimerHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_TIMER)
                return Timer((FTimerEvent&)ev);
        else
                return False;
}

Bool FTimerHandler::StartTimer(int interval, int timerid) {
        return WmsStartTimer(GetWnd()->GetHwnd(), timerid, interval, False);
}

Bool FTimerHandler::StopTimer(int timerid) {
        return WmsStopTimer(GetWnd()->GetHwnd(),timerid);
}


