/***
Filename: trackrct.cpp
Description:
  Implementation of TrackRect function
Host:
  Watcom 10.6
History:
  ISJ 96-05-11 Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_MOUSEHANDLERS
#define BIFINCL_KEYBOARDHANDLERS
#define BIFINCL_DYNMSG
#define BIFINCL_TRACKRECT
#define WMSINCL_CPI
#define WMSINCL_DESKTOP
#define WMSINCL_CAPTURE
#define WMSINCL_MSGS
#include <bif.h>

//The FTrackEventEater eats mouse and keyboard messages while tracking
class FTrackEventEater : public FHandler {
public:
        FTrackEventEater(FWnd *pwnd)
          : FHandler(pwnd)
        { SetDispatch(dispatchFunc_t(FTrackEventEater::Dispatch)); }
        Bool Dispatch(FEvent &ev);
};

Bool FTrackEventEater::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_HITTEST) {
                ev.SetResult((WmsMRESULT)HT_NORMAL);
                return True;
        }
        if(ev.GetID()>=WM_MOUSEFIRST && ev.GetID()<=WM_MOUSELAST)
                return True; //eat
        if(ev.GetID()>=WM_KEYBOARDFIRST && ev.GetID()<=WM_KEYBOARDLAST)
                return True; //eat
        return False;
}


//This handler responds to "wm_istracking". It is used to detect wether or
//not tracking already occurs in the window
class FTrackInProgressHandler : public FHandler {
public:
        static WmsMSG wm_istracking;
        FTrackInProgressHandler(FWnd *pwnd)
          : FHandler(pwnd)
        { SetDispatch(dispatchFunc_t(FTrackInProgressHandler::Dispatch)); 
          if(wm_istracking==0) wm_istracking=RegisterDynamicMessage("wm_istracking");
        }
        Bool Dispatch(FEvent &ev);
};

static WmsMSG FTrackInProgressHandler::wm_istracking=0;
Bool FTrackInProgressHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==wm_istracking) {
                ev.SetResult((WmsMRESULT)True);
                return True;
        } else
                return False;
}



class FTrackHandler : public FTrackEventEater, 
                      public FTrackInProgressHandler,
                      public FKeyboardHandler,
                      public FMouseHandler
{
        static WmsMSG wm_endtrack;
        uint32 flDC;
        TrackInfo ti;
        FPoint relativeMousePos;
        Bool relativeMousePosSet;
        uint16 *savebuf;

        void PutRect();
        void PutRect(WmsHDC hdc);
        void RemoveRect();
        void RemoveRect(WmsHDC hdc);
        void ChangeTrackRect(const FRect &newRect);
public:
        FTrackHandler(FWnd *pwnd);

        Bool DoTracking(uint32 fl, TrackInfo *trackinfo);

protected:
        Bool KeyPressed(FKeyboardEvent &ke);
        Bool Button1Down(FButtonDownEvent&);
        Bool Button1Up(FButtonUpEvent&);
        Bool MouseMove(FMouseMoveEvent&);
};

static WmsMSG FTrackHandler::wm_endtrack=0;

FTrackHandler::FTrackHandler(FWnd *pwnd)
  : FTrackEventEater(pwnd),
    FTrackInProgressHandler(pwnd),
    FKeyboardHandler(pwnd),
    FMouseHandler(pwnd)
{
        if(wm_endtrack==0)
                wm_endtrack=RegisterDynamicMessage("wm_endtrack");
}


void FTrackHandler::PutRect() {
        WmsHDC hdc = WmsGetWindowDC(FMouseHandler::GetWnd()->GetHwnd(), flDC);
        if(!hdc) return;

        PutRect(hdc);

        WmsReleaseDC(hdc);
}

void FTrackHandler::PutRect(WmsHDC hdc) {
        //save the rectangle background and draw the tracking rectangle

        uint16 *sp=savebuf;
        uint16 d;
        if(ti.track.yTop==ti.track.yBottom-1) {
                //rectangle is only one cell tall
                for(int x=ti.track.xLeft; x<ti.track.xRight; x++) {
                        CpiQueryCellData(hdc, FPoint(x,ti.track.yTop), &d);
                        *sp++ = d;
                        CpiSetCellData(hdc,   FPoint(x,ti.track.yTop), (uint16)('Ä' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
                }
                return;
        }
        //topleft
        CpiQueryCellData(hdc, FPoint(ti.track.xLeft,ti.track.yTop), &d);
        *sp++ = d;
        CpiSetCellData(hdc,   FPoint(ti.track.xLeft,ti.track.yTop), (uint16)('Ú' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
        //top
        for(int x=ti.track.xLeft+1; x<ti.track.xRight-1; x++) {
                CpiQueryCellData(hdc, FPoint(x,ti.track.yTop), &d);
                *sp++ = d;
                CpiSetCellData(hdc,   FPoint(x,ti.track.yTop), (uint16)('Ä' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
        }
        //topright
        CpiQueryCellData(hdc, FPoint(ti.track.xRight-1,ti.track.yTop), &d);
        *sp++ = d;
        CpiSetCellData(hdc,   FPoint(ti.track.xRight-1,ti.track.yTop), (uint16)('¿' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
        //left
        for(int y=ti.track.yTop+1; y<ti.track.yBottom-1; y++) {
                CpiQueryCellData(hdc, FPoint(ti.track.xLeft,y), &d);
                *sp++ = d;
                CpiSetCellData(hdc,   FPoint(ti.track.xLeft,y), (uint16)('³' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
        }
        //right
        for(y=ti.track.yTop+1; y<ti.track.yBottom-1; y++) {
                CpiQueryCellData(hdc, FPoint(ti.track.xRight-1,y), &d);
                *sp++ = d;
                CpiSetCellData(hdc,   FPoint(ti.track.xRight-1,y), (uint16)('³' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
        }
        //bottomleft
        CpiQueryCellData(hdc, FPoint(ti.track.xLeft,ti.track.yBottom-1), &d);
        *sp++ = d;
        CpiSetCellData(hdc,   FPoint(ti.track.xLeft,ti.track.yBottom-1), (uint16)('À' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
        //bottom
        for(x=ti.track.xLeft+1; x<ti.track.xRight-1; x++) {
                CpiQueryCellData(hdc, FPoint(x,ti.track.yBottom-1), &d);
                *sp++ = d;
                CpiSetCellData(hdc,   FPoint(x,ti.track.yBottom-1), (uint16)('Ä' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
        }
        //bottomright
        CpiQueryCellData(hdc, FPoint(ti.track.xRight-1,ti.track.yBottom-1), &d);
        *sp++ = d;
        CpiSetCellData(hdc,   FPoint(ti.track.xRight-1,ti.track.yBottom-1), (uint16)('Ù' | ((uint16)(clr_lightgreen)<<8) | d&0xf000));
}

void FTrackHandler::RemoveRect() {
        WmsHDC hdc = WmsGetWindowDC(FMouseHandler::GetWnd()->GetHwnd(), flDC);
        if(!hdc) return;

        RemoveRect(hdc);

        WmsReleaseDC(hdc);
}

void FTrackHandler::RemoveRect(WmsHDC hdc) {
        //restore the tracking rectangle background

        uint16 *sp=savebuf;
        if(ti.track.yTop==ti.track.yBottom-1) {
                for(int x=ti.track.xLeft; x<ti.track.xRight; x++)
                        CpiSetCellData(hdc, FPoint(x,ti.track.yTop), *sp++);
                return;
        }
        //topleft
        CpiSetCellData(hdc, FPoint(ti.track.xLeft,ti.track.yTop), *sp++);
        //top
        for(int x=ti.track.xLeft+1; x<ti.track.xRight-1; x++)
                CpiSetCellData(hdc, FPoint(x,ti.track.yTop), *sp++);
        //topright
        CpiSetCellData(hdc, FPoint(ti.track.xRight-1,ti.track.yTop), *sp++);
        //left
        for(int y=ti.track.yTop+1; y<ti.track.yBottom-1; y++)
                CpiSetCellData(hdc, FPoint(ti.track.xLeft,y), *sp++);
        //right
        for(y=ti.track.yTop+1; y<ti.track.yBottom-1; y++)
                CpiSetCellData(hdc, FPoint(ti.track.xRight-1,y), *sp++);
        //bottomleft
        CpiSetCellData(hdc, FPoint(ti.track.xLeft,ti.track.yBottom-1), *sp++);
        //bottom
        for(x=ti.track.xLeft+1; x<ti.track.xRight-1; x++)
                CpiSetCellData(hdc, FPoint(x,ti.track.yBottom-1), *sp++);
        //bottomright
        CpiSetCellData(hdc, FPoint(ti.track.xRight-1,ti.track.yBottom-1), *sp++);
}

void FTrackHandler::ChangeTrackRect(const FRect &newRect) {
        if(newRect.GetWidth()<ti.minWidth) return;
        if(newRect.GetWidth()>ti.maxWidth) return;
        if(newRect.GetHeight()<ti.minHeight) return;
        if(newRect.GetHeight()>ti.maxHeight) return;
        if(ti.fl&TF_ALLINBOUNDARY) {
                if(newRect.xLeft<ti.bounding.xLeft) return;
                if(newRect.xRight>ti.bounding.xRight) return;
                if(newRect.yTop<ti.bounding.yTop) return;
                if(newRect.yBottom>ti.bounding.yBottom) return;
        } else {
                if(newRect.xRight<=ti.bounding.xLeft) return;
                if(newRect.xLeft>=ti.bounding.xRight) return;
                if(newRect.yBottom<=ti.bounding.yTop) return;
                if(newRect.yTop>=ti.bounding.yBottom) return;
        }
        //new rectangle is OK
        if(ti.track.xLeft==newRect.xLeft &&
           ti.track.xRight==newRect.xRight &&
           ti.track.yTop==newRect.yTop &&
           ti.track.yBottom==newRect.yBottom)
                return; //optimize
        WmsHDC hdc=WmsGetWindowDC(FMouseHandler::GetWnd()->GetHwnd(), flDC);
        if(!hdc) return;
        RemoveRect(hdc);
        ti.track = newRect;
        PutRect(hdc);
        WmsReleaseDC(hdc);
        if(ti.fl&TF_SETPOINTERPOS) {
                //todo: set pointer pos
        }
}

Bool FTrackHandler::DoTracking(uint32 fl, TrackInfo *trackinfo) {
        //copy the flags and trackinfo so they can be accessed when wm_xxx arrives
        flDC = fl;
        ti = *trackinfo;

        //get capture
        if(SetCapture()) {
                int x,y;
                GetCurrentMousePosition(&x,&y);
                relativeMousePos.x = x;
                relativeMousePos.y = y;
                relativeMousePosSet=True;
        } else {
                //setting mouse position is postponed until we receive a
                //WM_MOUSExxxxx event
                relativeMousePosSet=False;
        }

        //allocate memory for saving the reactangle background
        unsigned savecells = ti.maxWidth*2+ti.maxHeight*2;
        savebuf = new uint16[savecells];
        if(!savebuf) {
                ReleaseCapture();
                return False;
        }

        //draw then tracking rectangle
        PutRect();

        //msgloop until wm_endtrack is received
        WmsHMQ hmq=GetWndMan()->GetHMQ(0);
        WmsQMSG qmsg;
        while(WmsGetMsg(hmq,&qmsg)) {
                //re-direct mouse messages since WMS does not redirect
                //messages that are already put into the msgqueue
                if(qmsg.msg>=WM_MOUSEFIRST &&
                   qmsg.msg<=WM_MOUSELAST) {
                        if(qmsg.hwnd!=FMouseHandler::GetWnd()->GetHwnd())
                                qmsg.hwnd = FMouseHandler::GetWnd()->GetHwnd();
                }
                if(qmsg.msg==wm_endtrack) {
                        //end tracking
                        if(UINT161FROMMP(qmsg.mp1)) {
                                //copy trackinfo back
                                *trackinfo = ti;
                        }
                        break;
                } else
                        WmsDispatchMsg(&qmsg);
        }
        if(qmsg.msg!=wm_endtrack) {
                //oops - WM_QUIT received - repost
                if(qmsg.hwnd==0)
                        WmsPostMsg(qmsg.hwnd,qmsg.msg,qmsg.mp1,qmsg.mp2);
                else
                        WmsPostQueueMsg(hmq,qmsg.msg,qmsg.mp1,qmsg.mp2);
        }

        //remove the tracking rectangle
        RemoveRect();
        //deallocate savebuf
        delete[] savebuf;
        
        ReleaseCapture();

        if(qmsg.msg==wm_endtrack && UINT161FROMMP(qmsg.mp1))
                return True;
        else
                return False;
}


Bool FTrackHandler::KeyPressed(FKeyboardEvent &ke) {
        FRect r=ti.track;
        unsigned amount=ke.IsCtrlDown()?8:1;

        switch(ke.GetLogicalKey()) {
                case lk_esc: {
                        WmsPostMsg(FMouseHandler::GetWnd()->GetHwnd(),wm_endtrack,MPFROMUINT16(False),0);
                        break;
                }
                case lk_enter:
                case lk_numenter: {
                        WmsPostMsg(FMouseHandler::GetWnd()->GetHwnd(),wm_endtrack,MPFROMUINT16(True),0);
                        break;
                }
                case lk_left: {
                        if(ke.IsShiftDown() && ti.fl&TF_SIZE) {
                                r.xRight -= amount;
                        } else if((!ke.IsShiftDown()) && ti.fl&TF_MOVE) {
                                r.xLeft -= amount;
                                r.xRight -= amount;
                        }
                        ChangeTrackRect(r);
                        break;
                }
                case lk_right: {
                        if(ke.IsShiftDown() && ti.fl&TF_SIZE) {
                                r.xRight += amount;
                        } else if((!ke.IsShiftDown()) && ti.fl&TF_MOVE) {
                                r.xLeft += amount;
                                r.xRight += amount;
                        }
                        ChangeTrackRect(r);
                        break;
                }
                case lk_up: {
                        if(ke.IsShiftDown() && ti.fl&TF_SIZE) {
                                r.yBottom -= amount;
                        } else if((!ke.IsShiftDown()) && ti.fl&TF_MOVE) {
                                r.yTop -= amount;
                                r.yBottom -= amount;
                        }
                        ChangeTrackRect(r);
                        break;
                }
                case lk_down: {
                        if(ke.IsShiftDown() && ti.fl&TF_SIZE) {
                                r.yBottom += amount;
                        } else if((!ke.IsShiftDown()) && ti.fl&TF_MOVE) {
                                r.yTop += amount;
                                r.yBottom += amount;
                        }
                        ChangeTrackRect(r);
                        break;
                }

        }
        return True;
}

Bool FTrackHandler::Button1Down(FButtonDownEvent&) {
        if(!HasCapture()) SetCapture();
        //nothing
        return True;
}

Bool FTrackHandler::Button1Up(FButtonUpEvent &) {
        WmsPostMsg(FMouseHandler::GetWnd()->GetHwnd(),wm_endtrack,MPFROMUINT16(True),0);
        return True;
}

Bool FTrackHandler::MouseMove(FMouseMoveEvent &ev) {
        if(!HasCapture())
                SetCapture();
        if(!relativeMousePosSet && ti.fl&TF_MOVE) {
                relativeMousePos.x = ev.GetX() - ti.track.xLeft;
                relativeMousePos.y = ev.GetY() - ti.track.yTop;
                relativeMousePosSet = True;
        }
        FRect r=ti.track;
        int x=ev.GetX();
        int y=ev.GetY();
        if(ti.fl&TF_MOVE) {
                //moving
                r.MoveAbsolute(x-relativeMousePos.x, y-relativeMousePos.y);
                ChangeTrackRect(r);
        } else {
                //sizing
                if((ti.fl&(TF_LEFT|TF_RIGHT|TF_TOP|TF_BOTTOM)) == 0) {
                        //sizing type not determined yet
                        //if the mouse hits the border we know
                        if(x==ti.track.xLeft) {
                                if(y<r.yTop)
                                        ; //nothing
                                else if(y<=r.yTop+1)
                                        ti.fl |= TF_LEFT|TF_TOP;
                                else if(y<r.yBottom-2)
                                        ti.fl |= TF_LEFT;
                                else if(y<r.yBottom)
                                        ti.fl |= TF_LEFT|TF_BOTTOM;
                                else
                                        ; //nothing
                        } else if(x==r.xRight-1) {
                                if(y<r.yTop)
                                        ; //nothing
                                else if(y<=r.yTop+1)
                                        ti.fl |= TF_RIGHT|TF_TOP;
                                else if(y<r.yBottom-2)
                                        ti.fl |= TF_RIGHT;
                                else if(y<r.yBottom)
                                        ti.fl |= TF_RIGHT|TF_BOTTOM;
                                else
                                        ; //nothing
                        } else if(y==r.yTop) {
                                if(x<r.xLeft)
                                        ; //nothing
                                else if(x<=r.xLeft+1)
                                        ti.fl|= TF_TOP|TF_LEFT;
                                else if(x<r.xRight-2)
                                        ti.fl |= TF_TOP;
                                else if(x<r.xRight)
                                        ti.fl |= TF_TOP|TF_RIGHT;
                                else
                                        ; //nothing
                        } else if(y==r.yBottom-1) {
                                if(x<r.xLeft)
                                        ; //nothing
                                else if(x<=r.xLeft+1)
                                        ti.fl|= TF_BOTTOM|TF_LEFT;
                                else if(x<r.xRight-2)
                                        ti.fl |= TF_BOTTOM;
                                else if(x<r.xRight)
                                        ti.fl |= TF_BOTTOM|TF_RIGHT;
                                else
                                        ; //nothing
                        }
                        //set pointer shape
                        switch(ti.fl&(TF_LEFT|TF_RIGHT|TF_TOP|TF_BOTTOM)) {
                                case TF_LEFT:
                                case TF_RIGHT:
                                        GetSystemWEPointer()->UsePointer();
                                        break;
                                case TF_TOP:
                                case TF_BOTTOM:
                                        GetSystemNSPointer()->UsePointer();
                                        break;
                                case TF_LEFT|TF_TOP:
                                case TF_RIGHT|TF_BOTTOM:
                                        GetSystemNWSEPointer()->UsePointer();
                                        break;
                                case TF_LEFT|TF_BOTTOM:
                                case TF_RIGHT|TF_TOP:
                                        GetSystemNESWPointer()->UsePointer();
                                        break;
                        }
                } else {
                        if(ti.fl&TF_LEFT)
                                r.xLeft = x;
                        if(ti.fl&TF_RIGHT)
                                r.xRight = x+1;
                        if(ti.fl&TF_TOP)
                                r.yTop = y;
                        if(ti.fl&TF_BOTTOM)
                                r.yBottom = y+1;
                        ChangeTrackRect(r);
                }
        }
        return True;
}

Bool TrackRect(FWnd *pwnd, uint32 flDC, TrackInfo *info) {
        if(!pwnd) return False;
        if(!info) return False;
        if(WmsQueryWindowMsgQueue(pwnd->GetHwnd()) != GetWndMan()->GetHMQ(0))
                return False; //tracking is only allowed in the thread's own windows
        if(WmsQueryCapture(HWND_DESKTOP))
                return False; //someone has the capture
        if(UINT161FROMMR(WmsSendMsg(pwnd->GetHwnd(),FTrackInProgressHandler::wm_istracking,0,0)))
                return False; //tracking is in progress

        FTrackHandler th(pwnd);
        return th.DoTracking(flDC,info);
}
 
