/***
Filename: control.cpp
Description:
  Implementation of FControl and FOwnerNotifyHandler
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
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
        if(ev.GetID()==WM_COMMAND) {
                FOwnerNotifyEvent& onev=(FOwnerNotifyEvent&)ev;
                if(onev.GetItemHwnd()==NULL)
                        return False; //it's a command
                if(onev.GetNotifyCode()==0/*BN_CLICKED*/) {
                        //ensure that it's not a command from a pushbutton
                        LRESULT l=SendMessage(onev.GetItemHwnd(),WM_GETDLGCODE,0,0);
                        if(l&(DLGC_DEFPUSHBUTTON|DLGC_UNDEFPUSHBUTTON))
                                return False;
                }
                return Notify(onev);
        } else
                return False;
}



/////////////////////
// FControl

Bool FControl::Create(FWnd *pParent, int ID) 
{
        TRACE_METHOD0("FControl::Create");
        HWND hwnd = GetDlgItem(pParent->GetHwnd(),ID);
        
        if(hwnd) {
                GetWndMan()->SubRegWnd( this, hwnd );
                return( True );
        } else {
                NOTE_TEXT1("FControl::Create: control not found");
                return False;
        }
}

Bool FControl::Create(FWnd *pParent, int ID,
                      const char *pszClassname, const char *pszControlText,
                      DWORD dwStyle, FRect *rect)
{
        TRACE_METHOD0("FControl::Create");

        FRect r(0,0,0,0);
        if(rect)
                r=*rect;

#if BIFOS_==BIFOS_WIN32_
        dwStyle |= WS_CHILD|WS_VISIBLE|WS_EX_CLIENTEDGE;
#else
	dwStyle |= WS_CHILD|WS_VISIBLE;
#endif
        
        return FWnd::Create(0,
                            pszClassname,
                            pszControlText?pszControlText:"",
                            dwStyle,
                            &r,
                            pParent->GetHwnd(),
                            (HMENU)ID
                           );
}


int FControl::GetID()
{
        return GetDlgCtrlID(GetHwnd());
}


int FControlTextHandler::GetTextLen()
{
        return (int)SendMessage(GetWnd()->GetHwnd(),WM_GETTEXTLENGTH,0,0);
}

Bool FControlTextHandler::GetText(char *buffer, int maxbuf)
{
        LRESULT l=SendMessage(GetWnd()->GetHwnd(), WM_GETTEXT, maxbuf, (LPARAM)(LPSTR)buffer);
        if(l<=0)
                return False;
        else
                return True;
}

Bool FControlTextHandler::SetText(const char *pszText)
{
        LRESULT r=SendMessage(GetWnd()->GetHwnd(), WM_SETTEXT, 0, (LPARAM)(LPSTR)pszText);
        if(r!=TRUE)
                return False;
        else
                return True;
}


