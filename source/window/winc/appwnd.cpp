/***
Filename: appwnd.cpp
Description:
  Implementation of applicationwindow and client
Host:
  win16: BC40, WC10-10.6, SC61
  win32: WC10.6
History:
  ISJ 94-11-?? Creation
  ISJ 96-09-01 Added forwarding of win95 and NT3.51 messages WM_NOTIFY and
               WM_NOTIFYFORMAT
  ISJ 96-10-06 Under win32 the window position will be saved in the registry
               instead of an INI file
               WM_SYSCOLORCHANGE is forwarded to the client, so it has a chance
               to forward the message if it has/is common controls
  ISJ 96-11-16 Forwarded palette messages to client too
  ISJ 97-02-16 Implemented frame formatting the OS/2 PM way
**/
#define BIFINCL_WINDOW
#define BIFINCL_APPWND
#define BIFINCL_APPLICATION
#include <bif.h>
#include <dbglog.h>
#include <string.h>


//registered messages for formatting the frame
static UINT wm_updateframe=0;
static UINT wm_queryframectlcount=0;;
static UINT wm_formatframe=0;
static UINT wm_queryformatsupport=0;

FClientForwardHandler::FClientForwardHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FClientForwardHandler::Dispatch));
}

/***
Visibility: protected
Description:
        Forward certain messages to the client window
***/
Bool FClientForwardHandler::Dispatch(FEvent& ev) {
        unsigned id=ev.GetID();

        //scroll commands
        if(id==WM_VSCROLL ||
           id==WM_HSCROLL)
                return ForwardToClient(ev);
        //close command
        if(id==WM_CLOSE)
                return ForwardToClient(ev);
        //menu notifications
        if(id==WM_INITMENU      ||
           id==WM_INITMENUPOPUP ||
           id==WM_MENUCHAR      ||
           id==WM_MENUSELECT)
                return ForwardToClient(ev);
        //control notifications&commands, menu commands and accelerator commands
        if(id==WM_COMMAND)
                return ForwardToClient(ev);
        //ownerdraw menu item drawing
        if(id==WM_MEASUREITEM && ev.GetWParam()==0 ||
           id==WM_DRAWITEM && ev.GetWParam()==0)
                return ForwardToClient(ev);
#if (BIFOS_==BIFOS_WIN32_)
#  if (WINVER >= 0x0400)
        if(id==WM_NOTIFY ||
           id==WM_NOTIFYFORMAT)
                return ForwardToClient(ev);
        if(id==WM_SYSCOLORCHANGE)
                return ForwardToClient(ev);
        if(id==WM_DISPLAYCHANGE)
                return ForwardToClient(ev);
#  endif
#endif
        if(id==WM_PALETTECHANGED ||
           id==WM_PALETTEISCHANGING ||
           id==WM_QUERYNEWPALETTE)
                return ForwardToClient(ev);
        return False;
}



/***
Visibility: ?
Description:
        React on size changes. The client window must have its size adjusted
***/
Bool FAppWindowResizeHandler::SizeChanged(FSizeEvent&) {
        TRACE_METHOD0("FAppWindowResizeHandler::SizeChanged");

        PostMessage(GetWnd()->GetHwnd(), wm_updateframe, 0,0);
        return True;
}


FAppWindowFocusForwarder::FAppWindowFocusForwarder(FAppWindow *pwnd)
  : FFocusHandler(pwnd),
    appwnd(pwnd)
{}


FFrameFormatHandler::FFrameFormatHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        if(wm_updateframe==0)        wm_updateframe       =RegisterWindowMessage("wm_updateframe");
        if(wm_formatframe==0)        wm_formatframe       =RegisterWindowMessage("wm_formatframe");
        if(wm_queryframectlcount==0) wm_queryframectlcount=RegisterWindowMessage("wm_queryframectlcount");
        if(wm_queryformatsupport==0) wm_queryformatsupport=RegisterWindowMessage("wm_queryformatsupport");


        SetDispatch(dispatchFunc_t(&FFrameFormatHandler::Dispatch));
}

Bool FFrameFormatHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==wm_updateframe)
                return UpdateFrame(ev);
        else if(ev.GetID()==wm_formatframe)
                return FormatFrame(ev);
        else if(ev.GetID()==wm_queryframectlcount)
                return QueryFrameCtlCount(ev);
        else if(ev.GetID()==wm_queryformatsupport) {
                ev.SetResult(TRUE);
                return True;
        } else
                return False;
};




/***
Visibility: ?
Description:
        Pass the focus on the the client window.
***/
Bool FAppWindowFocusForwarder::GotFocus(FSetFocusEvent&) {
        //pass the focus to the client
        if(appwnd->theClient && appwnd->theClient->IsValid()) {
                ForwardFocus(appwnd->theClient);
                return True;
        } else
                return False;
}



FAppWindow::FAppWindow()
  : FFrameWindow(),
    FClientForwardHandler(this),
    FFrameFormatHandler(this),
    resizehandler(this),
    focusforwarder(this)
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
        //swap pointers
        FWnd *old=theClient;
        theClient=pNewClient;
        
        //force FID_CLIENT id on the client
        if(theClient && theClient->IsValid())
#if BIFOS_==BIFOS_WIN16_
                SetWindowWord(theClient->GetHwnd(),GWW_ID,FID_CLIENT);
#else
                SetWindowLong(theClient->GetHwnd(),GWL_ID,FID_CLIENT);
#endif

        SendMessage(GetHwnd(), wm_updateframe, 0,0);
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




/***
Visibility: private
Description:
        Forward an event to the client window (if any)
***/
Bool FAppWindow::ForwardToClient(FEvent& ev) {
        TRACE_METHOD0("FAppWindow::ForwardToClient");
        if(theClient) {
                FEvent forwardEvent=ev;
                forwardEvent.SetHwnd(theClient->GetHwnd());
                SendEvent(forwardEvent);
                ev.SetResult(forwardEvent.GetResult());
                return True;
        } else
                return False;
}




/***
Visibility: protected
Description:
        When the size of the frame window has changed af wm_updateframe is posted
        wm_updateframe causes the framewindow to send a wm_queryframectlcoutn to
        itself to retreive the number for frame controls in it (normally just
        1 - the client window). This allows other frame controls (statusbars,
        toolbars, etc) to be easily integrated in the future.
        The frame the allocates an array of WINDOWPOS structures and sends a
        wm_formatframe to itself to fill out the array. Then it sizes and
        moves the frame controls.
***/
Bool FAppWindow::UpdateFrame(FEvent &) {
        //ask ourselves how many frame controls there are
        LRESULT ctls=SendMessage(GetHwnd(), wm_queryframectlcount, 0,0);
        if(ctls==0) return True; //no controls

        //allocate aray of WINDOWPOS
        WINDOWPOS *pswp = new WINDOWPOS[ctls];
        if(pswp==0) return False; //out of memory

        memset(pswp,0,(int)(sizeof(*pswp)*ctls));
        int i;
        for(i=0; i<ctls; i++) {
                pswp[i].flags = SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER;
                pswp[i].hwndInsertAfter = HWND_TOP;
        }
        

        //ask ourselves to fill out the pswp
        SendMessage(GetHwnd(), wm_formatframe, (WPARAM)ctls, (LPARAM)(LPWINDOWPOS)(pswp));

        //now move&size the frame controls
        HDWP hdwp = BeginDeferWindowPos((int)ctls);
        if(hdwp) {
                for(int i=0; i<ctls && hdwp; i++) {
                        if(IsWindow(pswp[i].hwnd) &&
                           (pswp[i].flags&(SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER)) != (SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER)
                          )
                                hdwp = DeferWindowPos(hdwp,
                                                      pswp[i].hwnd, pswp[i].hwndInsertAfter,
                                                      pswp[i].x, pswp[i].y,
                                                      pswp[i].cx, pswp[i].cy,
                                                      pswp[i].flags
                                                     );
                }
                if(hdwp)
                        if(EndDeferWindowPos(hdwp))
                                return True;
        }

        //deferwindowpos failed somehow

        for(i=0; i<ctls; i++) {
                SetWindowPos(pswp[i].hwnd, pswp[i].hwndInsertAfter,
                             pswp[i].x, pswp[i].y,
                             pswp[i].cx, pswp[i].cy,
                             pswp[i].flags
                            );
        }
        return True;
}

/***
Visibility: protected
Description: see UpdateFrame()
***/
Bool FAppWindow::QueryFrameCtlCount(FEvent &ev) {
        if(theClient && theClient->IsValid())
                ev.SetResult(1);
        else
                ev.SetResult(0);
        return True;
}

/***
Visibility: protected
Description: see UpdateFrame()
***/
Bool FAppWindow::FormatFrame(FEvent &ev) {
        if(theClient && theClient->IsValid()) {
                //since we are the end of the formatting chain and thus
                //executed first we can just use slot 0. Others need to scan
                //for an empty slot
                LPWINDOWPOS pswp = (LPWINDOWPOS)ev.GetLParam();
                pswp[0].hwnd = theClient->GetHwnd();
                pswp[0].hwndInsertAfter = HWND_TOP;
                pswp[0].x = 0;
                pswp[0].y = 0;
                FRect r; GetClientRect(GetHwnd(), &r);
                pswp[0].cx = r.GetWidth();
                pswp[0].cy = r.GetHeight();
                pswp[0].flags = 0;
        }
        return True;
}




//position save & restore---

/***
Visibility: local to this module
Description:
        Figure out the name of the current application
***/
static void getAppName(char *pszAppName) {
        //get application name of current application
        HINSTANCE hinst=GetCurrentApp()->GetHINSTANCE();
        char filename[255];
        GetModuleFileName(hinst,filename,255);
        //skip drive and directory
        char *p=filename+lstrlen(filename);
        while(p>filename && *p!='\\') p--;
        if(*p=='\\') p++;
        lstrcpy(pszAppName,p);
}


#if (BIFOS_==BIFOS_WIN16_)
/***
Visibility: local to this module
Description:
***/
static BOOL WritePrivateProfileInt(const char *app, const char *key, int value, BOOL fSigned, const char *iniFile)
{
        //This is silly, Windows doesn't provide at WriteXxxxInt even though it
        //provides a GetXxxxInt
        char buf[10];
        if(fSigned)
                wsprintf(buf,"%d",value);
        else
                wsprintf(buf,"%u",value);
        return WritePrivateProfileString(app,key,buf,iniFile);
}
#define WP_ININAME "wnd-pos.ini"
#endif

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

        WINDOWPLACEMENT wndpl;
        wndpl.length=sizeof(wndpl);
        if(!GetWindowPlacement(GetHwnd(),&wndpl))
                return False;
        else {
#if (BIFOS_==BIFOS_WIN16_)
                BOOL f=TRUE;
                char entry[128];
                f=f&&WritePrivateProfileInt(szAppName,szKeyName,1,FALSE,WP_ININAME);
                wsprintf(entry,"%s.showCmd",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.showCmd,FALSE,WP_ININAME);
                wsprintf(entry,"%s.min.x",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.ptMinPosition.x,FALSE,WP_ININAME);
                wsprintf(entry,"%s.min.y",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.ptMinPosition.y,FALSE,WP_ININAME);
                wsprintf(entry,"%s.max.x",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.ptMaxPosition.x,FALSE,WP_ININAME);
                wsprintf(entry,"%s.max.y",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.ptMaxPosition.y,FALSE,WP_ININAME);
                wsprintf(entry,"%s.normal.left",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.rcNormalPosition.left,FALSE,WP_ININAME);
                wsprintf(entry,"%s.normal.top",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.rcNormalPosition.top,FALSE,WP_ININAME);
                wsprintf(entry,"%s.normal.right",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.rcNormalPosition.right,FALSE,WP_ININAME);
                wsprintf(entry,"%s.normal.bottom",szKeyName);
                f=f&&WritePrivateProfileInt(szAppName,entry,(int)wndpl.rcNormalPosition.bottom,FALSE,WP_ININAME);
                return (Bool)f;
#else
                //Use registry as nice application should do
                HKEY hk;
                DWORD disposition;
                LONG l;
                l=RegCreateKeyEx(HKEY_CURRENT_USER,             //window positions is saved per user
                                 "Software\\SavedWindowPositions",
                                 0,                             //reserved
                                 0,                             //No class
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_READ|KEY_WRITE,
                                 0,                             //no security
                                 &hk,
                                 &disposition
                                );
                if(l!=ERROR_SUCCESS)
                        return False;

                HKEY hkapp;
                l=RegCreateKeyEx(hk,
                                 szAppName,
                                 0,
                                 0,
                                 REG_OPTION_NON_VOLATILE,
                                 KEY_ALL_ACCESS,
                                 0,
                                 &hkapp,
                                 &disposition
                                );
                RegCloseKey(hk);
                if(l!=ERROR_SUCCESS)
                        return False;
                                 
                l=RegSetValueEx(hkapp, szKeyName, 0, REG_BINARY, (BYTE *)&wndpl, sizeof(wndpl));
                RegCloseKey(hkapp);
                
                if(l!=ERROR_SUCCESS)
                        return False;
                else
                        return True;
#endif
        }
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

#if (BIFOS_==BIFOS_WIN16_)
        if(!GetPrivateProfileInt(szAppName,szKeyName,0,WP_ININAME))
                return False;
        else {
                WINDOWPLACEMENT wndpl;
                memset(&wndpl,0,sizeof(wndpl));
                wndpl.length=sizeof(wndpl);
                wndpl.flags=0;
                
                char entry[128];
                wsprintf(entry,"%s.showCmd",szKeyName);
                wndpl.showCmd=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                wsprintf(entry,"%s.min.x",szKeyName);
                wndpl.ptMinPosition.x=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                wsprintf(entry,"%s.min.y",szKeyName);
                wndpl.ptMinPosition.y=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                wsprintf(entry,"%s.max.x",szKeyName);
                wndpl.ptMaxPosition.x=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                wsprintf(entry,"%s.max.y",szKeyName);
                wndpl.ptMaxPosition.y=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                wsprintf(entry,"%s.normal.left",szKeyName);
                wndpl.rcNormalPosition.left=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                wsprintf(entry,"%s.normal.top",szKeyName);
                wndpl.rcNormalPosition.top=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                wsprintf(entry,"%s.normal.right",szKeyName);
                wndpl.rcNormalPosition.right=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                wsprintf(entry,"%s.normal.bottom",szKeyName);
                wndpl.rcNormalPosition.bottom=GetPrivateProfileInt(szAppName,entry,0,WP_ININAME);
                if(SetWindowPlacement(GetHwnd(),&wndpl))
                        return True;
                else
                        return False;
        }
#else
        //use registry
        HKEY hk;
        LONG l;
        l=RegOpenKeyEx(HKEY_CURRENT_USER,
                       "Software\\SavedWindowPositions",
                       0,
                       KEY_READ,
                       &hk);
        if(l!=ERROR_SUCCESS)
                return False;

        HKEY hkapp;
        l=RegOpenKeyEx(hk,
                       szAppName,
                       0,
                       KEY_READ,
                       &hkapp);
        RegCloseKey(hk);
        if(l!=ERROR_SUCCESS)
                return False;

        WINDOWPLACEMENT wndpl;
        DWORD dwType;
        DWORD cbData=sizeof(wndpl);
        l=RegQueryValueEx(hkapp, szKeyName, (LPDWORD)0, &dwType, (BYTE*)&wndpl, &cbData);
        RegCloseKey(hkapp);
        if(l!=ERROR_SUCCESS || dwType!=REG_BINARY || cbData!=sizeof(wndpl))
                return False;
        if(SetWindowPlacement(GetHwnd(),&wndpl))
                return True;
        else
                return False;
#endif
}


//client----------------------------------------------------------------------
Bool FClientWindow::Create(FAppWindow *pParent) {
        appwnd=pParent;
        FRect rect(0,0,0,0);
        Bool r=FWnd::Create(0, WS_CHILD|WS_VISIBLE, &rect, pParent->GetHwnd(), (HMENU)FID_CLIENT);
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
