/***
Filename: sizerest.cpp
Description:
  Implementation of size restriction handlers
Host:
  Watcom 10.0a, BC40, SC61
History:
  ISJ 95-02-06 Creation (copied from os/2 source)
***/

#define BIFINCL_WINDOW
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_SIZERESTRICTIONHANDLERS
#define BIFINCL_FRAME
#include <bif.h>

FSizeRestrictionFrameHandler::FSizeRestrictionFrameHandler(FFrameWindow* pwnd)
  : FHandler(pwnd)
{
         SetDispatch(dispatchFunc_t(&FSizeRestrictionFrameHandler::Dispatch));
}


/***
Visibility: protected
Description:
        Catches WM_GETMINMAXINFO and asks the clienthandler to fill the structure
***/
Bool FSizeRestrictionFrameHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_GETMINMAXINFO && clienthandler) {
                clienthandler->ModifyMMI((FGetMinMaxInfoEvent&)ev);
        }
        return False;
}

/***
Visibility: public
Description
        Applies size restrictions specified by the clienthandler
Return
        True if the size restrictions has been applied
Note:
        It simply calls clienthandler->ApplySizeRestrictions()
***/
Bool FSizeRestrictionFrameHandler::ApplySizeRestrictions() {
        if(clienthandler) 
                return clienthandler->ApplySizeRestrictions();
        else
                return False;
}

static void GetNCSize(HWND hwnd, int &w, int &h);

/***
Visibility: protected
Description:
        Modifies the minmaxinfo structure used by Windows when the user is 
        moving/sizing the frame, or when the size of the window is changing
***/
void FSizeRestrictionClientHandler::ModifyMMI(FGetMinMaxInfoEvent& ev) {
        
        MINMAXINFO FAR *p=ev.GetMMI();

        int minw=p->ptMinTrackSize.x,
            minh=p->ptMinTrackSize.y,
            maxw=p->ptMaxTrackSize.x,
            maxh=p->ptMaxTrackSize.y,
            gx=1,
            gy=1;
        minw=0;
        
        //ask the subclass what it thinks
        Bool minw_matters=GetMinimumWidth(minw);
        Bool minh_matters=GetMinimumHeight(minh);
        Bool maxw_matters=GetMaximumWidth(maxw);
        Bool maxh_matters=GetMaximumHeight(maxh);
        Bool gx_matters=GetHorizontalGrid(gx);
        Bool gy_matters=GetVerticalGrid(gx);

        int NCWidth,NCHeight;
        GetNCSize(pFrame->GetHwnd(),NCWidth,NCHeight);

        if(minw_matters && p->ptMinTrackSize.x-NCWidth<minw) {
                p->ptMinTrackSize.x = minw+NCWidth;
        }
        if(minh_matters && p->ptMinTrackSize.y-NCHeight<minh) {
                p->ptMinTrackSize.y = minh+NCHeight;
        }

        if(maxw_matters && p->ptMaxTrackSize.x-NCWidth>maxw) {
                p->ptMaxTrackSize.x = maxw+NCWidth;
        }
        if(maxh_matters && p->ptMaxTrackSize.y-NCHeight>maxh) {
                p->ptMaxTrackSize.y = maxh+NCHeight;
        }

        if(gx_matters) {
                p->ptMinTrackSize.x = ((p->ptMinTrackSize.x+gx-1)/gx)*gx;
                p->ptMaxTrackSize.x = ((p->ptMinTrackSize.x)/gx)*gx;
        }
        if(gy_matters) {
                p->ptMinTrackSize.y = ((p->ptMinTrackSize.y+gy-1)/gy)*gy;
                p->ptMaxTrackSize.y = ((p->ptMinTrackSize.y)/gy)*gy;
        }
}


/***
Description:
        Applies the size restrictions provided by 
        the GetMin/MaxWidth/Height() and GetVertical/HorizontalGrid() member 
        functions
***/
Bool FSizeRestrictionClientHandler::ApplySizeRestrictions() {
        FRect r;
        Bool resize=False;

        GetWindowRect(GetWnd()->GetHwnd(),&r);
        int width=r.GetWidth();
        int height=r.GetHeight();
        
        int tmp;
        if(GetHorizontalGrid(tmp) && width%tmp) {
                width = (width/tmp)*tmp;
                resize=True;
        }
        if(GetVerticalGrid(tmp) && height%tmp) {
                height = (height/tmp)*tmp;
                resize=True;
        }
        if(GetMinimumWidth(tmp) && tmp>width) {
                width= tmp;
                resize=True;
        }
        if(GetMinimumHeight(tmp) && tmp>height) {
                height= tmp;
                resize=True;
        }
        if(GetMaximumWidth(tmp) && tmp<width) {
                width= tmp;
                resize=True;
        }
        if(GetMaximumHeight(tmp) && tmp<height) {
                height= tmp;
                resize=True;
        }

        if(resize) {
                //resize needed
                int NCWidth,NCHeight;
                GetNCSize(pFrame->GetHwnd(),NCWidth,NCHeight);
                SetWindowPos(pFrame->GetHwnd(),
                             NULL,
                             0,0,
                             width+NCWidth,height+NCHeight,
                             SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER
                            );
        }
        return True;
}

/***
Visibility: local to this module
Description:
        Calculate how much space the non-client area needs
***/
static void GetNCSize(HWND hwnd, int &w, int &h) {
        LONG style=GetWindowLong(hwnd,GWL_STYLE);
        LONG exstyle=GetWindowLong(hwnd,GWL_EXSTYLE);
        w=0;
        h=0;
        if((style&WS_CAPTION)==WS_CAPTION || style&(WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU)) {
                h+= GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);
        }
        if((style&WS_CAPTION)==WS_BORDER) {
                w+= 2*GetSystemMetrics(SM_CXBORDER);
                h+= 2*GetSystemMetrics(SM_CYBORDER);
        }
        if((style&WS_CAPTION)==WS_DLGFRAME || (exstyle&WS_EX_DLGMODALFRAME)) {
                w+= 2*GetSystemMetrics(SM_CXDLGFRAME);
                h+= 2*GetSystemMetrics(SM_CYDLGFRAME);
        }
        if(style&WS_THICKFRAME) {
                w+= 2*GetSystemMetrics(SM_CXFRAME);
                h+= 2*GetSystemMetrics(SM_CYFRAME);
        }
        if(style&WS_VSCROLL) {
                int min,max;
                GetScrollRange(hwnd,SB_VERT,&min,&max);
                if(min!=max)
                        w+= GetSystemMetrics(SM_CXVSCROLL);
        }
        if(style&WS_HSCROLL) {
                int min,max;
                GetScrollRange(hwnd,SB_HORZ,&min,&max);
                if(min!=max)
                        h+= GetSystemMetrics(SM_CYHSCROLL);
        }
        if(GetMenu(hwnd)) {
                h+= GetSystemMetrics(SM_CYMENU)+GetSystemMetrics(SM_CYBORDER);
                //Menus gives trouble when they wrap to fill more than one line
                //There is no way to fix this
        }
#if (BIFOS_==BIFOS_WIN32_)
#  if (WINVER >= 0x0400)
        OSVERSIONINFO ovi;
        ovi.dwOSVersionInfoSize = sizeof(ovi);
        GetVersionEx(&ovi);
        if((ovi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) ||
           (ovi.dwPlatformId==VER_PLATFORM_WIN32_NT && ovi.dwMajorVersion>=4))
        {
                //Win95 and NT 4.0+ has additional interesting styles
                if(exstyle&WS_EX_CLIENTEDGE) {
                        w+= 2*2;
                        h+= 2*2;
                }
                if(exstyle&WS_EX_WINDOWEDGE) {
                        w+= 2*2;
                        h+= 2*2;
                }
        }
#  endif
#endif
}

