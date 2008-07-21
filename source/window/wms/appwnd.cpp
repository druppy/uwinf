/***
Filename: appwnd.cpp
Description:
  Implementation of applicationwindow and client
Host:
  WC 10.6
History:
  ISJ 94-11-?? Creation
  ISJ 96-05-26 Creation (migrated from win16)
**/
#define BIFINCL_WINDOW
#define BIFINCL_APPWND
#define BIFINCL_APPLICATION
#define WMSINCL_MSGS
#include <bif.h>
#include <dbglog.h>

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
        WmsHWND hwndClient = WmsWindowFromID(GetWnd()->GetHwnd(),FID_CLIENT);

        if(id==WM_CLOSE) {
                if(hwndClient)
                        WmsPostMsg(hwndClient, ev.GetID(), ev.mp1, ev.mp2);
                else
                        WmsPostMsg(GetWnd()->GetHwnd(), WM_QUIT, 0, 0);
                return True;
        }

        if(!hwndClient) 
                return False;

        //scroll commands
        if(id==WM_CONTROL) {
                WmsPostMsg(hwndClient, ev.GetID(), ev.mp1, ev.mp2);
                return True;
        }

        //menu notifications
        //todo
/*        if(id==WM_INITMENU      ||
           id==WM_INITMENUPOPUP ||
           id==WM_MENUCHAR      ||
           id==WM_MENUSELECT) {
                ev.SetResult(WmsSendMsg(hwndClient, ev.GetID(), ev.mp1, ev.mp2));
                return True;
        }
*/
        //control notifications&commands, menu commands and accelerator commands
        if(id==WM_COMMAND) {
                WmsPostMsg(hwndClient, ev.GetID(), ev.mp1, ev.mp2);
                return True;
        }

/*        //ownerdraw menu item drawing
        //todo:
        if(id==WM_MEASUREITEM && ev.GetWParam()==0 ||
           id==WM_DRAWITEM && ev.GetWParam()==0) {
                ev.SetResult(WmsSendMsg(hwndClient, ev.GetID(), ev.mp1, ev.mp2));
                return True;
        }
*/
        return False;
}



FAppWindowFocusForwarder::FAppWindowFocusForwarder(FAppWindow *pwnd)
  : FFocusHandler(pwnd),
    appwnd(pwnd)
{}

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



FAppWindowFormatFrameHandler::FAppWindowFormatFrameHandler(FWnd *pwnd)
  : FHandler(pwnd)
{ SetDispatch(dispatchFunc_t(FAppWindowFormatFrameHandler::Dispatch)); }

/***
Visibility: ?
Description:
        Catch WM_QUERYFRAMECTLCOUNT and WM_FORMATFRAME in order to let the
        frame size and position the client window
***/
Bool FAppWindowFormatFrameHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_QUERYFRAMECTLCOUNT) {
                if(WmsWindowFromID(GetWnd()->GetHwnd(),FID_CLIENT)==0)
                        return False;
                //tell the frame that the client is also there
                GetWnd()->CallHandlerChain(ev,this,True);
                ev.SetResult(MRFROMUINT32(UINT32FROMMR(ev.GetResult())+1));
                return True;
        } else if(ev.GetID()==WM_FORMATFRAME) {
                WmsHWND hwndClient = WmsWindowFromID(GetWnd()->GetHwnd(),FID_CLIENT);
                if(hwndClient==0)
                        return False;
                //let the frame do its best and then position the client
                GetWnd()->CallHandlerChain(ev,this,True);
                FRect *cr=(FRect*)PFROMMP(ev.mp2);
                WmsSWP *p=(WmsSWP*)PFROMMP(ev.mp1);
                //find free entry in WmsSWP array
                while(p->hwnd)
                        p++;
                //insert client
                p->hwnd = hwndClient;
                p->x = cr->xLeft;
                p->y = cr->yTop;
                p->cx = cr->GetWidth();
                p->cy = cr->GetHeight();
                if(p->cx<0) p->cx=0;
                if(p->cy<0) p->cy=0;
                p->flags = SWP_SIZE|SWP_MOVE;
                return True;
        } else
                return False;
}

FAppWindow::FAppWindow()
  : FFrameWindow(),
    FClientForwardHandler(this),
    focusforwarder(this),
    formatframehandler(this)
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
        //tell the frame ro reformat
        WmsSendMsg(GetHwnd(), WM_UPDATEFRAME, 0, 0);
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
Visibility: public
Description:
        Save the current position, size and state of the framewindow. The data
        is stored in a profile file
***/
//todo:
Bool FAppWindow::SavePosition() {
        return False;
}

/***
Visibility: public
Description:
        Restore the position, size and state from a profile file. If the data is
        not stored, or an error occurs, False is returned
***/
//todo:
Bool FAppWindow::RestorePosition() {
        return False;
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