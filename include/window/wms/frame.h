#ifndef __FRAME_H_INCLUDED
#define __FRAME_H_INCLUDED

#ifndef __CLHNDL_H_INCLUDED
#  include <window\wms\clhndl.h>
#endif
#ifndef __KEYBOARD_H_INCLUDED
#  include <window\wms\keyboard.h>
#endif
#ifndef __MOUSE_H_INCLUDED
#  include <window\wms\mouse.h>
#endif
#ifndef __COMHNDL_H_INCLUDED
#  include <window\wms\comhndl.h>
#endif

//forward declarations of frame-related classes
class FFrameWindow;
class FTitleBar;
class FScrollBar;
class FTitleBarButton;

//frame styles
#define FS_NOBORDER             0x0000
#define FS_THINBORDER           0x0001
#define FS_SIZEBORDER           0x0002
#define FS_MODALBORDER          0x0003
#define FS_NOHIDEWITHOWNER      0x0004
#define FS_NOMOVEWITHOWNER      0x0008

//frame controls IDs
#define FID_SYSMENU             0x8000
#define FID_TITLEBAR            0x8001
#define FID_MINIMIZE            0x8002
#define FID_MAXIMIZE            0x8003
#define FID_RESTORE             0x8004
#define FID_HIDE                0x8005
#define FID_VERTSCROLL          0x8006
#define FID_HORZSCROLL          0x8007
#define FID_MENU                0x8008
#define FID_CLIENT              0x8009

//system commands
#define SC_SIZE                 0x8001
#define SC_MOVE                 0x8002
#define SC_SIZEMOVE             0x8003
#define SC_MINIMIZE             0x8004
#define SC_MAXIMIZE             0x8005
#define SC_HIDE                 0x8006
#define SC_RESTORE              0x8007
#define SC_CLOSE                0x8008
#define SC_APPMENU              0x8009
#define SC_SYSMENU              0x800a
#define SC_NEXTFRAME            0x800b
#define SC_PREVFRAME            0x800c
#define SC_NEXTWINDOW           0x800d
#define SC_TASKLIST             0x800e

//response flags to wm_queryframeinfo
#define FI_FRAME                0x0001
#define FI_OWNERHIDE            0x0002
#define FI_NOMOVEWITHOWNER      0x0004
#define FI_ACTIVATEOK           0x0008


class BIFCLASS FFrameAccelHandler : public FHandler {
        friend class FFrameWindow;
        FAcceleratorTable *accel;
        FAcceleratorTable *sysaccel;
public:
        FFrameAccelHandler(FWnd *pwnd);
        Bool Dispatch(FEvent &ev);
};

class BIFCLASS FFrameHandler : public FHandler {
protected:
        FFrameHandler(FFrameWindow *pwnd);
        Bool Dispatch(FEvent &ev);
        void Invalidate(Bool includeChildren);
        virtual Bool Paint() =0;
        virtual Bool TrackFrame(uint32 tfl, uint32 dfl) =0;
        virtual Bool QueryTrackInfo(uint32 fl, void *p) =0;
        virtual uint32 QueryWindowState() =0;
        virtual Bool SetWindowState(unsigned s) =0;
        virtual Bool QueryActive() =0;
        virtual Bool SetActive(Bool f) =0;
        virtual Bool CalcFrameRect(FRect *r, Bool frameRectProvided) =0;
        virtual Bool UpdateFrame(uint32 fcf) =0;
        virtual unsigned QueryFrameControlCount() =0;
        virtual Bool FormatFrame(WmsSWP *pswp, FRect *client) =0;
        virtual Bool AdjustWindowPos(WmsSWP *swp) =0;
        virtual Bool WindowPosChanged(WmsSWP *swp) =0;
        virtual Bool OwnerPosChanging(WmsSWP *swp, WmsSWP *ownerSWP) =0;
        virtual Bool OwnerStateChanging(unsigned oldState, unsigned NewState) =0;
        virtual Bool MouseActivity(FMouseEvent &) =0;
        virtual Bool FocusChanging(WmsHWND otherHwnd, Bool f, uint16 fl) =0;
        virtual unsigned QueryFrameInfo() =0;
        virtual void TextChanging(const char *pszText) =0;
        virtual WmsHWND NextMenu(WmsHWND hwndMenu, Bool prev) =0;
};

class BIFCLASS FFrameWindow : public FWnd,
                              protected FFrameHandler,
                              public FMouseHandler,
                              public FFrameAccelHandler,
                              public FCommandHandler
{
public:
        FFrameWindow();
        ~FFrameWindow();

public:
        // Creation
        Bool Create(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect = 0 );
        enum {  //Auto-load flags
                alf_none         =0x0000,
                alf_icon         =0x0001,
                alf_menu         =0x0002,
                alf_accelerator  =0x0004,
                alf_caption      =0x0008
        };
        enum {  //frame creation flags
                fcf_none          =0x0000,
                fcf_sysmenu       =0x0001,      //system menu
                fcf_titlebar      =0x0002,      //title bar
                fcf_minbutton     =0x0004,      //minimize button
                fcf_maxbutton     =0x0008,      //maximize/restore button
                fcf_hidebutton    =0x0010,      //hide button
                fcf_minmaxbuttons =0x000c,      //minimize&maximize buttons
                fcf_bothbuttons   =0x0018,      //both buttons (os/2: hide+max)
                fcf_menu          =0x0020,      //menu
                fcf_horzscroll    =0x0040,      //horizontal scrollbar
                fcf_vertscroll    =0x0080,      //vertical scrollbar
                fcf_border        =0x0100,      //thin border
                fcf_sizeborder    =0x0200,      //size border
                fcf_dialogborder  =0x0300       //dialog border
        };

        // inter-frame relationship
        Bool BringToFront();
        Bool BringToBack();

        // state
        Bool Minimize();
        Bool Maximize();
        Bool Restore();

        //menu
        FMenu *GetMenu();
        void SetMenu( FMenu *m );
        FMenu *GetSystemMenu();
        void RedrawMenu();

        //caption
        Bool SetCaption(const char *s);
        int GetCaption(char *buf, int buflen);
        Bool SetSubtitle(const char *s);
        int GetSubtitle(char *buf, int buflen);

        //WMS only:
        enum WindowState { window_minimized, window_normal, window_maximized };
        WindowState GetWindowState();
private:
        FTitleBar *pTitleBar;
        FTitleBarButton *pSysMenuButton,
                        *pHideButton,
                        *pRestoreButton,
                        *pMinimizeButton,
                        *pMaximizeButton;
        FScrollBar *pVerticalScrollBar,
                   *pHorizontalScrollBar;
        FIcon *icon;
        char *caption;
        char *subtitle;
        FMenu *pMenu,
              *pAutoMenu;
        FMenu *pSysMenu;
        FAcceleratorTable *accel;
        FAcceleratorTable sysaccel;
        WindowState windowState,nextWindowState;  //minimized/normal/maximized
        FRect restoredRect;             //saved when minimized/maximized
        Bool isActive;                  //is window active?
        WmsSWP *owneePSWP;
        unsigned owneeSWPS;
        Bool hiddenDueToOwner;

        void UpdateTitle();
        void ShowHideTitleButtons(WindowState ws);
        void calcMinimizedPos(FRect *rect);
        void saveRestorePos();
        WmsHWND findNextFrame();
        WmsHWND findPrevFrame();
        WmsHWND findNextWindow();
        void deactivate();
protected:
        //icon
        FIcon *GetIcon();
        void SetIcon(FIcon *pIcon);

        Bool Paint();
        Bool TrackFrame(uint32 tfl, uint32 dfl);
        Bool QueryTrackInfo(uint32 fl, void *p);
        Bool ControlPointer(FControlPointerEvent&);
        Bool Button1Down(FButtonDownEvent&);
        Bool MouseActivity(FMouseEvent &);
        Bool SysCommand(FSysCommandEvent&);
        uint32 QueryWindowState();
        Bool SetWindowState(unsigned s);
        Bool QueryActive();
        Bool SetActive(Bool f);
        Bool CalcFrameRect(FRect *r, Bool frameRectProvided);
        Bool UpdateFrame(uint32 fcf);
        unsigned QueryFrameControlCount();
        Bool FormatFrame(WmsSWP *pswp, FRect *client);
        Bool AdjustWindowPos(WmsSWP *swp);
        Bool WindowPosChanged(WmsSWP *swp);
        Bool OwnerPosChanging(WmsSWP *swp, WmsSWP *ownerSWP);
        Bool OwnerStateChanging(unsigned oldState, unsigned newState);
        Bool FocusChanging(WmsHWND otherHwnd, Bool f, uint16 fl);
        unsigned QueryFrameInfo();
        void TextChanging(const char *pszText);
        WmsHWND NextMenu(WmsHWND hwndMenu, Bool prev);
}; // End of FFrameWindow

#endif

