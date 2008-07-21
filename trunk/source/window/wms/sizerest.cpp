/***
Filename: sizerest.cpp
Description:
  Implementation of size restriction handlers
Host:
  Watcom 10.0a-10.6
History:
  ISJ 95-01-24 Creation
  ISJ 95-08-14 Created the AdjustParentPos() function
  ISJ 96-10-20 Modified for WMS
***/

#define BIFINCL_WINDOW
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_SIZERESTRICTIONHANDLERS
#define BIFINCL_FRAME
#define BIFINCL_TRACKRECT
#define WMSINCL_MSGS
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
        else if(ev.GetID()==WM_ADJUSTWINDOWPOS) {
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
        unsigned ws=(unsigned)WmsSendMsg(WmsQueryWindowParent(GetWnd()->GetHwnd()), WM_QUERYWINDOWSTATE,0,0);
        if(ws==FFrameWindow::window_minimized)
                return False;
        if((ev.GetFlags()&(TF_SIZE|TF_LEFT|TF_TOP|TF_BOTTOM|TF_RIGHT))==0)
                return False;

        TrackInfo *p=ev.GetTrackInfo();


        int minw,       //setup below
            minh=0,
            maxw=GetDesktop()->GetWidth()*10,
            maxh=GetDesktop()->GetHeight()*10;
        minw=0;
        if(WmsWindowFromID(pFrame->GetHwnd(),FID_SYSMENU)) {
                //frame has a system menu
                minw+=3;
        }
        if(WmsWindowFromID(pFrame->GetHwnd(),FID_MINIMIZE)) {
                //frame has min/max/hide buttons
                minw+=3;
        }
        if(WmsWindowFromID(pFrame->GetHwnd(),FID_MAXIMIZE)) {
                //frame has min/max/hide buttons
                minw+=3;
        }
        if(WmsWindowFromID(pFrame->GetHwnd(),FID_HIDE)) {
                //frame has min/max/hide buttons
                minw+=3;
        }
        if(WmsWindowFromID(pFrame->GetHwnd(),FID_TITLEBAR)) {
                minw+=3;
        }
        minw+=1;

        //ask the subclass what it thinks
        GetMinimumWidth(minw);
        GetMinimumHeight(minh);
        GetMaximumWidth(maxw);
        GetMaximumHeight(maxh);

        //setup trackinfo
        //initial tracking rect is the current frame rectangle
        FRect fr;
        WmsQueryWindowRect(pFrame->GetHwnd(),&fr);
        p->track=fr;
        //boundary
        p->bounding.Set(0,0,GetDesktop()->GetWidth(),GetDesktop()->GetHeight());
        FRect cr=fr;
        WmsSendMsg(pFrame->GetHwnd(),WM_CALCFRAMERECT,MPFROMP(&cr),MPFROMUINT16(True));
        //minimum tracking size
        p->minWidth = minw + (fr.GetWidth()-cr.GetWidth());
        p->minHeight= minh + (fr.GetHeight()-cr.GetHeight());
        //maximum tracking size
        p->maxWidth = maxw + (fr.GetWidth()-cr.GetWidth());
        p->maxHeight= maxh + (fr.GetHeight()-cr.GetHeight());
        //tracking flags
        p->fl=ev.GetFlags();

        ev.SetResult(MRFROMUINT16(True));
        return True;
}


/***
Description:
        Applies the size restrictions provided by 
        the GetMin/MaxWidth/Height() and GetVertical/HorizontalGrid() member 
        functions
***/
Bool FSizeRestrictionClientHandler::ApplySizeRestrictions() {
        uint16 ws=UINT161FROMMR(WmsSendMsg(WmsQueryWindowParent(GetWnd()->GetHwnd()),WM_QUERYWINDOWSTATE,0,0));
        if(ws==FFrameWindow::window_minimized)
                return False;   //frame is minimized

        FRect r;
        Bool resize=False;

        WmsQueryWindowRect(GetWnd()->GetHwnd(),&r);
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
                r.yBottom=r.yTop + tmp;
                resize=True;
        }
        if(GetMaximumWidth(tmp) && tmp<r.GetWidth()) {
                r.xRight=r.xLeft + tmp;
                resize=True;
        }
        if(GetMaximumHeight(tmp) && tmp<r.GetHeight()) {
                r.yBottom=r.yTop + tmp;
                resize=True;
        }

        if(resize) {
                //resize needed
                WmsHWND hwndFrameParent = WmsQueryWindowParent(pFrame->GetHwnd());
                WmsMapWindowRects(GetWnd()->GetHwnd(), hwndFrameParent, &r, 1);
                WmsSendMsg(pFrame->GetHwnd(), WM_CALCFRAMERECT, MPFROMP(&r), MPFROMUINT16(False));
                WmsSetWindowPos(pFrame->GetHwnd(),
                                0,
                                r.xLeft,r.yTop,
                                r.GetWidth(),r.GetHeight(),
                                SWP_SIZE|SWP_MOVE
                               );
        }
        return True;
}

Bool FSizeRestrictionClientHandler::AdjustParentPos(FEvent &ev) {
        //adjust the new size&position of the frame (a WM_ADJUSTWINDOWPOS)

        WmsSWP *pswp= (WmsSWP*)PFROMMP(ev.GetMP1());
        if((pswp->flags&SWP_SIZE)==0) return False;        //not resizing

        uint16 nextws=UINT162FROMMR(WmsSendMsg(WmsQueryWindowParent(GetWnd()->GetHwnd()),WM_QUERYWINDOWSTATE,0,0));
        if(nextws==FFrameWindow::window_minimized)
                return False;   //frame is minimized or going to be minimized

        Bool resize=False;

        FRect r(pswp->x,pswp->y,pswp->x+pswp->cx,pswp->y+pswp->cy);
        WmsSendMsg(pFrame->GetHwnd(), WM_CALCFRAMERECT, MPFROMP(&r), MPFROMUINT16(True));

        int tmp;
        if(GetHorizontalGrid(tmp) && r.GetWidth()%tmp) {
                r.xRight = r.xLeft + (r.GetWidth()/tmp)*tmp;
                resize=True;
        }
        if(GetVerticalGrid(tmp) && r.GetHeight()%tmp) {
                r.yTop = r.yBottom + (r.GetWidth()/tmp)*tmp;
                resize=True;
        }
        if(GetMinimumWidth(tmp) && tmp>r.GetWidth()) {
                r.xRight=r.xLeft + tmp;
                resize=True;
        }
        if(GetMinimumHeight(tmp) && tmp>r.GetHeight()) {
                r.yBottom=r.yTop + tmp;
                resize=True;
        }
        if(GetMaximumWidth(tmp) && tmp<r.GetWidth()) {
                r.xRight=r.xLeft + tmp;
                resize=True;
        }
        if(GetMaximumHeight(tmp) && tmp<r.GetHeight()) {
                r.yBottom=r.yTop + tmp;
                resize=True;
        }

        if(resize) {
                //resize needed
                WmsHWND hwndFrameParent = WmsQueryWindowParent(pFrame->GetHwnd());
                WmsMapWindowRects(GetWnd()->GetHwnd(), hwndFrameParent, &r, 1);
                WmsSendMsg(pFrame->GetHwnd(), WM_CALCFRAMERECT, MPFROMP(&r), MPFROMUINT16(False));

                //pswp->x = r.GetLeft();
                //pswp->y = r.GetBottom();
                pswp->cx = r.GetWidth();
                pswp->cy = r.GetHeight();
        }

        return False; //let other modify if they want to
}

