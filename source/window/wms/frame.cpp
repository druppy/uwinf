/***                    
Filename: control.cpp
Description:
  Implementation of FFrame
  The mecanisms use are strongly influenced by the way PM handles frames
  (Ok, we admit it, it IS the way PM handles frames)
Host:
  OS/2: Watcom 10.6
  DOS: Watcom 10.6
History:
  ISJ 96-05-15 - ? Creation
  ISJ 96-08-14 Optimized MouseActivity() so it won't change focus if a child 
               already has the focus. This showed up on SMP systems
  ISJ 96-10-15 Implemented support for menus
  ISJ 97-02-16 Implemented WM_SAVEAPPLICATION
***/
#define BIFINCL_WINDOW
#define BIFINCL_FRAME
#define BIFINCL_TRACKRECT
#define BIFINCL_CONTROLS
#define BIFINCL_SCROLLBAR
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#define BIFINCL_RESOURCETEMPLATE
#define BIFINCL_APPLICATION
#define WMSINCL_CPI
#define WMSINCL_MSGS
#define WMSINCL_DESKTOP
#define WMSINCL_FOCUS
#include <bif.h>

#include "presparm.h"
#include "switchls.h"

#include <string.h>

//undocumented messages because our system menu does not have a MS_ACTIONBAR owner
//We leave it to Andrew Schulman to document it :-)
#define SC_LEFTKEY 0x8100
#define SC_RIGHTKEY 0x8101

//FFrameAccelHandler--------------------------------------------------------------------------------------------
FFrameAccelHandler::FFrameAccelHandler(FWnd *pwnd)
  : FHandler(pwnd),
    accel(0),
    sysaccel(0)
{ SetDispatch(dispatchFunc_t(FFrameAccelHandler::Dispatch)); }

Bool FFrameAccelHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()!=WM_KEYBOARD) 
                return False;
        FEvent tev;
        if((accel && TranslateAccelerator(*accel, (FKeyboardEvent&)ev, tev)) ||
           TranslateAccelerator(*sysaccel, (FKeyboardEvent&)ev, tev)) {
                GetWnd()->PostEvent(tev);
                return True;
        }
        return False;
}

//FFrameHandler-------------------------------------------------------------------------------------------------

FFrameHandler::FFrameHandler(FFrameWindow *pwnd)
  : FHandler(pwnd)
{ SetDispatch(dispatchFunc_t(FFrameHandler::Dispatch)); }

Bool FFrameHandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_PAINT:
                        return Paint();
                case WM_TRACKFRAME:
                        return TrackFrame(UINT32FROMMP(ev.mp1),UINT32FROMMP(ev.mp2));
                case WM_QUERYTRACKINFO:
                        return QueryTrackInfo(UINT32FROMMP(ev.mp1),PFROMMP(ev.mp2));
                case WM_QUERYWINDOWSTATE:
                        ev.SetResult((WmsMRESULT)QueryWindowState());
                        return True;
                case WM_SETWINDOWSTATE:
                        ev.SetResult((WmsMRESULT)SetWindowState((unsigned)UINT161FROMMP(ev.mp1)));
                        return True;
                case WM_QUERYACTIVE:
                        ev.SetResult((WmsMRESULT)QueryActive());
                        return True;
                case WM_SETACTIVE:
                        ev.SetResult((WmsMRESULT)SetActive((Bool)UINT161FROMMP(ev.mp1)));
                        return True;
                case WM_CALCFRAMERECT:
                        ev.SetResult((WmsMRESULT)CalcFrameRect((FRect*)PFROMMP(ev.mp1),(Bool)UINT161FROMMP(ev.mp2)));
                        return True;
                case WM_UPDATEFRAME:
                        ev.SetResult((WmsMRESULT)UpdateFrame(UINT32FROMMP(ev.mp1)));
                        return True;
                case WM_QUERYFRAMECTLCOUNT:
                        ev.SetResult((WmsMRESULT)QueryFrameControlCount());
                        return True;
                case WM_FORMATFRAME:
                        ev.SetResult((WmsMRESULT)FormatFrame((WmsSWP*)PFROMMP(ev.mp1), (FRect*)PFROMMP(ev.mp2)));
                        return True;
                case WM_ADJUSTWINDOWPOS:
                        return AdjustWindowPos((WmsSWP*)PFROMMP(ev.mp1));
                case WM_WINDOWPOSCHANGED:
                        return WindowPosChanged((WmsSWP*)PFROMMP(ev.mp1));
                case WM_OWNERPOSCHANGING:
                        return OwnerPosChanging((WmsSWP*)PFROMMP(ev.mp1), (WmsSWP*)PFROMMP(ev.mp2));
                case WM_OWNERSTATECHANGING:
                        return OwnerStateChanging(UINT161FROMMP(ev.mp1),UINT162FROMMP(ev.mp1));
                case WM_MOUSEACTIVITY:
                        return MouseActivity((FMouseEvent&)ev);
                case WM_FOCUSCHANGING:
                        return FocusChanging(HWNDFROMMP(ev.mp1), (Bool)UINT161FROMMP(ev.mp2), UINT162FROMMP(ev.mp2));
                case WM_QUERYFRAMEINFO:
                        ev.SetResult(MRFROMUINT32(QueryFrameInfo()));
                        return True;
                case WM_SETTEXT:
                        TextChanging((const char *)PFROMMP(ev.mp1));
                        return False;
                case WM_NEXTMENU:
                        ev.SetResult(MRFROMHWND(NextMenu(HWNDFROMMP(ev.mp1),(Bool)UINT161FROMMP(ev.mp2))));
                        return True;
                case WM_DESTROY:
                        WmsSendMsg(GetWnd()->GetHwnd(), WM_SAVEAPPLICATION, 0,0);
                        return False;
                default:
                        return False;
        }
}

void FFrameHandler::Invalidate(Bool includeChildren) {
        WmsInvalidateRect(GetWnd()->GetHwnd(), 0, includeChildren);
}


//FTitleBar----------------------------------------------------------------------------------------------------
class FTitleBar : public FWnd,
                  public FPaintHandler,
                  public FMouseHandler
{
public:
        FTitleBar()
          : FWnd(),
            FPaintHandler(this),
            FMouseHandler(this)
          {}
        Bool Create(FFrameWindow *fw);
protected:
        Bool Paint(FPaintEvent &);
        Bool Button1Down(FButtonDownEvent &);
        Bool Button1DblClk(FButtonDblClkEvent &);
        Bool ControlPointer(FControlPointerEvent &ev);
};

Bool FTitleBar::Create(FFrameWindow *fw) {
        FRect r(0,0,0,0);
        return FWnd::Create("", WS_VISIBLE, &r, fw->GetHwnd(), FID_TITLEBAR);
}

Bool FTitleBar::Paint(FPaintEvent &) {
        WmsHDC hdc=WmsBeginPaint(GetHwnd(),0);
        if(!hdc) return False;

        uint8 clr_foreground,clr_background;
        Bool isActive=(Bool)WmsSendMsg(WmsQueryWindowParent(GetHwnd()),WM_QUERYACTIVE,0,0);
        if(isActive) {
                WmsQueryPresParam(GetHwnd(), PP_ACTIVETEXTFOREGROUNDCOLOR, PP_ACTIVEBORDERCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_ACTIVETEXTBACKGROUNDCOLOR, PP_ACTIVEBORDERCOLOR, 0, &clr_background, 1);
        } else {
                WmsQueryPresParam(GetHwnd(), PP_INACTIVETEXTFOREGROUNDCOLOR, PP_INACTIVEBORDERCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_INACTIVETEXTBACKGROUNDCOLOR, PP_INACTIVEBORDERCOLOR, 0, &clr_background, 1);
        }
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        CpiEraseRect(hdc,r,clr_foreground|clr_background);

        char buf[256];
        WmsSendMsg(WmsQueryWindowParent(GetHwnd()), WM_QUERYTEXT, MPFROMP(buf), MPFROMUINT16(256));
        CpiSetTextColor(hdc,(color)clr_foreground);
        CpiSetBkMode(hdc,bm_transparent);
        CpiOuttext(hdc,r.xLeft+1,r.yTop,buf);

        WmsReleaseDC(hdc);
        return True;
}

Bool FTitleBar::Button1Down(FButtonDownEvent &ev) {
        if(ev.IsCtrlDown()) {
                WmsSendMsg(WmsQueryWindowParent(GetHwnd()), WM_TRACKFRAME, MPFROMUINT32(TF_MOVE), 0);
                return True;
        } else {
                WmsSendMsg(WmsQueryWindowParent(GetHwnd()), WM_TRACKFRAME, MPFROMUINT32(TF_MOVE), MPFROMUINT32(QSR_NOCLIPSIBLINGSABOVE));
                return False;
        }
}

Bool FTitleBar::Button1DblClk(FButtonDblClkEvent &) {
        uint16 ws=(uint16)WmsSendMsg(WmsQueryWindowParent(GetHwnd()), WM_QUERYWINDOWSTATE, 0,0);
        if(ws==FFrameWindow::window_minimized)
                WmsPostMsg(WmsQueryWindowParent(GetHwnd()), WM_SYSCOMMAND, MPFROMUINT16(SC_RESTORE),  MPFROM2UINT16(CMDSRC_OTHER,True));
        else if(ws==FFrameWindow::window_maximized)
                WmsPostMsg(WmsQueryWindowParent(GetHwnd()), WM_SYSCOMMAND, MPFROMUINT16(SC_RESTORE),  MPFROM2UINT16(CMDSRC_OTHER,True));
        else if(ws==FFrameWindow::window_normal)
                WmsPostMsg(WmsQueryWindowParent(GetHwnd()), WM_SYSCOMMAND, MPFROMUINT16(SC_MAXIMIZE), MPFROM2UINT16(CMDSRC_OTHER,True));
        return False;
}       

Bool FTitleBar::ControlPointer(FControlPointerEvent &ev) {
        ev.SetPointer(*GetSystemArrowPointer());
        return True;
}



//FTitleButton-------------------------------------------------------------------------------------------------
class FTitleBarButton : public FWnd,
                        public FMouseHandler,
                        public FPaintHandler
{
        WmsMSG WM_what; //wm_command/wm_syscommand/wm_help
        int SC_what;    //sc_xxx
        int PP_what;
public:
        FTitleBarButton(WmsMSG msg, int sc, int pp)
          : FWnd(),
            FMouseHandler(this),
            FPaintHandler(this),
            WM_what(msg),
            SC_what(sc),
            PP_what(pp)
           {}
        Bool Create(FFrameWindow *pOwner, FFrameWindow *pParent, int ID);
        Bool Button1Down(FButtonDownEvent&);
        Bool Paint(FPaintEvent &);
};

Bool FTitleBarButton::Create(FFrameWindow *pOwner, FFrameWindow *pParent, int ID) {
        FRect r(0,0,0,0);
        return FWnd::Create(pParent?pParent->GetHwnd():0,
                            "",
                            WS_VISIBLE,
                            &r,
                            pOwner?pOwner->GetHwnd():0,
                            HWND_TOP,
                            (uint16)ID
                           );
}

Bool FTitleBarButton::Button1Down(FButtonDownEvent&) {
        WmsPostMsg(WmsQueryWindowOwner(GetHwnd()), WM_what, MPFROMUINT16(SC_what), MPFROM2UINT16(CMDSRC_OTHER,True));
        return False;
}

Bool FTitleBarButton::Paint(FPaintEvent &) {
        WmsHDC hdc=WmsBeginPaint(GetHwnd(),0);
        if(!hdc) return False;

        char buttonchars[3];
        WmsQueryPresParam(GetHwnd(), PP_what, 0,0, buttonchars,3);

        Bool isActive=(Bool)WmsSendMsg(WmsQueryWindowParent(GetHwnd()),WM_QUERYACTIVE,0,0);
        uint16 clr,clr_foreground,clr_background;
        if(isActive)
                WmsQueryPresParam(GetHwnd(), PP_ACTIVEBORDERCOLOR, 0,0, &clr,1);
        else
                WmsQueryPresParam(GetHwnd(), PP_INACTIVEBORDERCOLOR,0,0, &clr,1);
        WmsQueryPresParam(GetHwnd(), PP_TITLEBUTTONFOREGROUNDCOLOR, 0,0, &clr_foreground,1);
        WmsQueryPresParam(GetHwnd(), PP_TITLEBUTTONBACKGROUNDCOLOR, 0,0, &clr_background,1);

        CpiSetCellData(hdc, FPoint(0,0), (uint16)(clr<<8|buttonchars[0]) );
        CpiSetCellData(hdc, FPoint(1,0), (uint16)((clr_foreground|clr_background)<<8|buttonchars[1]) );
        CpiSetCellData(hdc, FPoint(2,0), (uint16)(clr<<8|buttonchars[2]) );

        WmsReleaseDC(hdc);
        return True;
}


//FFrameWindow-------------------------------------------------------------------------------------------------
FFrameWindow::FFrameWindow() 
  : FWnd(), 
    FFrameHandler(this),
    FMouseHandler(this),
    FFrameAccelHandler(this),
    FCommandHandler(this),
    sysaccel(0xFFFE),
    accel(0)
{
        pTitleBar = 0;
        pSysMenuButton = pHideButton = pRestoreButton = pMinimizeButton = pMaximizeButton = 0;
        pVerticalScrollBar = pHorizontalScrollBar = 0;
        icon = 0;
        caption = 0;
        subtitle = 0;
        pMenu = pAutoMenu = 0;
        pSysMenu = 0;
        windowState = window_normal;
        nextWindowState = window_normal;
        isActive = False;
        owneePSWP=0;
        owneeSWPS=0;
        hiddenDueToOwner=False;
        FFrameAccelHandler::sysaccel = &sysaccel;
}

FFrameWindow::~FFrameWindow() {
        //delete this and that
        if(pTitleBar) delete pTitleBar;
        pTitleBar=0;

        if(pSysMenuButton) delete pSysMenuButton;
        pSysMenuButton=0;
        if(pHideButton) delete pHideButton;
        pHideButton=0;
        if(pRestoreButton) delete pRestoreButton;
        pRestoreButton=0;
        if(pMinimizeButton) delete pMinimizeButton;
        pMinimizeButton=0;
        if(pMaximizeButton) delete pMaximizeButton;
        pMaximizeButton=0;
        if(pVerticalScrollBar) delete pVerticalScrollBar;
        pVerticalScrollBar=0;
        if(pHorizontalScrollBar) delete pHorizontalScrollBar;
        pHorizontalScrollBar=0;

        if(pSysMenu) delete pSysMenu;
        pSysMenu=0;
        if(pAutoMenu) delete pAutoMenu;
        pAutoMenu=0;
        
        if(caption) delete[] caption;
        caption=0;
        if(subtitle) delete[] subtitle;
        subtitle=0;
        if(accel) delete accel;
        accel=0;
}

//globals for assigning positions to new frames
static FMutexSemaphore stockpos_mutex;
static FRect stockpos;
static Bool stockpos_initialized=False;

Bool FFrameWindow::Create(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect ) {
        if(!pParent) pParent=GetDesktop();

        uint32 style=WS_CLIPCHILDREN;
        if((fcf&0x0300)==fcf_border) style |= FS_THINBORDER;
        if((fcf&0x0300)==fcf_sizeborder) style |= FS_SIZEBORDER;
        if((fcf&0x0300)==fcf_dialogborder) style |= FS_MODALBORDER;
        //todo: more flags
        FResourceString restitle(Id);
        if(alf&alf_caption && ((const char *)restitle)==0)
                return False; //caption not found in resources
        if(alf&alf_accelerator) {
                accel = new FAcceleratorTable(Id);
                if(!accel) return False;
                if(!accel->GetAccelTable()) {
                        delete accel;
                        accel=0;
                        return False;
                }
                FFrameAccelHandler::accel = accel;
        }

        FRect r;
        if(pRect)
                r=*pRect;
        else {
                //generate a new position for the frame
                FRect pr;
                WmsQueryWindowRect(pParent->GetHwnd(),&pr);
                stockpos_mutex.Request();
                if(!stockpos_initialized) {
                        stockpos.Set(0,0,pr.GetWidth()/2,(pr.GetHeight()*2)/3);
                        stockpos_initialized=True;
                } else {
                        stockpos.MoveRelative(2,2);
                        if(stockpos.xRight>pr.xRight ||
                           stockpos.yBottom>pr.yBottom)
                                stockpos.MoveAbsolute(0,0);
                        if(stockpos.xRight>pr.xRight) stockpos.xRight=pr.xRight;
                        if(stockpos.yBottom>pr.yBottom) stockpos.yBottom=pr.yBottom;
                }
                r=stockpos;
                stockpos_mutex.Release();
        }

        Bool rc=FWnd::Create(pParent->GetHwnd(),
                             restitle,
                             style,
                             &r,
                             pParent->GetHwnd(),
                             HWND_TOP,
                             (uint16)Id
                            );
        if(!rc) return False;
        if(fcf&fcf_titlebar) {
                pTitleBar = new FTitleBar;
                if(!pTitleBar) goto fail;
                rc = pTitleBar->Create(this);
                if(!rc) goto fail;
        }
        if(fcf&fcf_sysmenu) {
                pSysMenuButton = new FTitleBarButton(WM_SYSCOMMAND, SC_SYSMENU, PP_TITLEBUTTONSYSMENUCHARS);
                if(!pSysMenuButton) goto fail;
                rc = pSysMenuButton->Create(this,this,FID_SYSMENU);
                if(!rc) goto fail;
        }
        if(fcf&fcf_minbutton) {
                pMinimizeButton = new FTitleBarButton(WM_SYSCOMMAND, SC_MINIMIZE, PP_TITLEBUTTONMINCHARS);
                if(!pMinimizeButton) goto fail;
                rc = pMinimizeButton->Create(this,this,FID_MINIMIZE);
                if(!rc) goto fail;
        }
        if(fcf&fcf_maxbutton) {
                pMaximizeButton = new FTitleBarButton(WM_SYSCOMMAND, SC_MAXIMIZE, PP_TITLEBUTTONMAXCHARS);
                if(!pMaximizeButton) goto fail;
                rc = pMaximizeButton->Create(this,this,FID_MAXIMIZE);
                if(!rc) goto fail;
        }
        if(fcf&(fcf_minbutton|fcf_maxbutton)) {
                pRestoreButton = new FTitleBarButton(WM_SYSCOMMAND, SC_RESTORE, PP_TITLEBUTTONRESTORECHARS);
                if(!pRestoreButton) goto fail;
                rc = pRestoreButton->Create(this,this,FID_RESTORE);
                if(!rc) goto fail;
        }
        if(fcf&fcf_hidebutton) {
                pHideButton = new FTitleBarButton(WM_SYSCOMMAND, SC_HIDE, PP_TITLEBUTTONHIDECHARS);
                if(!pHideButton) goto fail;
                rc = pHideButton->Create(this,this,FID_HIDE);
                if(!rc) goto fail;
        }
        if(fcf&fcf_vertscroll) {
                pVerticalScrollBar = new FScrollBar;
                if(!pVerticalScrollBar) goto fail;
                rc = pVerticalScrollBar->Create(this, FID_VERTSCROLL, FScrollBar::vertical, 0);
                if(!rc) goto fail;
        }
        if(fcf&fcf_horzscroll) {
                pHorizontalScrollBar = new FScrollBar;
                if(!pHorizontalScrollBar) goto fail;
                rc = pHorizontalScrollBar->Create(this, FID_HORZSCROLL, FScrollBar::horizontal, 0);
                if(!rc) goto fail;
        }
        if(fcf&fcf_sysmenu) {
                pSysMenu = new FMenu(this,0xFFFE);
                if(!pSysMenu) goto fail;
                //delete unused items
                if((fcf&fcf_minbutton)==0) pSysMenu->DeleteItem(SC_MINIMIZE);
                if((fcf&fcf_maxbutton)==0) pSysMenu->DeleteItem(SC_MAXIMIZE);
                if((fcf&fcf_hidebutton)==0) pSysMenu->DeleteItem(SC_HIDE);
        }
        if(fcf&fcf_menu) {
                pAutoMenu = new FMenu(this, Id);
                if(!pAutoMenu) goto fail;
                WmsSetWindowID(pAutoMenu->GetHandle(),FID_MENU);
                WmsSetWindowParent(pAutoMenu->GetHandle(),GetHwnd());
                pAutoMenu->TurnIntoActionBar();
                WmsSetWindowPos(pAutoMenu->GetHandle(), 0, 0,0,0,0, SWP_SHOW);
        }
        if(alf&alf_caption) {
                char s[128];
                if(GetCurrentApp()->LoadString(Id,s,128)!=0) { 
                        if(s[0]=='-')
                                SetSubtitle(s+1);
                        else
                                SetCaption(s);
                }
        }

        ShowHideTitleButtons(windowState);
        //Format_frame
        WmsSendMsg(GetHwnd(), WM_UPDATEFRAME, MPFROMUINT32(fcf), 0);

        if(fcf&fcf_titlebar) {
                //create switch entry
                FSwitchList::SwitchEntry e;
                e.pwnd = this;
                e.visible = True;
                e.jumpable = True;
                WmsSendMsg(GetHwnd(), WM_QUERYTEXT, MPFROMP(e.title), MPFROMUINT16(160));
                GetSwitchList()->AddEntry(&e);
        }

        return True;

fail:
        Destroy();
        return False;
}



//Window front/back--
Bool FFrameWindow::BringToFront() {
        return (Bool)WmsSetFocus(GetDesktop()->GetHwnd(), GetHwnd());
}

Bool FFrameWindow::BringToBack() {
        //the frame is not put at the bottom but just "out of the way"
        WmsSendMsg(GetHwnd(), WM_SYSCOMMAND, MPFROMUINT16(SC_NEXTFRAME), MPFROM2UINT16(CMDSRC_OTHER,False));
        return True;
}



//Window state---
/***
Visibility: public
***/
Bool FFrameWindow::Minimize() {
        WmsSendMsg(GetHwnd(), WM_SETWINDOWSTATE, MPFROMUINT16(window_minimized), 0);
        return True;
}
/***
Visibility: public
***/
Bool FFrameWindow::Maximize() {
        WmsSendMsg(GetHwnd(), WM_SETWINDOWSTATE, MPFROMUINT16(window_maximized), 0);
        return True;
}
/***
Visibility: public
***/
Bool FFrameWindow::Restore() {
        WmsSendMsg(GetHwnd(), WM_SETWINDOWSTATE, MPFROMUINT16(window_normal), 0);
        return True;
}


//menu
/***
Visibility: public
***/
FMenu *FFrameWindow::GetMenu() {
        if(pMenu)
                return pMenu;
        else
                return pAutoMenu;
}

/***
Visibility: public
***/
void FFrameWindow::SetMenu(FMenu *m) {
        if(pAutoMenu) {
                delete pAutoMenu;
                pAutoMenu=0;
        }
        pMenu = m;
        if(pMenu) {
                WmsSetWindowID(pMenu->GetHandle(),FID_MENU);
                WmsSetWindowParent(pMenu->GetHandle(),GetHwnd());
                WmsSetWindowOwner(pMenu->GetHandle(),GetHwnd());
                pMenu->TurnIntoActionBar();
                RedrawMenu();
        }
}

/***
Visibility: public
***/
FMenu *FFrameWindow::GetSystemMenu() {
        return pSysMenu;
}

/***
Visibility: public
Description: Cause the action bar to reformat/redraw to reflect changes
***/
void FFrameWindow::RedrawMenu() {
        WmsSendMsg(GetHwnd(), WM_UPDATEFRAME, MPFROMUINT32(fcf_menu), 0);
}



//caption/subtitle

/***
Visibility: public
Description:
        Set the caption of the frame window
Return:
        True on success
Note:
        The full title of the frame window consists of a caption and a subtitle
        Usually the caption and subtitle is used for either application+object
        or object+view
***/
Bool FFrameWindow::SetCaption(const char *s) {
        if(s) {
                if(caption) delete[] caption;
                caption=new char[strlen(s)+1];
                if(!caption) return False;
                strcpy(caption,s);
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

/***
Visibility: public
Description:
        retrieve the caption of the frame
Parameters:
        buf     (output) caption
        buflen  sizeo of <buf>
Return:
        length of retrieved string, 0 if none
***/
int FFrameWindow::GetCaption(char *buf, int buflen) {
        if(caption) {
                if(strlen(caption)+1>buflen)
                        return 0;
                strcpy(buf,caption);
                return strlen(buf);
        } else
                return 0;
}

/***
Visibility: public:
Description:
        Set the subtitle of the frame
Parameters:
        s    new subtitle
Return:
        True on success
***/
Bool FFrameWindow::SetSubtitle(const char *s) {
        if(s) {
                if(subtitle) delete[] subtitle;
                subtitle=new char[strlen(s)+1];
                if(!subtitle) return False;
                strcpy(subtitle,s);
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

/***
Visibility: public
Description:
        retrieve the subtitle of the frame
Parameters:
        buf     (output) subtitle
        buflen  size of <buf>
Return:
        length of retrieved string, 0 if none
***/
int FFrameWindow::GetSubtitle(char *buf, int buflen) {
        if(subtitle) {
                if(strlen(subtitle)+1>buflen)
                        return 0;
                strcpy(buf,subtitle);
                return strlen(buf);
        } else
                return 0;
}


/***
Visibility: private
Description:
        concatenate caption+subtitle and set text of titlebar
***/
void FFrameWindow::UpdateTitle() {
        char title[256];
        if(caption)
                if(subtitle)
                        strcat(strcat(strcpy(title,caption)," - "),subtitle);
                else
                        strcpy(title,caption);
        else
                if(subtitle)
                        strcat(strcpy(title,"- "),subtitle);
                else
                        title[0]='\0';
        WmsSendMsg(GetHwnd(), WM_SETTEXT, MPFROMP(title), 0);
}


/***
Visibility: private
Description:
        Hide or show each titlebutton depending on the window state
        Also enable and disable items in the system menu
        The frame is _not_ reformatted
***/
void FFrameWindow::ShowHideTitleButtons(WindowState ws) {
        if(ws==window_normal) {
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_HIDE),       0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_MINIMIZE),   0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_RESTORE),    0, 0,0,0,0, SWP_HIDE);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_MAXIMIZE),   0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_VERTSCROLL), 0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_HORZSCROLL), 0, 0,0,0,0, SWP_SHOW);
                if(pSysMenu) {
                        pSysMenu->EnableItem(SC_HIDE,    True);
                        pSysMenu->EnableItem(SC_MINIMIZE,True);
                        pSysMenu->EnableItem(SC_RESTORE, False);
                        pSysMenu->EnableItem(SC_MAXIMIZE,True);
                }
        } else if(ws==window_maximized) {
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_HIDE),       0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_MINIMIZE),   0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_RESTORE),    0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_MAXIMIZE),   0, 0,0,0,0, SWP_HIDE);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_VERTSCROLL), 0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_HORZSCROLL), 0, 0,0,0,0, SWP_SHOW);
                if(pSysMenu) {
                        pSysMenu->EnableItem(SC_HIDE,    True);
                        pSysMenu->EnableItem(SC_MINIMIZE,True);
                        pSysMenu->EnableItem(SC_RESTORE, True);
                        pSysMenu->EnableItem(SC_MAXIMIZE,False);
                }
        } else if(ws==window_minimized) {
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_HIDE),       0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_MINIMIZE),   0, 0,0,0,0, SWP_HIDE);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_RESTORE),    0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_MAXIMIZE),   0, 0,0,0,0, SWP_SHOW);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_VERTSCROLL), 0, 0,0,0,0, SWP_HIDE);
                WmsSetWindowPos(WmsWindowFromID(GetHwnd(),FID_HORZSCROLL), 0, 0,0,0,0, SWP_HIDE);
                if(pSysMenu) {
                        pSysMenu->EnableItem(SC_HIDE,    True);
                        pSysMenu->EnableItem(SC_MINIMIZE,False);
                        pSysMenu->EnableItem(SC_RESTORE, True);
                        pSysMenu->EnableItem(SC_MAXIMIZE,True);
                }
        }
}

/***
Visibility: private
Description:
        Remember the current position, so it can be restored when the
        window is restored from minimized/maximized
***/
void FFrameWindow::saveRestorePos() {
        if(windowState==window_normal) {
                WmsHWND hwndParent=WmsQueryWindowParent(GetHwnd());
                if(!hwndParent) return;
                WmsQueryWindowRect(GetHwnd(),&restoredRect);
                WmsMapWindowRects(GetHwnd(),hwndParent,&restoredRect,1);
        }
}

void FFrameWindow::calcMinimizedPos(FRect *rect) {
        FRect pr;
        WmsHWND hwndParent=WmsQueryWindowParent(GetHwnd());
        WmsQueryWindowRect(hwndParent,&pr);
        int x,y;
        x=0;
        y = pr.yBottom-1;
        rect->Set(x,y,x+20,y+1);
        for(;;) {
                WmsHWND hwndChild=WmsQueryWindow(hwndParent,QW_FIRSTCHILD);
                Bool noCollision=True;
                while(hwndChild && noCollision) {
                        if(hwndChild!=GetHwnd() && (WindowState)(WmsSendMsgTimeout(hwndChild,WM_QUERYWINDOWSTATE,0,0,100))==window_minimized) {
                                FRect cr;
                                WmsQueryWindowRect(hwndChild,&cr);
                                WmsMapWindowRects(hwndChild,hwndParent,&cr,1);
                                if(cr.Intersects(*rect))
                                        noCollision=False;
                        }
                        hwndChild=WmsQueryWindow(hwndChild,QW_NEXTSIBLING);
                }
                if(noCollision)
                        return;
                x=x+20;
                if(x>=pr.xRight) {
                        x=0;
                        y--;
                }
                rect->Set(x,y,x+20,y+1);
        }
}

static WmsHWND findNextPrevFrame(WmsHWND hwnd, int firstFlag, int nextFlag) {
        //find next/prev frame that is a child of the desktop
        //Note: "next/prev" is not the next/prev window in sibling order but
        //      rather the next/prev window to activate (which usually is the
        //      first/last sibling)
        WmsHWND next;
        next = WmsQueryWindow(hwnd, firstFlag);
        while(next) {
                unsigned fi = (unsigned)WmsSendMsgTimeout(next, WM_QUERYFRAMEINFO, 0,0, 500);
                if((fi&FI_FRAME) && (fi&FI_ACTIVATEOK))
                        return next;
                next = WmsQueryWindow(next, nextFlag);
        }
        return 0;
}

WmsHWND FFrameWindow::findNextFrame() {
        return findNextPrevFrame(GetHwnd(), QW_LASTSIBLING, QW_PREVSIBLING);
}

WmsHWND FFrameWindow::findPrevFrame() {
        return findNextPrevFrame(GetHwnd(), QW_NEXTSIBLING, QW_NEXTSIBLING);
}

WmsHWND FFrameWindow::findNextWindow() {
        //find next frame with the same owner
        WmsHWND next;
        next = WmsQueryWindow(GetHwnd(), QW_LASTSIBLING);
        while(next) {
                if(WmsQueryWindowOwner(next)==WmsQueryWindowOwner(GetHwnd())) {
                        unsigned fi = (unsigned)WmsSendMsgTimeout(next, WM_QUERYFRAMEINFO, 0,0, 500);
                        if((fi&FI_FRAME) && (fi&FI_ACTIVATEOK))
                                return next;
                }
                next = WmsQueryWindow(next, QW_PREVSIBLING);
        }
        return 0;
}

void FFrameWindow::deactivate() {
        if(isActive)
                WmsPostMsg(GetHwnd(), WM_SYSCOMMAND, MPFROMUINT16(SC_PREVFRAME), MPFROM2UINT16(CMDSRC_OTHER,False));
}

Bool FFrameWindow::Paint() {
        WmsHDC hdc = WmsBeginPaint(GetHwnd(),0);
        if(!hdc) return False;

        FRect rect;
        WmsQueryWindowRect(GetHwnd(),&rect);
        if((WmsQueryWindowStyle(GetHwnd())&0x0003) != FS_NOBORDER) {
                char borderchars[8];
                switch(WmsQueryWindowStyle(GetHwnd())&0x0003) {
                        case FS_THINBORDER:
                                WmsQueryPresParam(GetHwnd(), PP_THINBORDERCHARS, 0,0, borderchars,8);
                                break;
                        case FS_SIZEBORDER:
                                WmsQueryPresParam(GetHwnd(), PP_SIZEBORDERCHARS, 0,0, borderchars,8);
                                break;
                        case FS_MODALBORDER:
                                WmsQueryPresParam(GetHwnd(), PP_MODALBORDERCHARS, 0,0, borderchars,8);
                                break;
                }
                uint16 clr;
                if(isActive)
                        WmsQueryPresParam(GetHwnd(), PP_ACTIVEBORDERCOLOR, 0,0, &clr,1);
                else
                        WmsQueryPresParam(GetHwnd(), PP_INACTIVEBORDERCOLOR, 0,0, &clr,1);
                CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yTop), (uint16)(clr<<8|borderchars[0]));
                CpiFillRect(hdc, rect.xLeft+1,rect.yTop,rect.xRight-1,rect.yTop+1, (color)clr, borderchars[1]);
                CpiSetCellData(hdc, FPoint(rect.xRight-1,rect.yTop), (uint16)(clr<<8|borderchars[2]));
                CpiFillRect(hdc, rect.xLeft,rect.yTop+1,rect.xLeft+1,rect.yBottom-1, (color)clr, borderchars[3]);
                CpiFillRect(hdc, rect.xRight-1,rect.yTop+1,rect.xRight,rect.yBottom-1, (color)clr, borderchars[4]);
                CpiSetCellData(hdc, FPoint(rect.xLeft,rect.yBottom-1), (uint16)(clr<<8|borderchars[5]));
                CpiFillRect(hdc, rect.xLeft+1,rect.yBottom-1,rect.xRight-1,rect.yBottom, (color)clr, borderchars[6]);
                CpiSetCellData(hdc, FPoint(rect.xRight-1,rect.yBottom-1), (uint16)(clr<<8|borderchars[7]));

                rect.Shrink(1,1);
        }

        uint8 clr;
        WmsQueryPresParam(GetHwnd(), PP_DIALOGBACKGROUNDCOLOR, 0,0, &clr,1);
        CpiEraseRect(hdc, rect, (color)clr);

        WmsReleaseDC(hdc);
        return True;
}


Bool FFrameWindow::ControlPointer(FControlPointerEvent &ev) {
        if((WmsQueryWindowStyle(GetHwnd())&0x0003)!=FS_SIZEBORDER) {
                ev.SetPointer(*GetSystemArrowPointer());
                return True;
        }
        if(windowState==window_minimized) {
                ev.SetPointer(*GetSystemArrowPointer());
                return True;
        }

        FPoint p=ev.pt;
        WmsMapWindowPoints(GetDesktop()->GetHwnd(),GetHwnd(),&p,1);
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        if(p.GetX()==r.GetLeft()) {
                if(p.GetY()==r.GetTop())
                        ev.SetPointer(*GetSystemNWSEPointer());
                else if(p.GetY()==r.GetBottom()-1)
                        ev.SetPointer(*GetSystemNESWPointer());
                else
                        ev.SetPointer(*GetSystemWEPointer());
        } else if(p.GetX()==r.GetRight()-1) {
                if(p.GetY()==r.GetTop())
                        ev.SetPointer(*GetSystemNESWPointer());
                else if(p.GetY()==r.GetBottom()-1)
                        ev.SetPointer(*GetSystemNWSEPointer());
                else
                        ev.SetPointer(*GetSystemWEPointer());
        } else {
                if(p.GetY()==r.GetTop())
                        ev.SetPointer(*GetSystemNSPointer());
                else if(p.GetY()==r.GetBottom()-1)
                        ev.SetPointer(*GetSystemNSPointer());
                else
                        return False;
        }
        return True;
}
        
Bool FFrameWindow::Button1Down(FButtonDownEvent &ev) {
        if((WmsQueryWindowStyle(GetHwnd())&0x0003)!=FS_SIZEBORDER)
                return False;
        //Start tracking the frame
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        uint32 flTrack=TF_SIZE;
        if(ev.GetX()==r.GetLeft()) {
                if(ev.GetY()==r.GetTop())
                        flTrack |= TF_LEFT|TF_TOP;
                else if(ev.GetY()==r.GetBottom()-1)
                        flTrack |= TF_LEFT|TF_BOTTOM;
                else
                        flTrack |= TF_LEFT;
        } else if(ev.GetX()==r.GetRight()-1) {
                if(ev.GetY()==r.GetTop())
                        flTrack |= TF_RIGHT|TF_TOP;
                else if(ev.GetY()==r.GetBottom()-1)
                        flTrack |= TF_RIGHT|TF_BOTTOM;
                else
                        flTrack |= TF_RIGHT;
        } else {
                if(ev.GetY()==r.GetTop())
                        flTrack |= TF_TOP;
                else if(ev.GetY()==r.GetBottom()-1)
                        flTrack |= TF_BOTTOM;
                else
                        return False;
        }
        if(ev.IsCtrlDown())
                WmsSendMsg(GetHwnd(), WM_TRACKFRAME, MPFROMUINT32(flTrack), 0);
        else
                WmsSendMsg(GetHwnd(), WM_TRACKFRAME, MPFROMUINT32(flTrack), MPFROMUINT32(QSR_NOCLIPSIBLINGSABOVE));
        return False; //we don't mind others getting this
}
             
Bool FFrameWindow::MouseActivity(FMouseEvent &ev) {
        if(!ev.IsCtrlDown()) {
                WmsHWND hwnd_desktop = GetDesktop()->GetHwnd();
                WmsHWND hwnd_currentfocus = WmsQueryFocus(hwnd_desktop);
                if(hwnd_currentfocus!=GetHwnd() && !WmsIsChild(GetHwnd(),hwnd_currentfocus))
                        WmsChangeFocus(hwnd_desktop, GetHwnd(), 0);
        }
        return False;
}

Bool FFrameWindow::TrackFrame(uint32 tfl, uint32 dfl) {
        if((WmsQueryWindowStyle(GetHwnd())&0x0003)!=FS_SIZEBORDER && tfl&TF_SIZE)
                return False;
        TrackInfo ti;
        Bool rc=(Bool)WmsSendMsg(GetHwnd(),WM_QUERYTRACKINFO, MPFROMUINT32(tfl), MPFROMP(&ti));
        if(rc) {
                //remove TF_SIZE if the window is minimized or does not have a sizing border
                if(windowState==window_minimized ||
                   (WmsQueryWindowStyle(GetHwnd())&3)!=FS_SIZEBORDER)
                        ti.fl &= ~TF_SIZE;
                rc = TrackRect(this, QSR_PARENTCLIP|QSR_NOCLIPCHILDREN|dfl, &ti);
                if(rc) {
                        WmsMapWindowRects(GetHwnd(), WmsQueryWindowParent(GetHwnd()), &ti.track, 1);
                        WmsSetWindowPos(GetHwnd(),
                                        0,
                                        ti.track.GetLeft(), ti.track.GetTop(),
                                        ti.track.GetWidth(), ti.track.GetHeight(),
                                        SWP_SIZE|SWP_MOVE
                                       );
                }
        }
        return True;
}

Bool FFrameWindow::QueryTrackInfo(uint32 fl, void *p) {
        //return default tracking info
        TrackInfo *ti = (TrackInfo*)p;
        WmsQueryWindowRect(GetHwnd() ,&ti->track);
        WmsQueryWindowRect(WmsQueryWindowParent(GetHwnd()), &ti->bounding);
        WmsMapWindowRects(WmsQueryWindowParent(GetHwnd()), GetHwnd(), &ti->bounding,1);
        ti->minWidth = 3;
        ti->maxWidth = ti->bounding.GetWidth() * 2;
        ti->minHeight = windowState==window_minimized ? 1 : 3;
        ti->maxHeight = windowState==window_minimized ? 1 : ti->bounding.GetHeight() * 2;
        ti->fl = fl;
        if(windowState==window_minimized) ti->fl|=TF_ALLINBOUNDARY;
        return True;
}

//System accelerator table:
ACCELTABLE(0xFFFE)
  ACCELKEY(SC_CLOSE,      SYSCOMMAND,lk_f4,   UP,UP,DOWN)
  ACCELKEY(SC_RESTORE,    SYSCOMMAND,lk_f5,   UP,UP,DOWN)
  ACCELKEY(SC_NEXTWINDOW, SYSCOMMAND,lk_f6,   UP,UP,DOWN)
  ACCELKEY(SC_MOVE,       SYSCOMMAND,lk_f7,   UP,UP,DOWN)
  ACCELKEY(SC_SIZE,       SYSCOMMAND,lk_f8,   UP,UP,DOWN)
  ACCELKEY(SC_MINIMIZE,   SYSCOMMAND,lk_f9,   UP,UP,DOWN)
  ACCELKEY(SC_MAXIMIZE,   SYSCOMMAND,lk_f10,  UP,UP,DOWN)
  ACCELKEY(SC_APPMENU,    SYSCOMMAND,lk_f10,  UP,UP,UP)
  ACCELKEY(SC_HIDE,       SYSCOMMAND,lk_f11,  UP,UP,DOWN)
  ACCELKEY(SC_SIZEMOVE,   SYSCOMMAND,lk_f12,  UP,UP,DOWN)
  ACCELKEY(SC_SYSMENU,    SYSCOMMAND,lk_esc,  DOWN,UP,UP)
  ACCELKEY(SC_TASKLIST,   SYSCOMMAND,lk_esc,  UP,DOWN,UP)
  ACCELKEY(SC_NEXTFRAME,  SYSCOMMAND,lk_esc,  UP,UP,DOWN)
//ACCELKEY(SC_SYSMENU,    SYSCOMMAND,lk_space,UP,UP,DOWN) //WMS does have a lk_space
  ACCELKEY(SC_SYSMENU,    SYSCOMMAND,lk_f10,  DOWN,UP,DOWN)
//needed to support system menu:
  ACCELKEY(SC_LEFTKEY,    SYSCOMMAND,lk_left, UP,UP,UP)
  ACCELKEY(SC_RIGHTKEY,   SYSCOMMAND,lk_right,UP,UP,UP)
ENDACCELTABLE

//System menu definition:
MENU(0xFFFE)
  MENUITEM(SC_RESTORE,   "text=\"@Restore\tAlt+F5\" syscommand")
  MENUITEM(SC_MOVE,      "text=\"@Move\tAlt+F7\" syscommand")
  MENUITEM(SC_SIZE,      "text=\"@Size\tAlt+F8\" syscommand")
  MENUITEM(SC_MINIMIZE,  "text=\"Mi@nimize\tAlt+F9\" syscommand")
  MENUITEM(SC_MAXIMIZE,  "text=\"Ma@ximize\tAlt+F10\" syscommand")
  MENUITEM(SC_HIDE,      "text=\"@Hide\tAlt+F11\" syscommand")
  MENUITEM(-1,"separator")
  MENUITEM(SC_CLOSE,     "text=\"@Close\tAlt+F4\" syscommand")
  MENUITEM(-2,"separator")
  MENUITEM(SC_TASKLIST,  "text=\"Window @list\tCtrl+Esc\" syscommand")
  MENUITEM(SC_NEXTWINDOW,"text=\"Next window\tAlt+F6\" syscommand")
  MENUITEM(SC_NEXTFRAME, "text=\"Next frame\tAlt+Esc\" syscommand")
  MENUITEM(SC_PREVFRAME, "text=\"Previous frame\" syscommand")
ENDMENU

Bool FFrameWindow::SysCommand(FSysCommandEvent &ev) {
        switch(ev.GetItem()) {
                case SC_SIZE:
                        WmsPostMsg(GetHwnd(), WM_TRACKFRAME, MPFROMUINT32(TF_SIZE), 0);
                        return True;
                case SC_MOVE:
                        WmsPostMsg(GetHwnd(), WM_TRACKFRAME, MPFROMUINT32(TF_MOVE), 0);
                        return True;
                case SC_SIZEMOVE:
                        WmsPostMsg(GetHwnd(), WM_TRACKFRAME, MPFROMUINT32(TF_SIZE|TF_MOVE), 0);
                        return True;
                case SC_MINIMIZE:
                        if(WmsQueryWindowStyle(WmsWindowFromID(GetHwnd(),FID_MINIMIZE))&WS_VISIBLE)
                                WmsPostMsg(GetHwnd(), WM_SETWINDOWSTATE, MPFROMUINT16(window_minimized), 0);
                        return True;
                case SC_MAXIMIZE:
                        if(WmsQueryWindowStyle(WmsWindowFromID(GetHwnd(),FID_MAXIMIZE))&WS_VISIBLE)
                                WmsPostMsg(GetHwnd(), WM_SETWINDOWSTATE, MPFROMUINT16(window_maximized), 0);
                        return True;
                case SC_HIDE:
                        Hide();
                        return True;
                case SC_RESTORE:
                        if(WmsQueryWindowStyle(WmsWindowFromID(GetHwnd(),FID_RESTORE))&WS_VISIBLE)
                                WmsPostMsg(GetHwnd(), WM_SETWINDOWSTATE, MPFROMUINT16(window_normal), 0);
                        return True;
                case SC_CLOSE:
                        WmsPostMsg(GetHwnd(), WM_CLOSE, 0,0);
                        return True;
                case SC_APPMENU: {
                        if(pMenu)
                                pMenu->EnterActionBar();
                        else if(pAutoMenu)
                                pAutoMenu->EnterActionBar();
                        return True;
                }
                case SC_SYSMENU: {
                        if(pSysMenu) {
                                FPoint p(1,1);
                                WmsMapWindowPoints(GetHwnd(), GetDesktop()->GetHwnd(), &p,1);
                                pSysMenu->Popup(GetDesktop(),this,&p);
                        }
                        return True;
                }
                case SC_NEXTFRAME:
                case SC_PREVFRAME: {
                        WmsHWND hwndParent=WmsQueryWindowParent(GetHwnd());
                        if(hwndParent!=GetDesktop()->GetHwnd()) {
                                //If we are not a child of the desktop, we let
                                //the event bubble up to our parent
                                WmsPostMsg(hwndParent, ev.msg,ev.mp1,ev.mp2);
                        } else {
                                WmsHWND otherFrame;
                                if(ev.GetItem()==SC_NEXTFRAME)
                                        otherFrame=findNextFrame();
                                else
                                        otherFrame=findPrevFrame();
                                if(otherFrame && otherFrame!=GetHwnd()) {
                                        WmsChangeFocus(hwndParent, otherFrame, 0);
                                }
                        }
                        return True;
                }
                case SC_NEXTWINDOW: {
                        WmsHWND nextWindow=findNextWindow();
                        if(nextWindow && nextWindow!=GetHwnd())
                                WmsChangeFocus(WmsQueryWindowParent(nextWindow), nextWindow, 0);
                        return True;
                }
                case SC_TASKLIST: {
                        //tell the desktop to show the switchlist
                        WmsPostMsg(GetDesktop()->GetHwnd(), WM_SHOWSWITCHLIST, MPFROMUINT16(True), 0);
                        return True;
                }
                case SC_LEFTKEY:
                case SC_RIGHTKEY: {
                        //With an MDI interface we have to do this right
                        if(pSysMenu &&
                           WmsQueryFocus(GetDesktop()->GetHwnd())==pSysMenu->GetHandle())
                        {
                                if(pMenu)
                                        pMenu->EnterActionBar();
                                else if(pAutoMenu)
                                        pAutoMenu->EnterActionBar();
                        }
                        return True;
                }
                default:
                        return True;
        }
}

uint32 FFrameWindow::QueryWindowState() {
        return (uint32)windowState | (((uint32)nextWindowState)<<16);
}

Bool FFrameWindow::SetWindowState(unsigned s) {
        if(s!=window_minimized && s!=window_normal && s!=window_maximized) return False;
        if(s==windowState) return True;
        WindowState oldState=windowState;
        FRect newRect;
        switch(s) {
                case window_minimized: {
                        WmsHWND hwndParent=WmsQueryWindowParent(GetHwnd());
                        if(!hwndParent) 
                                return False; //cannot minimize orphan window
                        saveRestorePos();
                        calcMinimizedPos(&newRect);
                        deactivate();
                        break;
                }
                case window_normal: {
                        newRect = restoredRect;
                        break;
                }
                case window_maximized: {
                        WmsHWND hwndParent=WmsQueryWindowParent(GetHwnd());
                        if(!hwndParent) 
                                return False; //cannot maximize orphan window
                        saveRestorePos();
                        WmsQueryWindowRect(hwndParent,&newRect);
                        break;
                }
        }
        nextWindowState = (WindowState)s;
        if(oldState!=window_minimized) windowState = (WindowState)s;
        //notify owned frames of our state change
        WmsHWND hwndOwnee=WmsQueryWindow(GetHwnd(),QW_FIRSTOWNEE);
        while(hwndOwnee) {
                if(WmsQueryWindowParent(hwndOwnee)!=GetHwnd())
                        WmsSendMsgTimeout(hwndOwnee, WM_OWNERSTATECHANGING, MPFROM2UINT16(oldState,windowState), 0, 500);
                hwndOwnee=WmsQueryWindow(hwndOwnee,QW_NEXTOWNEE);
        }
        ShowHideTitleButtons((WindowState)s);
        WmsSetWindowPos(GetHwnd(),
                        0,
                        newRect.xLeft, newRect.yTop,
                        newRect.GetWidth(), newRect.GetHeight(),
                        SWP_SIZE|SWP_MOVE
                       );
        if(oldState==window_minimized) windowState = (WindowState)s;
        //update frame
        WmsSendMsg(GetHwnd(), WM_UPDATEFRAME, MPFROMUINT32(fcf_maxbutton), 0);
        return True;
}

Bool FFrameWindow::QueryActive() {
        return isActive;
}

Bool FFrameWindow::SetActive(Bool f) {
        if(isActive!=f) {
                isActive=f;
                //invalidate frame and frame controls
                WmsInvalidateRect(GetHwnd(), 0, False);
                WmsInvalidateRect(WmsWindowFromID(GetHwnd(),FID_SYSMENU), 0, False);
                WmsInvalidateRect(WmsWindowFromID(GetHwnd(),FID_TITLEBAR), 0, False);
                WmsInvalidateRect(WmsWindowFromID(GetHwnd(),FID_HIDE), 0, False);
                WmsInvalidateRect(WmsWindowFromID(GetHwnd(),FID_RESTORE), 0, False);
                WmsInvalidateRect(WmsWindowFromID(GetHwnd(),FID_MINIMIZE), 0, False);
                WmsInvalidateRect(WmsWindowFromID(GetHwnd(),FID_MAXIMIZE), 0, False);
        }
        return True;
}

//frame formatting
Bool FFrameWindow::CalcFrameRect(FRect *r, Bool frameRectProvided) {
        int fleft,fright,ftop,fbottom;
        if(WmsQueryWindowStyle(GetHwnd())&0x03) {
                //some sort of border
                fleft=1,fright=1,ftop=1,fbottom=1;
        } else {
                fleft=0;
                if(WmsWindowFromID(GetHwnd(),FID_SYSMENU) ||
                   WmsWindowFromID(GetHwnd(),FID_TITLEBAR) ||
                   WmsWindowFromID(GetHwnd(),FID_HIDE) ||
                   WmsWindowFromID(GetHwnd(),FID_RESTORE) ||
                   WmsWindowFromID(GetHwnd(),FID_MINIMIZE) ||
                   WmsWindowFromID(GetHwnd(),FID_MAXIMIZE))
                        ftop=1;
                else
                        ftop=0;
                if(WmsWindowFromID(GetHwnd(),FID_VERTSCROLL))
                        fright=1;
                else
                        fright=0;
                if(WmsWindowFromID(GetHwnd(),FID_HORZSCROLL))
                        fbottom=1;
                else
                        fbottom=0;
        }
        WmsHWND hwndMenu=WmsWindowFromID(GetHwnd(),FID_MENU);
        if(hwndMenu) {
                //calc vertical space taken by the menu
                WmsSWP swp;
                swp.flags = SWP_SIZE;
                swp.x=0;
                swp.y=0;
                swp.cx = frameRectProvided?r->GetWidth()-fleft-fright:r->GetWidth();
                swp.cy=0;
                swp.hwndBehind=0;
                swp.hwnd = hwndMenu;
                WmsSendMsg(hwndMenu,WM_ADJUSTWINDOWPOS,MPFROMP(&swp),MPFROMUINT32(swp.flags));
                ftop += swp.cy;
        }
        if(frameRectProvided) {
                r->xLeft +=fleft;
                r->xRight-=fright;
                r->yTop+=ftop;
                r->yBottom-=fbottom;
        } else {
                r->xLeft-=fleft;
                r->xRight+=fright;
                r->yTop-=ftop;
                r->yBottom+=fbottom;
        }
        return True;
}

Bool FFrameWindow::UpdateFrame(uint32 /*fcf*/) {
        unsigned swps = (unsigned)WmsSendMsg(GetHwnd(), WM_QUERYFRAMECTLCOUNT, 0,0);
        WmsSWP *pswp = new WmsSWP[swps];
        if(!pswp) return False;
        for(unsigned i=0; i<swps; i++) {
                pswp[i].hwnd=0;
                pswp[i].flags=0;
        }
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        Bool rc = (Bool)WmsSendMsg(GetHwnd(), WM_FORMATFRAME, MPFROMP(pswp), MPFROMP(&r));
        if(rc) {
                WmsSetMultWindowPos(pswp,swps);
                delete[] pswp;
                return True;
        } else {
                delete[] pswp;
                return False;
        }
}

unsigned FFrameWindow::QueryFrameControlCount() {
        unsigned c=0;
        if(WmsWindowFromID(GetHwnd(),FID_SYSMENU)) c++;
        if(WmsWindowFromID(GetHwnd(),FID_TITLEBAR)) c++;
        if(WmsWindowFromID(GetHwnd(),FID_MINIMIZE)) c++;
        if(WmsWindowFromID(GetHwnd(),FID_MAXIMIZE)) c++;
        if(WmsWindowFromID(GetHwnd(),FID_RESTORE)) c++;
        if(WmsWindowFromID(GetHwnd(),FID_HIDE)) c++;
        if(WmsWindowFromID(GetHwnd(),FID_VERTSCROLL)) c++;
        if(WmsWindowFromID(GetHwnd(),FID_HORZSCROLL)) c++;
        if(WmsWindowFromID(GetHwnd(),FID_MENU)) c++;

        return c;
}

Bool FFrameWindow::FormatFrame(WmsSWP *pswp, FRect *client) {
        FRect rect;
        WmsQueryWindowRect(GetHwnd(),&rect);

        *client = rect;
        if((WmsQueryWindowStyle(GetHwnd())&0x3)!=FS_NOBORDER) {
                client->Shrink(1,1);
        }

        unsigned l=rect.xLeft;
        if(windowState!=window_minimized) l += 2;

        WmsSWP *p=pswp;
        Bool titleBarLineUsed=False;
        WmsHWND h;
        h=WmsWindowFromID(GetHwnd(),FID_SYSMENU);
        if(h) {
                p->hwnd = h;
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->x = l;
                        p->y = rect.yTop;
                        p->cx = 3;
                        p->cy = 1;
                        l += 3;
                        p->flags = SWP_SIZE|SWP_MOVE;
                        titleBarLineUsed=True;
                } else
                        p->flags =0;
                p++;
        }

        Bool rightButtonsFound=False;
        unsigned r=rect.xRight;
        if(windowState!=window_minimized) r -= 2;
        h=WmsWindowFromID(GetHwnd(),FID_MAXIMIZE);
        if(h) {
                p->hwnd = h;
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->x = r-3;
                        p->y = rect.yTop;
                        p->cx = 3;
                        p->cy = 1;
                        p->flags =SWP_SIZE|SWP_MOVE|SWP_SHOW;
                        r-=3;
                        titleBarLineUsed=True;
                        rightButtonsFound=True;
                } else
                        p->flags=0;
                p++;
        }

        h=WmsWindowFromID(GetHwnd(),FID_RESTORE);
        if(h) {
                p->hwnd = h;
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->x = r-3;
                        p->y = rect.yTop;
                        p->cx = 3;
                        p->cy = 1;
                        p->flags =SWP_SIZE|SWP_MOVE|SWP_SHOW;
                        r-=3;
                        titleBarLineUsed=True;
                        rightButtonsFound=True;
                } else
                        p->flags=0;
                p++;
        }

        h=WmsWindowFromID(GetHwnd(),FID_MINIMIZE);
        if(h) {
                p->hwnd = h;
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->x = r-3;
                        p->y = rect.yTop;
                        p->cx = 3;
                        p->cy = 1;
                        p->flags =SWP_SIZE|SWP_MOVE|SWP_SHOW;
                        r-=3;
                        titleBarLineUsed=True;
                        rightButtonsFound=True;
                } else
                        p->flags=0;
                p++;
        }

        h=WmsWindowFromID(GetHwnd(),FID_HIDE);
        if(h) {
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->hwnd = h;
                        p->x = r-3;
                        p->y = rect.yTop;
                        p->cx = 3;
                        p->cy = 1;
                        p->flags =SWP_SIZE|SWP_MOVE|SWP_SHOW;
                        r-=3;
                        titleBarLineUsed=True;
                        rightButtonsFound=True;
                } else
                        p->flags=0;
                p++;
        }

        h=WmsWindowFromID(GetHwnd(),FID_TITLEBAR);
        if(h) {
                p->hwnd = h;
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->x = l;
                        p->y = rect.yTop;
                        p->cx = r-l;
                        p->cy = 1;
                        if(windowState!=window_minimized) {
                                p->x++;
                                p->cx--;
                                if(rightButtonsFound) p->cx--;
                        }
                        if(p->cx<0) p->cx=0;
                        p->flags =SWP_SIZE|SWP_MOVE;
                        titleBarLineUsed=True;
                } else
                        p->flags = 0;
                p++;
        }


        h=WmsWindowFromID(GetHwnd(),FID_MENU);
        if(h) {
                p->hwnd = h;
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->x = 1;
                        p->cx = rect.GetWidth()-2;
                        p->y = titleBarLineUsed?1:0;
                        p->cy = 1;
                        p->flags = SWP_SIZE|SWP_MOVE|SWP_SHOW;
                        //since the menu may need several lines let it adjust its size beforehand
                        WmsSendMsg(h, WM_ADJUSTWINDOWPOS, MPFROMP(p), MPFROMUINT32(p->flags));
                        client->yTop = p->y+p->cy;
                } else
                        p->flags=0;
                p++;
        }

        h=WmsWindowFromID(GetHwnd(),FID_HORZSCROLL);
        if(h) {
                p->hwnd = h;
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->cy = 1;
                        p->y = rect.yBottom-1;
                        p->x = rect.xLeft+1;
                        p->cx = rect.GetWidth()-1;
                        if(windowState==window_maximized) {
                                if(WmsQueryWindowStyle(WmsWindowFromID(GetHwnd(),FID_VERTSCROLL))&WS_VISIBLE)
                                        p->cx -= 1;
                        } else
                                p->cx -= 2;
                        p->flags =SWP_SIZE|SWP_MOVE;
                } else
                        p->flags = 0;
                p++;
        }

        h=WmsWindowFromID(GetHwnd(),FID_VERTSCROLL);
        if(h) {
                p->hwnd = h;
                if(WmsQueryWindowStyle(h)&WS_VISIBLE) {
                        p->y = rect.yTop+1;
                        p->cy = rect.GetHeight()-1;
                        p->x = rect.xRight-1;
                        p->cx = 1;
                        if(windowState==window_maximized) {
                                if(WmsQueryWindowStyle(WmsWindowFromID(GetHwnd(),FID_HORZSCROLL))&WS_VISIBLE)
                                        p->cy -= 1;
                        } else
                                p->cy -= 2;
                        p->flags =SWP_SIZE|SWP_MOVE;
                } else
                        p->flags = 0;
                p++;
        }

        return True;
}


Bool FFrameWindow::AdjustWindowPos(WmsSWP *swp) {
        WmsHWND hwndParent=WmsQueryWindowParent(GetHwnd());
        WmsHWND hwndOwnee;
        //count ownees
        unsigned ownedFrames=0;
        hwndOwnee=WmsQueryWindow(GetHwnd(), QW_FIRSTOWNEE);
        while(hwndOwnee) {
                if(WmsQueryWindowParent(hwndOwnee)==hwndParent)
                        ownedFrames++;
                hwndOwnee = WmsQueryWindow(hwndOwnee, QW_NEXTOWNEE);
        }
        //allocate owneeSWP array
        owneePSWP = new WmsSWP[ownedFrames];
        owneeSWPS=0;

        //let each owned frame specify its new position
        WmsSWP *pswp=owneePSWP;
        hwndOwnee=WmsQueryWindow(GetHwnd(), QW_FIRSTOWNEE);
        while(hwndOwnee) {
                if(WmsQueryWindowParent(hwndOwnee)==hwndParent) {
                        pswp->hwnd = hwndOwnee;
                        pswp->flags = 0;
                        WmsSendMsg(hwndOwnee, WM_OWNERPOSCHANGING, MPFROMP(pswp), MPFROMP(swp));
                        if(pswp->flags!=0) {
                                pswp++;
                                owneeSWPS++;
                        }
                }
                hwndOwnee = WmsQueryWindow(hwndOwnee, QW_NEXTOWNEE);
        }

        if(swp->flags&SWP_HIDE) {
                deactivate();
        }
        return False;
}

Bool FFrameWindow::WindowPosChanged(WmsSWP *swp) {
        if(swp->flags&SWP_SIZE)
                WmsSendMsg(GetHwnd(), WM_UPDATEFRAME, MPFROMUINT32(0xFFFFFFFF), 0);

        if(owneePSWP) {
                WmsSetMultWindowPos(owneePSWP, owneeSWPS);
                delete[] owneePSWP;
                owneePSWP=0;
        }
        return False;
}

Bool FFrameWindow::OwnerPosChanging(WmsSWP *swp, WmsSWP *ownerSWP) {
        if(ownerSWP->flags&SWP_HIDE && (WmsQueryWindowStyle(GetHwnd())&FS_NOHIDEWITHOWNER)==0) {
                //owner is being hidden
                if(WmsQueryWindowStyle(GetHwnd())&WS_VISIBLE) {
                        hiddenDueToOwner=True;
                        swp->flags |= SWP_HIDE;
                }
        }
        if(ownerSWP->flags&SWP_SHOW && (WmsQueryWindowStyle(GetHwnd())&FS_NOHIDEWITHOWNER)==0) {
                //owner is being shown
                if(hiddenDueToOwner) {
                        swp->flags |= SWP_SHOW;
                        hiddenDueToOwner=False;
                }
        }
        if(ownerSWP->flags&SWP_MOVE && (WmsQueryWindowStyle(GetHwnd())&FS_NOMOVEWITHOWNER)==0) {
                //we should specify to move relative to the topleft edge of the owner
                if((WindowState)WmsSendMsg(ownerSWP->hwnd,WM_QUERYWINDOWSTATE,0,0) != window_minimized) {
                        FPoint p(0,0); WmsMapWindowPoints(GetHwnd(), ownerSWP->hwnd, &p, 1);
                        swp->x = ownerSWP->x + p.x;
                        swp->y = ownerSWP->y + p.y;
                        swp->flags |= SWP_MOVE;
                }
        }
        return True; //handled
}

Bool FFrameWindow::OwnerStateChanging(unsigned oldState, unsigned newState) {
        if((WmsQueryWindowStyle(GetHwnd())&FS_NOHIDEWITHOWNER))
                return True; //owner state has no effect on us
        if(oldState==window_minimized) {
                //owner is being maximized/restored
                if(hiddenDueToOwner) {
                        //we have hidden our selves previously
                        WmsSetWindowPos(GetHwnd(), 0, 0,0,0,0, SWP_SHOW);
                        hiddenDueToOwner=False;
                }
        } else if(newState==window_minimized) {
                //owner is being minimized
                if(WmsQueryWindowStyle(GetHwnd())&WS_VISIBLE) {
                        hiddenDueToOwner=True;
                        WmsSetWindowPos(GetHwnd(), 0, 0,0,0,0, SWP_HIDE);
                }
        }
        return True; //handled
}

Bool FFrameWindow::FocusChanging(WmsHWND otherHwnd, Bool f, uint16 fl) {
        WmsHWND hwndFocus=WmsQueryFocus(GetDesktop()->GetHwnd());
        if(f) {
                //gaining focus
                if(hwndFocus==GetHwnd() || WmsIsChild(GetHwnd(),hwndFocus)) {
                        if((fl&FC_NOACTIVATE)==0)
                                WmsSendMsg(GetHwnd(), WM_SETACTIVE, MPFROMUINT16(True), 0);
                        if((fl&FC_NOBRINGTOTOP)==0)
                                WmsSetWindowPos(GetHwnd(), HWND_TOP, 0,0, 0,0, SWP_ZORDER);
                } else {
                        //non-child ownee is getting focus (probably a popup menu, owned frame or something else
                }
        } else {
                //losing focus
                if(otherHwnd!=GetHwnd() &&
                   (!WmsIsChild(GetHwnd(),otherHwnd))) {
                        //losing focus to non-child
                        if((fl&FC_NODEACTIVATE)==0)
                                WmsSendMsg(GetHwnd(), WM_SETACTIVE, MPFROMUINT16(False), 0);
                }
        }
        return False;
}

unsigned FFrameWindow::QueryFrameInfo() {
        unsigned fl= FI_FRAME;
        if(WmsQueryWindowStyle(GetHwnd())&FS_NOMOVEWITHOWNER)
                fl |= FI_NOMOVEWITHOWNER;
        if((WmsQueryWindowStyle(GetHwnd())&FS_NOHIDEWITHOWNER)==0)
                fl |= FI_OWNERHIDE;
        if(WmsIsWindowEnabled(GetHwnd()) && WmsIsWindowVisible(GetHwnd()))
                fl |= FI_ACTIVATEOK;
        return fl;
}

void FFrameWindow::TextChanging(const char *pszText) {
        //update the switchlist
        FSwitchList::SwitchEntry e;
        if(GetSwitchList()->QueryEntry(this, &e)) {
                strncpy(e.title, pszText, sizeof(e.title));
                e.title[sizeof(e.title)-1] = '\0';
                GetSwitchList()->ChangeEntry(&e);
        }
}

WmsHWND FFrameWindow::NextMenu(WmsHWND hwndMenu, Bool /*prev*/) {
        if((pAutoMenu && pAutoMenu->GetHandle()==hwndMenu) ||
           (pMenu     && pMenu->GetHandle()==hwndMenu)) {
                if(pSysMenu)
                        return pSysMenu->GetHandle();
                else
                        return 0;
        } else if(pSysMenu && pSysMenu->GetHandle()==hwndMenu) {
                if(pMenu)
                        return pMenu->GetHandle();
                if(pAutoMenu)
                        return pAutoMenu->GetHandle();
                return 0;
        } else
                return 0;
}

