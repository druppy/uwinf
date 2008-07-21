/***
Filename: mouse.cpp
Description:
  Implementation of mouse handlers
Host:
  WC10.5-10.6
History:
  ISJ 94-11-?? Creation
  ISJ 95-02-20 Corrected bug with noninitialized member
  ISJ 96-05-05 Migrated from Windows
**/
#define BIFINCL_WINDOW
#define BIFINCL_MOUSEHANDLERS
#define WMSINCL_CAPTURE
#define WMSINCL_DESKTOP
#define WMSINCL_SYSVALUE
#define WMSINCL_MSGS
#define WMSINCL_HARDWAREMOUSE
#include <bif.h>

#include <stdlib.h>

Bool FMouseEvent::IsButtonDown(int b) const {
        if(b==1) return Bool(UINT82FROMMP(mp2)&1);
        if(b==2) return Bool(UINT82FROMMP(mp2)&2);
        return Bool(UINT82FROMMP(mp2)&4);
}

int FButtonDownEvent::GetButton() const {
        if(GetID()==WM_BUTTON1DOWN) return 1;
        if(GetID()==WM_BUTTON2DOWN) return 2;
        return 3;
}

int FButtonUpEvent::GetButton() const {
        if(GetID()==WM_BUTTON1UP) return 1;
        if(GetID()==WM_BUTTON2UP) return 2;
        return 3;
}

int FButtonDblClkEvent::GetButton() const {
        if(GetID()==WM_BUTTON1DBLCLK) return 1;
        if(GetID()==WM_BUTTON2DBLCLK) return 2;
        return 3;
}


void FControlPointerEvent::SetPointer(FPointer& p) {
        p.UsePointer();
        SetResult((WmsMRESULT)True);
}


FMouseHandler::FMouseHandler(FWnd *pwnd)
  : FHandler(pwnd), hasCapture(0)
{
        SetDispatch(dispatchFunc_t(&FMouseHandler::Dispatch));
}

Bool FMouseHandler::Dispatch(FEvent& ev) {
        switch(ev.GetID()) {
                case WM_BUTTON1DOWN:
                case WM_BUTTON2DOWN:
                case WM_BUTTON3DOWN: {
                        FButtonDownEvent& bdev=(FButtonDownEvent&)ev;
                        return ButtonDown(bdev);
                }
                case WM_BUTTON1UP:
                case WM_BUTTON2UP:
                case WM_BUTTON3UP: {
                        FButtonUpEvent& buev=(FButtonUpEvent&)ev;
                        return ButtonUp(buev);
                }
                case WM_BUTTON1DBLCLK:
                case WM_BUTTON2DBLCLK:
                case WM_BUTTON3DBLCLK: {
                        FButtonDblClkEvent& bdcev=(FButtonDblClkEvent&)ev;
                        return ButtonDblClk(bdcev);
                }
                case WM_MOUSEMOVE: {
                        FMouseMoveEvent& mmev=(FMouseMoveEvent&)ev;
                        return MouseMove(mmev);
                }
                case WM_SETPOINTER: {
                        FControlPointerEvent &cpev=(FControlPointerEvent &)ev;
                        if(cpev.GetHwndCursor()==GetWnd()->GetHwnd())
                                return ControlPointer(cpev);
                        else
                                return ControlPointerNonWindow(cpev);
                }
                default:
                        return False;
        }
}


Bool FMouseHandler::SetCapture() {
        if(hasCapture) {
                hasCapture++;
                return True;
        }
        if(::WmsQueryCapture(HWND_DESKTOP))
                return False;   //cannot take capture from other window
        if(::WmsSetCapture(HWND_DESKTOP,GetWnd()->GetHwnd())) {
                hasCapture++;
                return True;
        } else
                return False;
}

Bool FMouseHandler::ReleaseCapture() {
        if(hasCapture==0)
                return False;
        if(--hasCapture==0)
                ::WmsReleaseCapture(HWND_DESKTOP);
        return True;
}

Bool FMouseHandler::GetCurrentMousePosition(int *x, int *y) {
        WmsHWND hwndCapture=WmsQueryCapture(GetDesktop()->GetHwnd());
        if(hwndCapture!=NULL && hwndCapture!=GetWnd()->GetHwnd()) {
                //if another window has the capture the mouse position should not be queryed
                if(x) *x=0;
                if(y) *y=0;
                return False;
        } else {
                unsigned ux,uy;
                WmsQueryPointerPos(&ux,&uy);
                FPoint p((int)ux,(int)uy);
                WmsMapWindowPoints(GetDesktop()->GetHwnd(), GetWnd()->GetHwnd(), &p, 1);
                if(x) *x=p.x;
                if(y) *y=p.y;
                return True;
        }
}

Bool FMouseHandler::ButtonDown(FButtonDownEvent& ev) {
        int b=ev.GetButton();
        if(b==1) return Button1Down(ev);
        if(b==2) return Button2Down(ev);
        return Button3Down(ev);
}

Bool FMouseHandler::ButtonUp(FButtonUpEvent& ev) {
        int b=ev.GetButton();
        if(b==1) return Button1Up(ev);
        if(b==2) return Button2Up(ev);
        return Button3Up(ev);
}

Bool FMouseHandler::ButtonDblClk(FButtonDblClkEvent& ev) {
        int b=ev.GetButton();
        if(b==1) return Button1DblClk(ev);
        if(b==2) return Button2DblClk(ev);
        return Button3DblClk(ev);
}




//FExtendedMouseHandler--------------------------------------------------------
void FExtendedMouseHandler::button_down(int button, FMouseEvent &ev) {
        if(downButton==0) {
                downButton=button;
                downTime=ev.time;
                downPoint=ev.GetPoint();
                SetCapture();
        } else {
                if(downTime+maxClickTime>=ev.time) {
                        //user pressed two buttons simultaneously
                        ButtonChord(ev);      
                }
        }
}               

void FExtendedMouseHandler::button_up(int button, FMouseEvent &ev) {
        if(downButton==button) {
                if(motionInProgress) {
                        //end motion
                        if(downButton==1) Button1MotionEnd(ev);
                        if(downButton==2) Button2MotionEnd(ev);
                        if(downButton==3) Button3MotionEnd(ev);
                        motionInProgress=False;
                } else {
                        //maybe a click
                        if(abs(ev.GetX()-downPoint.GetX())<=dxMinMove &&
                           abs(ev.GetY()-downPoint.GetY())<=dyMinMove &&
                           ev.time<=downTime+maxClickTime)
                        {
                                //yup - it's a click
                                if(downButton==1) Button1Click(ev);
                                if(downButton==2) Button2Click(ev);
                                if(downButton==3) Button3Click(ev);
                        }
                }
                downButton=0;
                motionInProgress=False;
                ReleaseCapture();
        }
}

FExtendedMouseHandler::FExtendedMouseHandler(FWnd *pwnd)
  : FMouseHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FExtendedMouseHandler::Dispatch));
        dxMinMove = (int)WmsQuerySysValue(HWND_DESKTOP,SV_CXDBLCLK) / 2;
        dyMinMove = (int)WmsQuerySysValue(HWND_DESKTOP,SV_CYDBLCLK) / 2;
        maxClickTime = (int)WmsQuerySysValue(HWND_DESKTOP,SV_DBLCLKTIME);
        downButton=0;
        motionInProgress=False;
}

Bool FExtendedMouseHandler::Dispatch(FEvent& ev) {
        FMouseEvent &mev=(FMouseEvent &)ev;
        switch(ev.GetID()) {
                case WM_BUTTON1DOWN:
                        button_down(1,mev);
                        break;
                case WM_BUTTON2DOWN:
                        button_down(2,mev);
                        break;        
                case WM_BUTTON3DOWN:
                        button_down(3,mev);
                        break;
                case WM_BUTTON1UP:
                        button_up(1,mev);
                        break;
                case WM_BUTTON2UP:
                        button_up(2,mev);
                        break;
                case WM_BUTTON3UP:
                        button_up(3,mev);
                        break;
                case WM_MOUSEMOVE:
                        if(downButton!=0 && !motionInProgress) {
                                FMouseEvent &mev=(FMouseEvent &)ev;
                                if(abs(downPoint.GetX()-mev.GetX())>dxMinMove ||
                                   abs(downPoint.GetY()-mev.GetY())>dyMinMove)
                                {
                                        motionInProgress=True;
                                        if(downButton==1) Button1MotionStart(mev);
                                        if(downButton==2) Button2MotionStart(mev);
                                        if(downButton==3) Button3MotionStart(mev);
                                }
                        }
                        break;
        }
        return FMouseHandler::Dispatch(ev);
}



//FTranslatedMouseHandler-------------------------------------------------------
FTranslatedMouseHandler::FTranslatedMouseHandler(FWnd *pwnd)
  : FExtendedMouseHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FTranslatedMouseHandler::Dispatch));
}


Bool FTranslatedMouseHandler::Dispatch(FEvent& ev) {
        //currently, nothing
        return FExtendedMouseHandler::Dispatch(ev);
}

Bool FTranslatedMouseHandler::Button1MotionStart(FMouseEvent &ev) {
        return MouseSelectStart(ev);
}

Bool FTranslatedMouseHandler::Button1MotionEnd(FMouseEvent &ev) {
        return MouseSelectEnd(ev);
}

Bool FTranslatedMouseHandler::Button1Click(FMouseEvent &ev) {
        if(UINT81FROMMP(ev.GetMP2())&kss_alt)
                return MouseTextEdit(ev);
        else
                return MouseSingleSelect(ev);
}

Bool FTranslatedMouseHandler::Button2MotionStart(FMouseEvent &ev) {
        return MouseDragStart(ev);
}

Bool FTranslatedMouseHandler::Button2MotionEnd(FMouseEvent &ev) {
        return MouseDragEnd(ev);
}

Bool FTranslatedMouseHandler::Button2Click(FMouseEvent &ev) {
        return MouseContextMenu(ev);
}

Bool FTranslatedMouseHandler::Button1DblClk(FButtonDblClkEvent &ev) {
        return MouseOpen(ev);
}

