/***
Filename: control.cpp
Description:
  Implementation of FControl and FOwnerNotifyHandler
Host:
  Watcom 10.0a-10.5
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-05-01 Migrated from PM to WMS
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define WMSINCL_MSGS
#define WMSINCL_BW
#define BIFINCL_RESOURCETEMPLATE
#include <bif.h>
#include <dbglog.h>
#include <string.h>
#include <stdlib.h>

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


//WMSControlHandler------------------------------------------------------------
Bool WMSControlHandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_PAINT:
                        Paint();
                        return True;
                case WM_SETTEXT:
                        if(invalidateWhenTextChanges)
                                WmsInvalidateRect(GetWnd()->GetHwnd(), (FRect*)0, False);
                        return False;
                case WM_MATCHMNEMONIC:
                        ev.SetResult((WmsMRESULT)MatchMnemonic(UINT81FROMMP(ev.mp1)));
                        return True;
                case WM_QUERYDLGCODE:
                        ev.SetResult((WmsMRESULT)(QueryDlgCode()|DLGC_CONTROL));
                        return True;
                default:
                        return False;
        }
}

//FControl---------------------------------------------------------------------

/***
Visibility: public
Description:
        Create a control window from resource template
Parameters:
        pParent         parent window
        resID           id of the native window and id of resource item
Return:
        True on success
***/
Bool FControl::Create(FWnd *pParent, int resID) {
        TRACE_METHOD1("FControl::Create");

        //find template
        WmsMRESULT mr=WmsSendMsg(pParent->GetHwnd(), WM_QUERYRESOURCEITEMPTR, MPFROMUINT16(resID), 0);
        const DialogItemTemplate *restemplate=(const DialogItemTemplate *)PFROMMR(mr);
        if(!restemplate) return False;

        //create window
        FRect r(0,0,0,0);
        uint32 flStyle=WS_VISIBLE;
        Bool b=FWnd::Create(pParent->GetHwnd(),                        //parent
                            "",
                            flStyle,
                            &r,
                            pParent->GetHwnd(),                         //owner
                            HWND_BOTTOM,                                //insertbehind
                            (uint16)resID
                           );
        if(!b) return False;

        int props;
        char **name;
        char **value;
        if(!DialogResource::SplitSetup(restemplate->setup,&props,&name,&value)) {
                Destroy();
                return False;
        }
        b = ProcessResourceSetup(props,name,value);
        DialogResource::DeallocateSetup(name,value);
        if(!b) {
                Destroy();
                return False;
        } else
                return True;
}


/***
Visibility: public
Description:
        Create a control window
Parameters:
        pParent         parent window
        ID              id of the native window
        pszControlText  initial control text (can be NULL)
        flStyle         style of the native window
        rect            size/position of the window (can be NULL, but shouldn't be)
Return:
        True on success
Note:
        The window is always created visible
***/
Bool FControl::Create(FWnd *pParent, int ID,
                      const char *pszControlText,
                      uint32 flStyle, FRect *rect)
{
        TRACE_METHOD1("FControl::Create");

        FRect r(0,0,0,0);
        if(rect)
                r=*rect;

        flStyle|=WS_VISIBLE;
        return FWnd::Create(pParent?pParent->GetHwnd():0,               //parent
                            pszControlText?pszControlText:"",
                            flStyle,
                            &r,
                            pParent?pParent->GetHwnd():0,               //owner
                            HWND_BOTTOM,                                //insertbehind
                            (uint16)ID
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
        return (int)WmsQueryWindowID(GetHwnd());
}


/***
Visibility: protected
Description:
  Apply setup specified in the resource template
***/
Bool FControl::ProcessResourceSetup(int props, char *name[], char *value[]) {
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"x")==0 && value[p]) {
                        int x=atoi(value[p]);
                        FPoint p(0,0);
                        WmsMapWindowPoints(GetHwnd(),GetParent()->GetHwnd(),&p,1);
                        WmsSetWindowPos(GetHwnd(), 0, x,p.GetY(), 0,0, SWP_MOVE);
                } else if(strcmp(name[p],"y")==0 && value[p]) {
                        int y=atoi(value[p]);
                        FPoint p(0,0);
                        WmsMapWindowPoints(GetHwnd(),GetParent()->GetHwnd(),&p,1);
                        WmsSetWindowPos(GetHwnd(), 0, p.GetX(),y, 0,0, SWP_MOVE);
                } else if(strcmp(name[p],"width")==0 && value[p]) {
                        int w=atoi(value[p]);
                        FRect wr;
                        WmsQueryWindowRect(GetHwnd(),&wr);
                        WmsSetWindowPos(GetHwnd(), 0, 0,0, w,wr.GetHeight(), SWP_SIZE);
                } else if(strcmp(name[p],"height")==0 && value[p]) {
                        int h=atoi(value[p]);
                        FRect wr;
                        WmsQueryWindowRect(GetHwnd(),&wr);
                        WmsSetWindowPos(GetHwnd(), 0, 0,0, wr.GetWidth(),h, SWP_SIZE);
                } else if(strcmp(name[p],"visible")==0) {
                        Show();
                } else if(strcmp(name[p],"hidden")==0) {
                        Hide();
                } else if(strcmp(name[p],"tabstop")==0) {
                        SetStyle(~WS_TABSTOP,WS_TABSTOP);
                } else if(strcmp(name[p],"group")==0) {
                        SetStyle(~WS_GROUP,WS_GROUP);
                }
        }
        return True;
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
        return (int)WmsSendMsg(GetWnd()->GetHwnd(), WM_QUERYTEXTLENGTH, 0,0);
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
        WmsSendMsg(GetWnd()->GetHwnd(), WM_QUERYTEXT, MPFROMP(buffer), MPFROMUINT16(maxbuf));
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
        return (Bool)WmsSendMsg(GetWnd()->GetHwnd(), WM_SETTEXT, MPFROMP(pszText), 0);
}

