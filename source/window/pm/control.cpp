/***
Filename: control.cpp
Description:
  Implementation of FControl and FOwnerNotifyHandler
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#include <bif.h>
#include <dbglog.h>

FOwnerNotifyHandler::FOwnerNotifyHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FOwnerNotifyHandler::Dispatch));
}

Bool FOwnerNotifyHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_CONTROL) {
                FOwnerNotifyEvent& onev=(FOwnerNotifyEvent&)ev;
                return Notify(onev);
        } else
                return False;
}


//FControl---------------------------------------------------------------------
/***
Visibility: public
Description:
        Associate the BIF-window with the 'native' window already created by the OS
Parameters:
        pParent   parent window
        ID        id of the native window
Return:
        True on success
***/
Bool FControl::Create(FWnd *pParent, int ID) {
        TRACE_METHOD0("FControl::Create");
        HWND hwnd=WinWindowFromID(pParent->GetHwnd(),ID);
        if(hwnd) {
                GetWndMan()->SubRegWnd(this,hwnd);
                return True;
        } else {
                NOTE1("FControl::Create: control not found");
                return False;
        }
}

/***
Visibility: public
Description:
        Create a control window
Parameters:
        pParent         parent window
        ID              id of the native window
        pszClassname    name of the native window class (can be NULL)
        pszControlText  initial control text (can be NULL)
        flStyle         style of the native window
        rect            size/position of the window (can be NULL, but shouldn't be)
        pCtlData        control data
        pPresParam      presentation parameters
Return:
        True on success
Note:
        The window is always created visible
***/
Bool FControl::Create(FWnd *pParent, int ID,
                      const char *pszClassname, const char *pszControlText,
                      ULONG flStyle, FRect *rect,
                      PVOID pCtlData, PVOID pPresParam)
{
        TRACE_METHOD1("FControl::Create");

        FRect r(0,0,0,0);
        if(rect)
                r=*rect;

        flStyle|=WS_VISIBLE;
        return FWnd::Create(pParent->GetHwnd(),                        //parent
                            pszClassname,
                            pszControlText?pszControlText:"",
                            flStyle,
                            &r,
                            pParent->GetHwnd(),                         //owner
                            HWND_TOP,                                   //insertbehind
                            ID,
                            pCtlData,
                            pPresParam
                           );
}


/***
Visibility: public
Description:
        Return the ID of the control
Return:
        The window ID
***/
int FControl::GetID() {
        TRACE_METHOD1("FControl::GetID");
        return WinQueryWindowUShort(GetHwnd(),QWS_ID);
}



//FControlTextHandler----------------------------------------------------------

/***
Visibility: public
Description:
        Return the length of the window text
Return:
        Length of the window text
***/
int FControlTextHandler::GetTextLen() {
        return WinQueryWindowTextLength(GetWnd()->GetHwnd());
}

/***
Visibility: public
Description:
        Retreive the text of the window
Parameters:
        buffer  (output) window text
        maxbuf  size of <buffer>
Return:
        True on success
***/
Bool FControlTextHandler::GetText(char *buffer, int maxbuf) {
        WinQueryWindowText(GetWnd()->GetHwnd(), maxbuf, buffer);
        return True;
}

/***
Visibility: public
Description:
        Set the window text
Parameters:
        pszText   new window text
Return:
        True on success
***/
Bool FControlTextHandler::SetText(const char *pszText) {
        return (Bool)WinSetWindowText(GetWnd()->GetHwnd(),pszText);
}

