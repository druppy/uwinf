/***
Filename: appwnd.cpp
Description:
  Implementation of applicationwindow and client
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
**/
#define INCL_DOSPROCESS
#define BIFINCL_WINDOW
#define BIFINCL_APPWINDOW
#define BIFINCL_APPLICATION
#include <bif.h>
#include <dbglog.h>
#include <string.h>
#include <stdlib.h>

FCommandEater::FCommandEater(FAppWindow *pwnd)
  : FHandler(pwnd)
{ 
        SetDispatch(dispatchFunc_t(&FCommandEater::Dispatch)); 
}

Bool FCommandEater::Dispatch(FEvent &ev) {
        //At least OS/2 Warp acts strange when a frame without a client receives a WM_COMMAND
        if(ev.GetID()==WM_COMMAND &&    //it's a wm_command
           WinWindowFromID(GetWnd()->GetHwnd(),FID_CLIENT)==NULLHANDLE) //no client
                return True;    //eat the event
        else
                return False;
}



FAppWindow::FAppWindow()
  : FFrameWindow(), FCommandEater(this)
{
        TRACE_METHOD1("FAppWindow::FAppWindow");
        theClient=0;
}

FAppWindow::~FAppWindow() {
        TRACE_METHOD1("FAppWindow::~FAppWindow");
        //nothing
}


/***
Visibility: public
Description:
        Set the clientwindow.
Return:
        The old client window, NULL if none
***/
FWnd *FAppWindow::SetClient(FWnd *pNewClient) {
        TRACE_METHOD1("FAppWindow::SetClient");
        FWnd *old=theClient;
        theClient=pNewClient;

        //ensure that the client has an ID of FID_CLIENT
        if(WinQueryWindowUShort(theClient->GetHwnd(),QWS_ID)!=FID_CLIENT)
                WinSetWindowUShort(theClient->GetHwnd(),QWS_ID,FID_CLIENT);

        //tell the frame to update
        WinSendMsg(GetHwnd(),WM_UPDATEFRAME,MPFROMLONG(0),NULL);

        return old;
}

/***
Visibility: public
Return:
        The current client window
***/
FWnd *FAppWindow::GetClient() {
        TRACE_METHOD1("FAppWindow::GetClient");
        return theClient;
}




//position save & restore---

/***
Visibility: local to this module
Description:
        Figure out the name of the current application
***/
static void getAppName(char *pszAppName) {
        PPIB ppib;
        PTIB ptib;
        DosGetInfoBlocks(&ptib,&ppib);
        if(ppib->pib_pchcmd) {
#ifdef __WATCOMC__
                char drive[_MAX_DRIVE],dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];
                _splitpath(ppib->pib_pchcmd,drive,dir,name,ext);
                strcpy(pszAppName,name);
#else
                //insert your favorite compiler here... 
                strcpy(pszAppName,ppib->pib_pchcmd);
#endif
        } else {
                strcpy(pszAppName,"BIF/Window-test-application");
        }
}

/***
Visibility: public
Description:
        Save the current position, size and state of the framewindow. The data
        is stored in a profile file
***/
Bool FAppWindow::SavePosition() {
        if(!IsValid()) return False;

        char szAppName[128];
        getAppName(szAppName);
        char szKeyName[128];
        GetCaption(szKeyName,128);

        return (Bool)WinStoreWindowPos(szAppName,szKeyName,GetHwnd());
}

/***
Visibility: public
Description:
        Restore the position, size and state from a profile file. If the data is
        not stored, or an error occurs, False is returned
***/
Bool FAppWindow::RestorePosition() {
        if(!IsValid()) return False;

        char szAppName[128];
        getAppName(szAppName);
        char szKeyName[128];
        GetCaption(szKeyName,128);

        return (Bool)WinRestoreWindowPos(szAppName,szKeyName,GetHwnd());
}


//client----------------------------------------------------------------------
Bool FClientWindow::Create(FAppWindow *pParent) {
        appwnd=pParent;
        FRect rect(0,0,0,0);
        Bool r=FWnd::Create("", WS_VISIBLE, &rect, pParent->GetHwnd(), FID_CLIENT);
        if(r) {
                pParent->SetClient(this);
        } else
                appwnd=0;
        return r;
}

FClientWindow::~FClientWindow() {
        if(appwnd && appwnd->IsValid())
                appwnd->SetClient(0);
}
