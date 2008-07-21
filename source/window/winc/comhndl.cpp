/***
Filename: comhndl.cpp
Description:
  Implementation of miscellanous handlers
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
  ISJ 94-12-21 Added FFocusHandler::ForwardFocus
  ISJ 94-12-21 Added FCreateDestroyHandler
***/
#define BIFINCL_WINDOW
#define BIFINCL_COMMONHANDLERS
#include <bif.h>

//create/destroy-----------------------------------------------------------------
FCreateDestroyHandler::FCreateDestroyHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FCreateDestroyHandler::Dispatch));
}

Bool FCreateDestroyHandler::Dispatch(FEvent &ev) {
        //Note: these two events are _never_ eaten
        if(ev.GetID()==GetWndMan()->wm_created) {
                Created((FCreatedEvent&)ev);
        } else if(ev.GetID()==WM_DESTROY) {
                Destroying((FDestroyingEvent&)ev);
        }
        return False;
}


//command------------------------------------------------------------------------
FCommandHandler::FCommandHandler(FWnd *pwnd)
  :FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FCommandHandler::Dispatch));
}

Bool FCommandHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_COMMAND) {
                //Windows sends both commands and notifications in the same
                //message, so _we_ have to solve _their_ confusion.
                //A wm_command is a command if:
                //  the wm_command comes from something else than a control
                //or
                //  the control is a button and the "notification" is bn_clicked
                FCommandEvent& cev=(FCommandEvent&)ev;

                if(cev.GetHwnd()) {
                        //sent from a control. We assume that if a control sends
                        //a command, then it will set the "notification code" to
                        //0 (BN_CLICKED). We are lucky that bn_clicked is the
                        //only notification code that is zero
                        if(cev.GetNotifyCode()!=0) {
                                //a notification - not a command
                                return False;
                        } else {
                                //ensure that if it is a button it is a pushbutton
                                LRESULT l=SendMessage(cev.GetHwnd(),WM_GETDLGCODE,0,0);
                                if((l&(DLGC_DEFPUSHBUTTON|DLGC_UNDEFPUSHBUTTON))==0) {
                                        //not a button
                                        return False;
                                }
                        }
                }
                return Command(cev);
        } else if(ev.GetID()==WM_SYSCOMMAND) {
                FSysCommandEvent& cev=(FSysCommandEvent&)ev;
                return SysCommand(cev);
        } else
                return False;
}



//enable------------------------------------------------------------------------
FEnableHandler::FEnableHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FEnableHandler::Dispatch));

}

Bool FEnableHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_ENABLE) {
                FEnableEvent& eev=(FEnableEvent&)ev;
                if(eev.GetFlag())
                        return Enabled(eev);
                else
                        return Disabled(eev);
        } else
                return False;
}


//focus------------------------------------------------------------------------
FFocusHandler::FFocusHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FFocusHandler::Dispatch));
}


Bool FFocusHandler::Dispatch(FEvent& ev) {
        switch(ev.GetID()) {
                case WM_SETFOCUS: {
                        FSetFocusEvent& sfev=(FSetFocusEvent&)ev;
                        return GotFocus(sfev);
                }
                case WM_KILLFOCUS: {
                        FKillFocusEvent& kfev=(FKillFocusEvent&)ev;
                        return LostFocus(kfev);
                }
                default:
                        return False;
        }
}

Bool FFocusHandler::HasFocus() {
        if(::GetFocus()==GetWnd()->GetHwnd())
                return True;
        else
                return False;
}

Bool FFocusHandler::SetFocus() {
        ::SetFocus(GetWnd()->GetHwnd());
        return True;
}

Bool FFocusHandler::ForwardFocus(FWnd *pwnd) {
        //Windows can handle this
        ::SetFocus(pwnd->GetHwnd());
        return True;
}


//size------------------------------------------------------------------------
FSizeHandler::FSizeHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FSizeHandler::Dispatch));
}


Bool FSizeHandler::SetSize(int w, int h) {
        return (Bool)SetWindowPos(GetWnd()->GetHwnd(), NULL, 0,0, w,h, SWP_NOZORDER|SWP_NOMOVE);
}

Bool FSizeHandler::GetSize(int *w, int *h) {
        FRect r;
        GetWindowRect(GetWnd()->GetHwnd(),&r);
        *w = r.GetWidth();
        *h = r.GetHeight();
        return True;
}


Bool FSizeHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_SIZE) {
                FSizeEvent& sev=(FSizeEvent&)ev;
                return SizeChanged(sev);
        } else
                return False;
}



//move------------------------------------------------------------------------
FMoveHandler::FMoveHandler(FWnd *pWnd) : FHandler(pWnd)
{
        SetDispatch(dispatchFunc_t(&FMoveHandler::Dispatch));
}


Bool FMoveHandler::Move(int x, int y) 
{
        return (Bool)SetWindowPos(FHandler::GetWnd()->GetHwnd(), 
                NULL, x,y, 0,0, SWP_NOZORDER | SWP_NOSIZE);
}


Bool FMoveHandler::GetPosition(int *x, int *y) {
        RECT rect;
        GetWindowRect(GetWnd()->GetHwnd(), &rect);
        MapWindowPoints(HWND_DESKTOP, GetParent(GetWnd()->GetHwnd()), (POINT*)&rect, 2);
        *x = rect.left;
        *y = rect.top;
        return True;
}

Bool FMoveHandler::Dispatch(FEvent& ev) 
{
        if(ev.GetID()==WM_MOVE) {
                FMoveEvent& sev=(FMoveEvent&)ev;
                return Moved(sev);
        } else
                return False;
}


//close------------------------------------------------------------------------

FCloseHandler::FCloseHandler(FWnd *pwnd)
  : FHandler(pwnd),
    wm_saveapplication(RegisterWindowMessage("BIF/Window-wm_saveapplication"))
{
        SetDispatch(dispatchFunc_t(&FCloseHandler::Dispatch));
}

/***
Visibility: public
Description:
        Ask the window nicely to close. The window can reject
***/
void FCloseHandler::Close() {
        SendMessage(GetWnd()->GetHwnd(), WM_CLOSE, 0, 0);
}

/***
Visibility: public
Description:
        Force the window to close. The window will be notified of this
***/
void FCloseHandler::CloseUnconditionally() {
        //notify the window
        SendMessage(GetWnd()->GetHwnd(), wm_saveapplication, 0, 0);
        //kill it if it still exist
        if(GetWnd()->IsValid() && IsWindow(GetWnd()->GetHwnd()))
                GetWnd()->Destroy();
}

Bool FCloseHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_CLOSE) {
                //request to close
                return Close(ev);
        } else if(ev.GetID()==wm_saveapplication)
                //the window will be forced to die
                return CloseUnconditionally(ev);
        else if(ev.GetID()==WM_DESTROY) {
                //the window is being killed
                CloseUnconditionally(ev);
                return False;   //always pass this message to other handlers
        } else if(ev.GetID()==WM_ENDSESSION && ev.wParam) {
                //Windows is terminating
                CloseUnconditionally(ev);
                return False;   //always pass this message to other handlers
        } else
                return False;
}

