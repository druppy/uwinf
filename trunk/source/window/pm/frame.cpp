/***
Filename: frame.cpp
Description:
  Implementation of FFrame and related stuff
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 97-02-01 Added support for resource FModule specification
***/
#define BIFINCL_WINDOW
#define BIFINCL_FRAME
#define BIFINCL_APPLICATION
#include <bif.h>
#include <dbglog.h>
#include <string.h>

//FFrameMenuDestroyHandler--------------------------------------------------------
//Since the frame window destroys the menu itself and doesn't let OS/2 do it,
//we have to remove the menu from the frame before its destroyed. This handler fixes it by removing
//the menu before the window is destroyed
FFrameMenuDestroyHandler::FFrameMenuDestroyHandler(FFrameWindow *pff)
  : FHandler(pff)
{
        SetDispatch(dispatchFunc_t(&FFrameMenuDestroyHandler::Dispatch));
}

Bool FFrameMenuDestroyHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_DESTROY) {
                HWND hwndMenu=WinWindowFromID(GetWnd()->GetHwnd(),FID_MENU);
                if(hwndMenu) {
                        WinSetParent(hwndMenu,HWND_OBJECT,FALSE);
                        WinSetOwner(hwndMenu,HWND_OBJECT);
                }
        }
        return False;
}


//FFrameWindow--------------------------------------------------------------------
FFrameWindow::FFrameWindow()
  : FWnd(), FFrameMenuDestroyHandler(this)
{
        TRACE_METHOD1("FFrameWindow::FFrameWindow");

        icon=0;
        caption=0;
        subtitle=0;
        pAutoMenu=pMenu=0;
}

FFrameWindow::~FFrameWindow() {
        TRACE_METHOD1("FFrameWindow::~FFrameWindow");
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
}


Bool FFrameWindow::Create(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect, FModule *module) {
        TRACE_METHOD1("FFrameWindow::Create");
        HMODULE hmod;
        if(module)
                hmod = module->GetMODULE();
        else
                hmod = FModule::GetDefaultResourceModule()->GetMODULE();

        //Translate symbolic names to WS_weird_stuff, FCF_weird_stuff and FS_weird_stuff
        ULONG flStyle=0;
        ULONG flCreateFlags=0;

        if(fcf&fcf_sysmenu)
                flCreateFlags|=FCF_SYSMENU;
        if(fcf&fcf_titlebar)
                flCreateFlags|=FCF_TITLEBAR;
        if(fcf&fcf_minbutton)
                flCreateFlags|=FCF_MINBUTTON;
        if(fcf&fcf_maxbutton)
                flCreateFlags|=FCF_MAXBUTTON;
        if(fcf&fcf_hidebutton)
                flCreateFlags|=FCF_HIDEBUTTON;
        if(fcf&fcf_menu && alf&alf_menu)
                flCreateFlags|=FCF_MENU;
        if(fcf&fcf_horzscroll)
                flCreateFlags|=FCF_HORZSCROLL;
        if(fcf&fcf_vertscroll)
                flCreateFlags|=FCF_VERTSCROLL;
        if((fcf&0x0300) == fcf_border)
                flCreateFlags|=FCF_BORDER;
        if((fcf&0x0300) == fcf_sizeborder)
                flCreateFlags|=FCF_SIZEBORDER;
        if((fcf&0x0300) == fcf_dialogborder)
                flCreateFlags|=FCF_DLGBORDER;

        if(alf&alf_icon)
                flCreateFlags|=FCF_ICON;
        if(alf&alf_accelerator)
                flCreateFlags|=FCF_ACCELTABLE;


        if(!pRect) {
                //Let OS/2 decide position and size
                flCreateFlags|=FCF_SHELLPOSITION;
        } 
        flCreateFlags|=FCF_TASKLIST;

        flCreateFlags|=FCF_AUTOICON;

        HWND hwndParent;
        if(pParent)
                hwndParent=pParent->GetHwnd();
        else
                hwndParent=HWND_DESKTOP;

        HWND hwndFrame;
        hwndFrame=WinCreateStdWindow(hwndParent,
                                     flStyle,
                                     &flCreateFlags,
                                     NULL,              //no client
                                     "",                //titlebar text
                                     0,                 //no client = no client style
                                     hmod,              //module containing resources
                                     Id,                //ID
                                     NULL               //no client hwnd
                                    );

        if(hwndFrame==NULL) {
                NOTE1("FFrameWindow::Create: Creation failed");
                return False;
        } 

        GetWndMan()->SubRegWnd(this,hwndFrame);

        if(alf&alf_caption) {
                char s[128];
                if(GetCurrentApp()->LoadString(GetWndMan()->GetHAB(),Id,s,128)!=0) { 
                        if(s[0]=='-')
                                SetSubtitle(s+1);
                        else
                                SetCaption(s);
                }
        }

        if(pRect) {
                WinSetWindowPos(hwndFrame, NULL, pRect->GetLeft(),pRect->GetTop(),pRect->GetWidth(),pRect->GetHeight(), SWP_SIZE|SWP_MOVE);
        }

        return True;
}


/***
Visibility: public
Description:
        Bring the frame in front of al other frames
Return:
        True on success
Note:
        The frame is also activated
***/
Bool FFrameWindow::BringToFront() {
        TRACE_METHOD1("FFrameWindow::BringToFront");
        return (Bool)WinSetWindowPos(GetHwnd(), HWND_TOP, 0,0,0,0, SWP_ZORDER|SWP_ACTIVATE);
}

/***
Visibility: public
Description:
        Move the frame behind al other frames
Return:
        True on success
Note:
        The frame is also deactivated
***/
Bool FFrameWindow::BringToBack() {
        TRACE_METHOD1("FFrameWindow::BringToBack");
        return (Bool)WinSetWindowPos(GetHwnd(), HWND_BOTTOM, 0,0,0,0, SWP_ZORDER|SWP_DEACTIVATE);
}

/***
Visibility: public
Description:
        Minimize the frame to an icon
Return:
        True on success
Note:
        Frame windows can always be minimized regardless of wether they have 
        a minimize button or not
***/
Bool FFrameWindow::Minimize() {
        TRACE_METHOD1("FFrameWindow::Minimize");
        return (Bool)WinSetWindowPos(GetHwnd(), NULL, 0,0,0,0, SWP_MINIMIZE);
}

/***
Visibility: public
Description:
        Maximize the frame into a fullscreen window (subject to size restrictions)
Return:
        True on success
***/
Bool FFrameWindow::Maximize() {
        TRACE_METHOD1("FFrameWindow::Maximize");
        return (Bool)WinSetWindowPos(GetHwnd(), NULL, 0,0,0,0, SWP_MAXIMIZE);
}

/***
Visibility: public
Description:
        Restore the frame to its normal size and position
Return:
        True on success
***/
Bool FFrameWindow::Restore() {
        TRACE_METHOD1("FFrameWindow::Restore");
        return (Bool)WinSetWindowPos(GetHwnd(), NULL, 0,0,0,0, SWP_RESTORE);
}


//menu management
/***
Visibility: public
Description:
        Return a pointer to the menu of the frame
Return:
        pointer to a menu (can be NULL)
Note:
        A NULL return value doesn't imply that the frame doesn't contain a 
        menu, but rather that the frame could not or would not return its 
        menu.
***/
FMenu *FFrameWindow::GetMenu() {
        TRACE_METHOD1("FFrameWindow::GetMenu");
        if(pMenu) 
                return pMenu;
        else if(pAutoMenu)
                return pAutoMenu;
        else {
                //create alias for the menu automatically
                HWND hwndMenu=WinWindowFromID(GetHwnd(),FID_MENU);
                if(hwndMenu)
                        pAutoMenu=new FMenu(hwndMenu,False);
                return pAutoMenu;
        }
}

/***
Visibility: public
Description:
        Set the menu of the frame window
Parameters:
        newmenu  the new menu of the frame
Note:
        The previous menu (if any) is remove and destroyed.
        The frame is repainted as necessary.
***/
void FFrameWindow::SetMenu(FMenu *newmenu) {
        TRACE_METHOD1("FFrameWindow::SetMenu");
        //remove old menu if any
        WinSetParent(WinWindowFromID(GetHwnd(),FID_MENU), HWND_OBJECT, FALSE);
        if(pAutoMenu) {
                //delete alias
                delete pAutoMenu;
                pAutoMenu=0;
        }
        if(pMenu) {
                pMenu=0;
        }

        if(newmenu) {
                //Insert the new menu
                //Ensure that the menu has the 'magic' FID_MENU ID
                WinSetWindowUShort(newmenu->GetHandle(), QWS_ID, FID_MENU);
                //Reparent the menu
                WinSetParent(newmenu->GetHandle(), GetHwnd(), FALSE);
                pMenu=newmenu;
        }
        RedrawMenu();   //tell the frame to update the menu
}


/***
Visibility: public
Description:
        Return the system menu of the frame
Return:
        the system menu
***/
FMenu *FFrameWindow::GetSystemMenu() {
        TRACE_METHOD1("FFrameWindow::GetSystemMenu");
        if(!pSysMenu) {
                HWND hwndSysMenu=WinWindowFromID(GetHwnd(), FID_SYSMENU);
                if(hwndSysMenu) 
                        pSysMenu=new FMenu(hwndSysMenu,False);
        }
        return pSysMenu;
}

/***
Visibility: public
Description:
        Redraw the menu in the frame. This function should be called after 
        any changes to the menu.
***/
void FFrameWindow::RedrawMenu() {
        TRACE_METHOD1("FFrameWindow::RedrawMenu");
        WinSendMsg(GetHwnd(),WM_UPDATEFRAME,MPFROMLONG(FCF_MENU),0);
}

/***
Visibility: protected (only experts should call this function)
Description:
        Return the icon of the frame
Return:
        pointer to the icon (can be NULL)
***/
FIcon *FFrameWindow::GetIcon() {
        TRACE_METHOD1("FFrameWindow::GetIcon");
        //<missing> automatic aliasing like the menu stuff
        return icon;
}

/***
Visibility: protected (only experts should call this function)
Description:
        Set the icon of the frame
Parameters:
        pIcon   pointer to the new icon
***/
void FFrameWindow::SetIcon(FIcon *pIcon) {
        TRACE_METHOD1("FFrameWindow::SetIcon");
        icon=pIcon;
        WinSendMsg(GetHwnd(),WM_SETICON,MPFROMP(pIcon->GetHandle()),NULL);
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
        Usually the caption and subtitle is used for either application+object or object+view
***/

Bool FFrameWindow::SetCaption(const char *s) {
        TRACE_METHOD1("FFrameWindow::SetCaption");
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
        TRACE_METHOD1("FFrameWindow::GetCaption");
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
        TRACE_METHOD1("FFrameWindow::SetSubtitle");
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
        TRACE_METHOD1("FFrameWindow::GetSubtitle");
        if(subtitle) {
                if(strlen(subtitle)+1>buflen)
                        return 0;
                strcpy(buf,subtitle);
                return strlen(buf);
        } else
                return 0;
}


void FFrameWindow::UpdateTitle() {
        TRACE_METHOD0("FFrameWindow::UpdateTitle");

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
        WinSetWindowText(GetHwnd(),title);
}

