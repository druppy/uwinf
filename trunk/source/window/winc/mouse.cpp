/***
Filename: mouse.cpp
Description:
  Implementation of mouse handlers
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
  ISJ 95-02-20 Corrected bug with noninitialized member
  ISJ 97-01-02 Added support for WM_CAPTURECHANGED
**/
#define BIFINCL_WINDOW
#define BIFINCL_MOUSEHANDLERS
#include <bif.h>
#include <stdlib.h>

Bool FMouseEvent::IsButtonDown(int b) const {
        if(b==1) return Bool(wParam&MK_LBUTTON);
        if(b==2) return Bool(wParam&MK_RBUTTON);
        return Bool(wParam&MK_MBUTTON);
}

int FButtonDownEvent::GetButton() const {
        if(message==WM_LBUTTONDOWN) return 1;
        if(message==WM_RBUTTONDOWN) return 2;
        return 3;
}

int FButtonUpEvent::GetButton() const {
        if(message==WM_LBUTTONUP) return 1;
        if(message==WM_RBUTTONUP) return 2;
        return 3;
}

int FButtonDblClkEvent::GetButton() const {
        if(message==WM_LBUTTONDBLCLK) return 1;
        if(message==WM_RBUTTONDBLCLK) return 2;
        return 3;
}


void FControlPointerEvent::SetPointer(FPointer& p) {
        ::SetCursor((HCURSOR)p.GetHandle());
        SetResult(TRUE);
}


FMouseHandler::FMouseHandler(FWnd *pwnd)
  : FHandler(pwnd), hasCapture(0)
{
        SetDispatch(dispatchFunc_t(&FMouseHandler::Dispatch));
}

Bool FMouseHandler::Dispatch(FEvent& ev) {
        switch(ev.GetID()) {
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_MBUTTONDOWN: {
                        FButtonDownEvent& bdev=(FButtonDownEvent&)ev;
                        return ButtonDown(bdev);
                }
                case WM_LBUTTONUP:
                case WM_RBUTTONUP:
                case WM_MBUTTONUP: {
                        FButtonUpEvent& buev=(FButtonUpEvent&)ev;
                        return ButtonUp(buev);
                }
                case WM_LBUTTONDBLCLK:
                case WM_RBUTTONDBLCLK:
                case WM_MBUTTONDBLCLK: {
                        FButtonDblClkEvent& bdcev=(FButtonDblClkEvent&)ev;
                        return ButtonDblClk(bdcev);
                }
                case WM_MOUSEMOVE: {
                        FMouseMoveEvent& mmev=(FMouseMoveEvent&)ev;
                        return MouseMove(mmev);
                }
                case WM_SETCURSOR: {
                        FControlPointerEvent &cpev=(FControlPointerEvent &)ev;
                        if(cpev.GetHwndCursor()==GetWnd()->GetHwnd())
                                return ControlPointer(cpev);
                        else
                                return ControlPointerNonWindow(cpev);
                }
#if (BIFOS_==BIFOS_WIN32_)
#  if (WINVER >= 0x0400)
                case WM_CAPTURECHANGED: {
                        hasCapture=0;
                        return False;
                }
#  endif
#endif
                case WM_CANCELMODE:
                        if(hasCapture) {
                                ::ReleaseCapture();
                                hasCapture=0;
                        }
                        //fallthrough
                default:
                        return False;
        }
}


Bool FMouseHandler::SetCapture() {
        if(hasCapture) {
                hasCapture++;
                return True;
        }
        if(::GetCapture())
                return False;   //cannot take capture from other window
        ::SetCapture(GetWnd()->GetHwnd());
        if(::GetCapture()==GetWnd()->GetHwnd()) {
                hasCapture++;
                return True;
        } else
                return False;
}

Bool FMouseHandler::ReleaseCapture() {
        if(hasCapture==0)
                return False;
        if(--hasCapture==0)
                ::ReleaseCapture();
        return True;
}


Bool FMouseHandler::GetCurrentMousePosition(int *x, int *y) {
        HWND hwndCapture=GetCapture();
        if(hwndCapture!=NULL && hwndCapture!=GetWnd()->GetHwnd()) {
                //If another window has the capture it is none of your
                //business where the mouse is!
                if(x) *x=0;
                if(y) *y=0;
                return False;
        } else {
                DWORD dw=GetMessagePos();
#if BIFOS_==BIFOS_WIN16_
                POINT p=MAKEPOINT(dw);
#else
                POINT p; p.x=MAKEPOINTS(dw).x; p.y=MAKEPOINTS(dw).y;
#endif
                MapWindowPoints(HWND_DESKTOP,GetWnd()->GetHwnd(),&p,1);
                if(x) *x = (int)p.x;
                if(y) *y = (int)p.y;
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
                downTime=::GetMessageTime();
                downPoint=ev.GetPoint();
                SetCapture();
        } else {
                if(downTime+maxClickTime>=::GetMessageTime()) {
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
                           ::GetMessageTime()<=downTime+maxClickTime)
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
        dxMinMove=GetSystemMetrics(SM_CXDOUBLECLK) / 2;
        dyMinMove=GetSystemMetrics(SM_CYDOUBLECLK) / 2;
        maxClickTime = GetProfileInt("windows","DoubleClickSpeed",550);
        downButton=0;
        motionInProgress=False;
}

Bool FExtendedMouseHandler::Dispatch(FEvent& ev) {
        FMouseEvent &mev=(FMouseEvent &)ev;
        switch(ev.GetID()) {
                case WM_LBUTTONDOWN:
                        button_down(1,mev);
                        break;
                case WM_RBUTTONDOWN:
                        button_down(2,mev);
                        break;        
                case WM_MBUTTONDOWN:
                        button_down(3,mev);
                        break;
                case WM_LBUTTONUP:
                        button_up(1,mev);
                        break;
                case WM_RBUTTONUP:
                        button_up(2,mev);
                        break;
                case WM_MBUTTONUP:
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
                case WM_CANCELMODE:
                        if(downButton!=0) {
                                if(motionInProgress)
                                        switch(downButton) {
                                                case 1: Button1MotionEnd(mev); break;
                                                case 2: Button2MotionEnd(mev); break;
                                                case 3: Button3MotionEnd(mev); break;
                                        }
                                downButton=0;
                                ReleaseCapture();
                        }
                        break;
#if (BIFOS_==BIFOS_WIN32_)
#  if (WINVER >= 0x0400)
                case WM_CAPTURECHANGED:
                        if(downButton!=0) {
                                if(motionInProgress)
                                        switch(downButton) {
                                                case 1: Button1MotionEnd(mev); break;
                                                case 2: Button2MotionEnd(mev); break;
                                                case 3: Button3MotionEnd(mev); break;
                                        }
                                downButton=0;
                        }
                        break;
#  endif
#endif
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
        if(GetKeyState(VK_MENU)&0x8000L)
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

