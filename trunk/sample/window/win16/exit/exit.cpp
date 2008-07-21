#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_APPWND
#define BIFINCL_COMMONHANDLERS
#include <bif.h>
#include "exit.h"

#define MID_EXITWINDOWS 5000

class QOHandler : public FHandler {
        //this handler catches Windows' WM_QUERYOPEN message
public:
        QOHandler(FWnd *pwnd);
        Bool Dispatch(FEvent&);
        virtual Bool QueryOpen(FEvent&) { return False; }
};

QOHandler::QOHandler(FWnd *pwnd)
  : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&QOHandler::Dispatch));
}

Bool QOHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_QUERYOPEN)
                return QueryOpen(ev);
        else
                return False;
};

class WinExitFrame : public FFrameWindow, public FCommandHandler, public QOHandler {
        FIcon icon;
public:
        WinExitFrame() : FFrameWindow(), FCommandHandler(this), QOHandler(this), icon(ID_EXIT) {}
        
        Bool Create();

        Bool SysCommand(FSysCommandEvent &ev);
        virtual Bool QueryOpen(FEvent&);
};

Bool WinExitFrame::Create() {
        //create the frame and minimize it
        //unfortunatly we have to bypass FFrameWindows create mechanism because
        //we have to create the window initially minimized, no activation and
        //invisible
        FRect pos(CW_USEDEFAULT,0,CW_USEDEFAULT,0);
        if(!FWnd::Create(0,                             //default window class
                         WS_BORDER|WS_CAPTION|WS_MINIMIZE|WS_OVERLAPPED|WS_SYSMENU|WS_VISIBLE,
                         &pos,                             //default position
                         HWND_DESKTOP,                  
                         0                              //no menu
                        )
          )
                return False;
        SetIcon(&icon);
        SetCaption(FResourceString(ID_EXIT));
        ShowWindow(GetHwnd(), SW_SHOWMINNOACTIVE);
        
        //modify the system menu
        FMenu *sysmenu=GetSystemMenu();
        //add our own item
        InsertMenu(sysmenu->GetHandle(), 0, MF_BYPOSITION, MID_EXITWINDOWS, FResourceString(IDS_EXIT));
        //remove unused items
        DeleteMenu(sysmenu->GetHandle(), SC_MINIMIZE, MF_BYCOMMAND);
        DeleteMenu(sysmenu->GetHandle(), SC_RESTORE, MF_BYCOMMAND);
        DeleteMenu(sysmenu->GetHandle(), SC_MAXIMIZE, MF_BYCOMMAND);
        DeleteMenu(sysmenu->GetHandle(), SC_SIZE, MF_BYCOMMAND);
        
        return True;
}

Bool WinExitFrame::SysCommand(FSysCommandEvent &ev) {
        if(ev.GetItem()==MID_EXITWINDOWS) {
                ExitWindows(0,0);
                return True;
        }
        return False;
}

Bool WinExitFrame::QueryOpen(FEvent&) {
        ExitWindows(0,0);
        return True;
}


class WinExitApp : public FApplication {
        WinExitFrame frame;
public:
        Bool StartUp(int,char**) {
                if(!frame.Create())
                        return False;
                frame.Show();
                return True;
        }
        int ShutDown() {
                frame.Destroy();
                return 0;
        }
};


DEFBIFWINDOWMAIN(WinExitApp);

