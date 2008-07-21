/***
Filename: comhndl.cpp
Description:
  Implementation of miscellanous handlers
Host:
  Watcom 10.5 -10.6 OS/2, DOS16
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-??-?? Copied from PM source
***/
#define BIFINCL_WINDOW
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_DYNMSG
#define WMSINCL_MSGS
#define WMSINCL_DESKTOP
#define WMSINCL_FOCUS
#include <bif.h>

//create/destroy-----------------------------------------------------------------
FCreateDestroyHandler::FCreateDestroyHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FCreateDestroyHandler::Dispatch));
}

Bool FCreateDestroyHandler::Dispatch(FEvent &ev) {
        //Note: these two events are _never_ eaten, but the notify functions 
        //      return a Bool for consistency
        if(ev.GetID()==GetWndMan()->wm_created) {
                //The window has been created
                Created((FCreatedEvent&)ev);
        } else if(ev.GetID()==WM_DESTROY) {
                //The window is being destroyed
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
                //Something wants us to do something
                return Command((FCommandEvent&)ev);
        } else if(ev.GetID()==WM_SYSCOMMAND) {
                //system-specific command
                return SysCommand((FSysCommandEvent&)ev);
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
                        return Enabled(eev);    //the window has been enabled
                else
                        return Disabled(eev);   //the window has been disabled
        } else
                return False;
}


//focus------------------------------------------------------------------------
WmsMSG FFocusHandler::wm_forwardfocus=0;
FFocusHandler::FFocusHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FFocusHandler::Dispatch));
        if(wm_forwardfocus==0) {
                //register our special message
                wm_forwardfocus = RegisterDynamicMessage("wm_forwardfocus");
        }
}

Bool FFocusHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_SETFOCUS) {
                FFocusEvent_base& ffeb=(FFocusEvent_base&)ev;
                if(ffeb.GetFocusFlag())
                        return GotFocus((FSetFocusEvent&)ffeb);         //window has received focus
                else
                        return LostFocus((FKillFocusEvent&)ffeb);       //window has lost focus
        } else if(ev.GetID()==wm_forwardfocus) {
                //we have told ourselves that we should forward the focus
                if(::WmsQueryFocus(HWND_DESKTOP)==GetWnd()->GetHwnd()) {
                        //we still have the focus
                        WmsHWND forwardWindow=HWNDFROMMP(ev.GetMP1());
                        //ensure that the forward window still exists
                        if(WmsIsWindow(forwardWindow)) {
                                WmsSetFocus(HWND_DESKTOP,forwardWindow);
                        }
                }
                return True;
        }
        return False;
}


/***
Visibility: public
Description:
        Set the focus to the window
Return:
        True on success
***/
Bool FFocusHandler::SetFocus() {
        WmsSetFocus(HWND_DESKTOP,GetWnd()->GetHwnd());
        return True;
}

/***
Visibility: public
Description:
        Forward the focus from this window to another
Parameters:
        pwnd   Window to forward the focus to
Return:
        True if succesful, False otherwise
***/
Bool FFocusHandler::ForwardFocus(FWnd *pwnd) {
        //WMS can't handle that we change the focus while it is changing :-)
        //So we _post_ a message to ourselves to postpone the change until
        //the current change has finished. If this litle trick doesn't work 
        //YOU lose
        WmsPostMsg(GetWnd()->GetHwnd(),wm_forwardfocus,MPFROMHWND(pwnd->GetHwnd()),(WmsMPARAM)0);
        return True;
}


/***
Visibility: public
Description:
        Returns wether the window has the focus or not
***/
Bool FFocusHandler::HasFocus() {
        if(::WmsQueryFocus(HWND_DESKTOP)==GetWnd()->GetHwnd())
                return True;
        else
                return False;

}

//size------------------------------------------------------------------------
FSizeHandler::FSizeHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FSizeHandler::Dispatch));
}


/***
Visibility: public
Description:
        Set the size (width and height) of the window
Parameters:
        w  new width of the window
        h  new height of the window
Return:
        True on success
***/
Bool FSizeHandler::SetSize(int w, int h) {
        return (Bool)WmsSetWindowPos(GetWnd()->GetHwnd(), (WmsHWND)0, 0,0, w,h, SWP_SIZE);
}

/***
Visibility: public
Description:
        Return the size (width and height) of the window
Parameters:
        w  (output) width of the window
        h  (output) height of the window
Return:
        True on success
***/
Bool FSizeHandler::GetSize(int *w, int *h) {
        FRect r;
        if(WmsQueryWindowRect(GetWnd()->GetHwnd(),&r)) {
                *w = r.GetWidth();
                *h = r.GetHeight();
                return True;
        } else
                return False;
}


Bool FSizeHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_WINDOWPOSCHANGED &&
           UINT161FROMMP(ev.GetMP2())&SWP_SIZE)
        {
                FSizeEvent& sev=(FSizeEvent&)ev;
                return SizeChanged(sev);                //the window has changed size
        } else
                return False;
}



//move------------------------------------------------------------------------
FMoveHandler::FMoveHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FMoveHandler::Dispatch));
}


/***
Visibility: public
Description:
        Move the window (change its origin)
Parameters:
        x  new x-origin of the window (relative to the window parent)
        y  new y-origin of the window (relative to the window parent)
Return:
        True on success
***/
Bool FMoveHandler::Move(int x, int y) {
        return (Bool)WmsSetWindowPos(GetWnd()->GetHwnd(), (WmsHWND)0, x,y, 0,0, SWP_MOVE);
}


/***
Visibility: public
Description:
        Query the position of the window
***/
Bool FMoveHandler::GetPosition(int *x, int *y) {
        FRect rct;
        if(!WmsQueryWindowRect(GetWnd()->GetHwnd(),&rct))
                return False;
        WmsMapWindowRects(GetWnd()->GetHwnd(), WmsQueryWindowParent(GetWnd()->GetHwnd()), &rct,1);
        *x = rct.GetLeft();
        *y = rct.GetTop();
        return True;
}


Bool FMoveHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_WINDOWPOSCHANGED &&
           UINT161FROMMP(ev.GetMP2())&SWP_MOVE)
        {
                FMoveEvent& mev=(FMoveEvent&)ev;
                return Moved(mev);
        } else
                return False;
}


//close------------------------------------------------------------------------
FCloseHandler::FCloseHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FCloseHandler::Dispatch));

}

/***
Visibility: public
Description:
        Ask the window nicely to close
***/
void FCloseHandler::Close() {
        WmsSendMsg(GetWnd()->GetHwnd(), WM_CLOSE, 0, 0);
}

/***
Visibility: public
Description:
        Close the window. The window will be notified that it is going to be closed
Note:
        The window cannot reject.
***/
void FCloseHandler::CloseUnconditionally() {
        WmsSendMsg(GetWnd()->GetHwnd(), WM_SAVEAPPLICATION, 0, 0);
        if(GetWnd()->IsValid() &&
           WmsIsWindow(GetWnd()->GetHwnd()))
                WmsDestroyWindow(GetWnd()->GetHwnd());
}


Bool FCloseHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_CLOSE)
                return Close(ev);
        else if(ev.GetID()==WM_SAVEAPPLICATION)
                return CloseUnconditionally(ev);
        else
                return False;
}


