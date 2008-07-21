/***
Filename: frame.cpp
Description:
  Implementation of FFrame and related stuff
Host:
  BC40, WC10, SC61, VC42
History:
  ISJ 94-11-?? Creation
  ISJ 97-02-18 Added support vor MSVC++ 4.2
  ISJ 97-02-22 Added support for WM_SETICON
***/
#define BIFINCL_WINDOW
#define BIFINCL_FRAME
#define BIFINCL_APPLICATION
#include <bif.h>
#include <dbglog.h>

//FFrameIconHandler--------------------------------------------------------
FFrameIconHandler::FFrameIconHandler(FFrameWindow *pff)
  : FHandler(pff), theFrame(pff)
{
        SetDispatch(dispatchFunc_t(&FFrameIconHandler::Dispatch));
}

#if BIFOS_==BIFOS_WIN16_
//wm_painticon is undocumented for win3.1, but documented for winNT and Win32s
#define WM_PAINTICON 0x0026
#endif

Bool FFrameIconHandler::Dispatch(FEvent &event) {
        switch(event.GetID()) {
                case WM_PAINT:
                        if(IsIconic(theFrame->GetHwnd()))
                                return PaintIcon(event);
                        else
                                return False;
                case WM_PAINTICON:
                        return PaintIcon(event);

                case WM_ERASEBKGND:
                        if(IsIconic(theFrame->GetHwnd()))
                                return EraseIconBkgnd(event);
                        else
                                return False;

                case WM_ICONERASEBKGND:
                        return EraseIconBkgnd(event);

                case WM_QUERYDRAGICON:
                        return QueryDragIcon(event);
                default:
                        return False;
        }
}



//FFrameMenuDestroyHandler--------------------------------------------------------
//Since the frame window destroys the menu itself and doesn't let Windows do it,
//the debugging Windows user.exe will complain. This handler fixes it by removing
//the menu before the window is destroyed
FFrameMenuDestroyHandler::FFrameMenuDestroyHandler(FFrameWindow *pff)
  : FHandler(pff)
{
        SetDispatch(dispatchFunc_t(&FFrameMenuDestroyHandler::Dispatch));
}

Bool FFrameMenuDestroyHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_DESTROY)
                SetMenu(GetWnd()->GetHwnd(),NULL);
        return False;
}


//FFrameAccelHandler--------------------------------------------------------------
FFrameAccelHandler::FFrameAccelHandler(FFrameWindow *pff)
  : FHandler(pff)
{
        SetDispatch(dispatchFunc_t(&FFrameAccelHandler::Dispatch));
}

Bool FFrameAccelHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==GetWndMan()->wm_created)
                GetWndMan()->AddPumpWindow(GetWnd());
        else if(ev.GetID()==WM_DESTROY)
                GetWndMan()->RemovePumpWindow(GetWnd());
        return False;
}

//FFrameWindow--------------------------------------------------------------------
FFrameWindow::FFrameWindow()
  : FWnd(),
    FFrameIconHandler(this),
    FFrameMenuDestroyHandler(this),
    FFrameAccelHandler(this)
{
        TRACE_METHOD1("FFrameWindow::FFrameWindow");

        accel = NULL;
        pAutoIcon=pIcon=0;
        caption=0;
        subtitle=0;
        pAutoMenu=pMenu=0;
        pSysMenu=0;
}

FFrameWindow::~FFrameWindow() {
        TRACE_METHOD1("FFrameWindow::~FFrameWindow");
        if(accel) {
                //Windows frees the accelerator table automatically
                accel=NULL;
        }
        if(pAutoIcon) {
                delete pAutoIcon;
                pAutoIcon=0;
        }
        if(caption) {
                delete[] caption;
                caption=0;
        }
        if(subtitle) {
                delete[] subtitle;
                subtitle=0;
        }
        if(pAutoMenu) {
                delete pAutoMenu;
                pAutoMenu=0;
        }
        if(pSysMenu) {
                delete pSysMenu;
                pSysMenu=0;
        }
}


/***
Visibility: protected
Description:
        Handle accelerator keys
***/
Bool FFrameWindow::PumpEvent(FEvent &event) {
        TRACE_METHOD1("FFrameWindow::PumpEvent");
        if(accel) {
                if(event.hwnd==GetHwnd() ||
                   ::IsChild(GetHwnd(),event.hwnd))
                {
                        if(TranslateAccelerator(GetHwnd(), accel, &event) != 0) {
                                TRACE_TEXT1("FFrameWindow::PumpEvent: accelerator translated");
                                return True;
                        }
                }
        }
        return False;
}


Bool FFrameWindow::Create(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect, FModule *module) {
        TRACE_METHOD1("FFrameWindow::Create");

        //Translate symbolic names to WS_weird_stuff
        DWORD dwStyle=WS_OVERLAPPED;
        DWORD dwExStyle=0;
        if(fcf&fcf_sysmenu)
                dwStyle|=WS_CAPTION|WS_SYSMENU;
        if(fcf&fcf_titlebar)
                dwStyle|=WS_CAPTION;
        if(fcf&fcf_minbutton)
                dwStyle|=WS_MINIMIZEBOX|WS_CAPTION;
        if(fcf&fcf_maxbutton)
                dwStyle|=WS_MAXIMIZEBOX|WS_CAPTION;
        //Windows doesn't need this one:
        //if(fcf&fcf_menu)
        //      dwStyle|=;
        if(fcf&fcf_horzscroll)
                dwStyle|=WS_HSCROLL;
        if(fcf&fcf_vertscroll)
                dwStyle|=WS_VSCROLL;
        if((fcf&0x0300) == fcf_border)
                dwStyle|=WS_BORDER;
        if((fcf&0x0300) == fcf_sizeborder)
                dwStyle|=WS_THICKFRAME;
        if((fcf&0x0300) == fcf_dialogborder)
                dwExStyle|=WS_EX_DLGMODALFRAME;

        //determine x,y,cx,cy
        FRect rect;
        if(pRect) {
                rect=*pRect;
        } else {
                //Let Windows decide position and size
                rect.SetLeft(CW_USEDEFAULT);
                //rect.SetTop(0);       //ignored
#if defined(__BORLANDC__) || defined(_MSC_VER)
                //Borland and MSVC are picky...
                rect.SetRight(0);
#else
                rect.SetRight(CW_USEDEFAULT+CW_USEDEFAULT);
#endif
                //rect.SetBottom(0);    //ignored
        }

        if(!module) module=FModule::GetDefaultResourceModule();

        HWND hwndParent;
        if(pParent)
                hwndParent=pParent->GetHwnd();
        else
                hwndParent=HWND_DESKTOP;

        Bool c=FWnd::Create(dwExStyle,
                            (LPCSTR)NULL, (LPCSTR)NULL,
                            dwStyle,
                            &rect,
                            hwndParent,
                            (HMENU)NULL);
        if(c==NULL) {
                NOTE_TEXT1("FFrameWindow::Create: Creation failed");
                return False;
        }

        //load the standard stuff
        if(alf&alf_icon) {
                pAutoIcon=new FIcon(Id,module);
                SetIcon(pAutoIcon);
        }
        if(alf&alf_menu) {
                pAutoMenu=new FMenu(Id,module);
                SetMenu(pAutoMenu);
        }
        if(alf&alf_accelerator) {
                HACCEL accel=LoadAccelerators(module->GetHINSTANCE(), MAKEINTRESOURCE(Id));
                SetAccel(accel);
        }
        if(alf&alf_caption) {
                char s[128];
                if(module->LoadString(Id,s,128) !=0) {
                        if(s[0]=='-')
                                SetSubtitle(s+1);
                        else
                                SetCaption(s);
                }
        }

        return True;
}


Bool FFrameWindow::BringToFront() {
        TRACE_METHOD1("FFrameWindow::BringToFront");
        return (Bool)SetWindowPos(GetHwnd(), HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
}

Bool FFrameWindow::BringToBack() {
        TRACE_METHOD1("FFrameWindow::BringToBack");
        return (Bool)SetWindowPos(GetHwnd(), HWND_BOTTOM, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
}

Bool FFrameWindow::Minimize() {
        TRACE_METHOD1("FFrameWindow::Minimize");
        return (Bool)ShowWindow(GetHwnd(), SW_MINIMIZE);
}

Bool FFrameWindow::Maximize() {
        TRACE_METHOD1("FFrameWindow::Maximize");
        return (Bool)ShowWindow(GetHwnd(), SW_SHOWMAXIMIZED);
}

Bool FFrameWindow::Restore() {
        TRACE_METHOD1("FFrameWindow::Restore");
        return (Bool)ShowWindow(GetHwnd(), SW_RESTORE);
}


//menu management
FMenu *FFrameWindow::GetMenu() {
        TRACE_METHOD1("FFrameWindow::GetMenu");
        return pMenu;
}

void FFrameWindow::SetMenu(FMenu *m) {
        TRACE_METHOD1("FFrameWindow::SetMenu");
        if(m)
                ::SetMenu(GetHwnd(), m->GetHandle());
        else
                ::SetMenu(GetHwnd(), NULL);

        pMenu=m;
}

FMenu *FFrameWindow::GetSystemMenu() {
        TRACE_METHOD1("FFrameWindow::GetSystemMenu");
        if(pSysMenu)
                return pSysMenu;                        //return alias already made
        if((GetStyle()&WS_SYSMENU)==0)
                return 0;                               //window cannot have system menu
        HMENU hm=::GetSystemMenu(GetHwnd(),FALSE);
        if(!hm)
                return 0;                               //??? no sysmenu
        pSysMenu=new FMenu(hm,False);
        return pSysMenu;
}

void FFrameWindow::RedrawMenu() {
        TRACE_METHOD1("FFrameWindow::RedrawMenu");
        DrawMenuBar(GetHwnd());
}

FIcon *FFrameWindow::GetIcon() {
        TRACE_METHOD1("FFrameWindow::GetIcon");
        return pIcon;
}

void FFrameWindow::SetIcon(FIcon *pNewIcon) {
        TRACE_METHOD1("FFrameWindow::SetIcon");
        pIcon=pNewIcon;
        if(IsIconic(GetHwnd())) {
                //redraw icon
                RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME|RDW_INVALIDATE);
        }
#if WINVER >= 0x0400
        if(pIcon)
                SendMessage(GetHwnd(), WM_SETICON, (WPARAM)TRUE, (LPARAM)pIcon->GetHandle());
        else
                SendMessage(GetHwnd(), WM_SETICON, (WPARAM)TRUE, (LPARAM)0);
#endif
}

Bool FFrameWindow::SetCaption(const char *s) {
        TRACE_METHOD1("FFrameWindow::SetCaption");
        if(s) {
                if(caption) delete[] caption;
                caption=new char[lstrlen(s)+1];
                if(!caption) return False;
                lstrcpy(caption,s);
                UpdateTitle();
                return True;
        } else {
                if(caption) {
                        delete[] caption;
                        caption=0;
                        UpdateTitle();
                        return True;
                } else {
                        return True;
                }
        }
}

int FFrameWindow::GetCaption(char *buf, int buflen) {
        TRACE_METHOD1("FFrameWindow::GetCaption");
        if(caption) {
                if(lstrlen(caption)+1>buflen)
                        return 0;
                lstrcpy(buf,caption);
                return lstrlen(buf);
        } else
                return 0;
}

Bool FFrameWindow::SetSubtitle(const char *s) {
        TRACE_METHOD1("FFrameWindow::SetSubtitle");
        if(s) {
                if(subtitle) delete[] subtitle;
                subtitle=new char[lstrlen(s)+1];
                if(!subtitle) return False;
                lstrcpy(subtitle,s);
                UpdateTitle();
                return True;
        } else {
                if(subtitle) {
                        delete[] subtitle;
                        subtitle=0;
                        UpdateTitle();
                        return True;
                } else {
                        return True;
                }
        }
}

int FFrameWindow::GetSubtitle(char *buf, int buflen) {
        TRACE_METHOD1("FFrameWindow::GetSubtitle");
        if(subtitle) {
                if(lstrlen(subtitle)+1>buflen)
                        return 0;
                lstrcpy(buf,subtitle);
                return lstrlen(buf);
        } else
                return 0;
}


void FFrameWindow::UpdateTitle() {
        TRACE_METHOD0("FFrameWindow::UpdateTitle");

        char title[256];
        if(caption)
                if(subtitle)
                        wsprintf(title,"%s - %s",caption,subtitle);
                else
                        lstrcpy(title,caption);
        else
                if(subtitle)
                        wsprintf(title,"- %s",subtitle);
                else
                        title[0]='\0';

        SetWindowText(GetHwnd(),title);
}

void FFrameWindow::SetAccel(HACCEL haccel) {
        TRACE_METHOD1("FFrameWindow::SetAccel");
        //DestroyAcceleratorTable() is only available under Windows NT
        //So there is a memory leak here. Blame MicroSoft
        accel=haccel;
}

HACCEL FFrameWindow::GetAccel() {
        TRACE_METHOD1("FFrameWindow::GetAccel");
        return accel;
}


Bool FFrameWindow::EraseIconBkgnd(FEvent& ev) {
        TRACE_METHOD0("FFrameWindow::EraseIconBkgnd");
        //erase the icon background
        //Let the default action be taken by transforming the event into a iconerasebkgnd
        FEvent iebev=ev;
        iebev.message=WM_ICONERASEBKGND;
        Default(iebev);
        ev.SetResult(iebev.GetResult());
        return True;
}

Bool FFrameWindow::PaintIcon(FEvent& ev) {
        TRACE_METHOD0("FFrameWindow::PaintIcon");
        //paint the icon
        if(pIcon) {
                PAINTSTRUCT ps;
                BeginPaint(GetHwnd(),&ps);
                DrawIcon(ps.hdc,0,0,pIcon->GetHandle());
                EndPaint(GetHwnd(),&ps);

                ev.SetResult(0);
                return True;
        } else {
                //no icon
                //check to see if the window class has an icon. if so paint it
#if BIFOS_==BIFOS_WIN16_
                HICON hiconClass=(HICON)GetClassWord(GetHwnd(),GCW_HICON);
#else
                HICON hiconClass=(HICON)GetClassLong(GetHwnd(),GCL_HICON);
#endif
                if(hiconClass) {
                        PAINTSTRUCT ps;
                        BeginPaint(GetHwnd(),&ps);
                        DrawIcon(ps.hdc,0,0,hiconClass);
                        EndPaint(GetHwnd(),&ps);

                        ev.SetResult(0);
                        return True;
                } else
                        return False;   //no icon at all
        }
}


Bool FFrameWindow::QueryDragIcon(FEvent& ev) {
        TRACE_METHOD0("FFrameWindow::QueryDragIcon");
        if(pIcon) {
                ev.SetResult((long)pIcon->GetHandle());
                return True;
        } else {
                //no icon. We *must* return an icon handle
                ev.SetResult((long)LoadIcon(NULL,IDI_APPLICATION));
                return True;
        }
}

