/***
Filename: sizerest.cpp
Description:
  Implementation of size restriction handlers
Host:
  Watcom 10.0a
History:
  ISJ 95-01-24 Creation
  ISJ 95-08-14 Created the AdjustParentPos() function
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
        Catches WM_QUERYTRACKINFO and asks the clienthandler to fill the structure
***/
Bool FSizeRestrictionFrameHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_QUERYTRACKINFO && clienthandler)
                return clienthandler->QueryTrackInfo((FQueryTrackInfoEvent&)ev);
        else if(ev.GetID()==WM_ADJUSTFRAMEPOS) {
                GetWnd()->CallHandlerChain(ev,this,True);
                clienthandler->AdjustParentPos(ev);
                return True;
        } else
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

static inline int bestKeyboardInc(int inc) {
        //There is no safe way to determine PM default increments, but it 
        //seems that it is usually 8
        int i=inc;
        while(i<8) i+=inc;
        return i;
}

/***
Visibility: protected
Description:
        Initializes the trackinfo structure used by OS/2 PM when the user is 
        moving/sizing the frame
***/
Bool FSizeRestrictionClientHandler::QueryTrackInfo(FQueryTrackInfoEvent& ev) {
        //If the frame is minimized we should not process this event but rather 
        //let the default handling take place. Otherwise the icon behaves strangly 
        //(moves to incorrect position etc.)
        SWP swpFrame;
        WinQueryWindowPos(pFrame->GetHwnd(),&swpFrame);
        if(swpFrame.fl&SWP_MINIMIZE)
                return False;


        PTRACKINFO p=ev.GetTrackInfo();

        int minw,       //setup below
            minh=0,
            maxw=GetDesktop()->GetWidth()*10,
            maxh=GetDesktop()->GetHeight()*10,
            gx=1,
            gy=1;
        minw=0;
        if(WinWindowFromID(pFrame->GetHwnd(),FID_SYSMENU)!=NULLHANDLE) {
                //frame has a system menu
                minw+=WinQuerySysValue(HWND_DESKTOP,SV_CXMINMAXBUTTON);
        }
        if(WinWindowFromID(pFrame->GetHwnd(),FID_MINMAX)!=NULLHANDLE) {
                //frame has min/max/hide buttons
                minw+=WinQuerySysValue(HWND_DESKTOP,SV_CXMINMAXBUTTON)*2;
        }
        if(WinWindowFromID(pFrame->GetHwnd(),FID_TITLEBAR)!=NULLHANDLE) {
                minw+=100;
        }
        minw+=1;

        //ask the subclass what it thinks
        GetMinimumWidth(minw);
        GetMinimumHeight(minh);
        GetMaximumWidth(maxw);
        GetMaximumHeight(maxh);
        Bool gx_matters=GetHorizontalGrid(gx);
        Bool gy_matters=GetVerticalGrid(gx);

        //setup trackinfo
        //border ("rubber band")
        POINTL borderSize;
        WinSendMsg(pFrame->GetHwnd(),WM_QUERYBORDERSIZE,MPFROMP(&borderSize),0);
        p->cxBorder= borderSize.x;
        p->cyBorder= borderSize.y;
        //grid
        p->cxGrid=     gx;
        p->cyGrid=     gy;
        p->cxKeyboard= bestKeyboardInc(gx);
        p->cyKeyboard= bestKeyboardInc(gy);
        //initial tracking rect is the current frame rectangle
        WinQueryWindowRect(pFrame->GetHwnd(),&p->rclTrack);
        WinMapWindowPoints(pFrame->GetHwnd(),HWND_DESKTOP,(PPOINTL)&p->rclTrack,2);
        //boundary
        p->rclBoundary.xLeft=   0;
        p->rclBoundary.yBottom= 0;
        p->rclBoundary.xRight=  GetDesktop()->GetWidth();
        p->rclBoundary.yTop=    GetDesktop()->GetHeight();
        //minimum tracking size
        FRect minRect;
        minRect.xLeft=0;
        minRect.yBottom=0;
        minRect.xRight=minw;
        minRect.yTop=minh;
        WinCalcFrameRect(pFrame->GetHwnd(),&minRect,FALSE);
        p->ptlMinTrackSize.x=minRect.GetWidth();
        p->ptlMinTrackSize.y=minRect.GetHeight();
        //maximum tracking size
        FRect maxRect;
        maxRect.xLeft=0;
        maxRect.yBottom=0;
        maxRect.xRight=maxw;
        maxRect.yTop=maxh;
        WinCalcFrameRect(pFrame->GetHwnd(),&maxRect,FALSE);
        p->ptlMaxTrackSize.x=maxRect.GetWidth();
        p->ptlMaxTrackSize.y=maxRect.GetHeight();
        //tracking flags
        p->fs=ev.GetFlags();
        if(gx_matters || gy_matters)
                p->fs|=TF_GRID;

        ev.SetResult(MRFROMSHORT(TRUE));
        return True;
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
        BOOL frameIsHidden=!WinIsWindowVisible(pFrame->GetHwnd());
        if(frameIsHidden) {
                //if the frame is hidden we must show it. Otherwise
                //WinQueryWindowRect returns an empty rectangle
                WinShowWindow(pFrame->GetHwnd(),TRUE);
        }

RECTL fr;
WinQueryWindowRect(pFrame->GetHwnd(),&fr);
WinMapWindowPoints(pFrame->GetHwnd(),HWND_DESKTOP,(PPOINTL)&fr,2);

        WinQueryWindowRect(GetWnd()->GetHwnd(),&r);
        int tmp;
        if(GetHorizontalGrid(tmp) && r.GetWidth()%tmp) {
                r.xRight = r.xLeft + (r.GetWidth()/tmp)*tmp;
                resize=True;
        }
        if(GetVerticalGrid(tmp) && r.GetHeight()%tmp) {
                r.yBottom = r.yTop - (r.GetWidth()/tmp)*tmp;
                resize=True;
        }
        if(GetMinimumWidth(tmp) && tmp>r.GetWidth()) {
                r.xRight=r.xLeft + tmp;
                resize=True;
        }
        if(GetMinimumHeight(tmp) && tmp>r.GetHeight()) {
                r.yBottom=r.yTop - tmp;
                resize=True;
        }
        if(GetMaximumWidth(tmp) && tmp<r.GetWidth()) {
                r.xRight=r.xLeft + tmp;
                resize=True;
        }
        if(GetMaximumHeight(tmp) && tmp<r.GetHeight()) {
                r.yBottom=r.yTop - tmp;
                resize=True;
        }

        if(resize) {
                //resize needed
                HWND hwndFrameParent=WinQueryWindow(pFrame->GetHwnd(),QW_PARENT);
                WinMapWindowPoints(GetWnd()->GetHwnd(), hwndFrameParent, (PPOINTL)&r, 2);
                WinCalcFrameRect(pFrame->GetHwnd(), &r, FALSE);
                WinSetWindowPos(pFrame->GetHwnd(), 
                                NULL,
                                r.xLeft,r.yBottom,
                                r.GetWidth(),r.GetHeight(),
                                SWP_SIZE|SWP_MOVE
                               );
        }
        if(frameIsHidden) {
                //If the frame initially was hidden it must be hidden again
                WinShowWindow(pFrame->GetHwnd(),FALSE);
        }
        return True;
}

Bool FSizeRestrictionClientHandler::AdjustParentPos(FEvent &ev) {
        //adjust the new size&position of the frame (a WM_ADJUSTFRAMEPOS)

        PSWP pswp= (PSWP)PVOIDFROMMP(ev.GetMP1());

//        if((pswp->fl&SWP_SIZE)==0) return False;        //not resizing
//        if((pswp->fl&SWP_MINIMIZE)) return False;       //not restore or maximize
        if((pswp->fl&(SWP_RESTORE|SWP_MAXIMIZE))==0) return False;

        Bool resize=False;

        FRect r(pswp->x,pswp->y,pswp->x+pswp->cx,pswp->y+pswp->cy);
        WinCalcFrameRect(pFrame->GetHwnd(), &r, TRUE);

        int tmp;
        if(GetHorizontalGrid(tmp) && r.GetWidth()%tmp) {
                r.xRight = r.xLeft + (r.GetWidth()/tmp)*tmp;
                resize=True;
        }
        if(GetVerticalGrid(tmp) && r.GetHeight()%tmp) {
                r.yBottom = r.yTop - (r.GetWidth()/tmp)*tmp;
                resize=True;
        }
        if(GetMinimumWidth(tmp) && tmp>r.GetWidth()) {
                r.xRight=r.xLeft + tmp;
                resize=True;
        }
        if(GetMinimumHeight(tmp) && tmp>r.GetHeight()) {
                r.yBottom=r.yTop - tmp;
                resize=True;
        }
        if(GetMaximumWidth(tmp) && tmp<r.GetWidth()) {
                r.xRight=r.xLeft + tmp;
                resize=True;
        }
        if(GetMaximumHeight(tmp) && tmp<r.GetHeight()) {
                r.yBottom=r.yTop - tmp;
                resize=True;
        }

        if(resize) {
                //resize needed
                HWND hwndFrameParent=WinQueryWindow(pFrame->GetHwnd(),QW_PARENT);
                WinMapWindowPoints(GetWnd()->GetHwnd(), hwndFrameParent, (PPOINTL)&r, 2);
                WinCalcFrameRect(pFrame->GetHwnd(), &r, FALSE);

                //pswp->x = r.GetLeft();
                //pswp->y = r.GetBottom();
                pswp->cx = r.GetWidth();
                pswp->cy = r.GetHeight();
        }

        return False; //let other modify if they want to
}

