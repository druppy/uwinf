/***
Filename: mouse.cpp
Description:
  Implementation of mouse handlers
Host:
  Watcom 10.0a-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
**/
#define BIFINCL_WINDOW
#define BIFINCL_MOUSEHANDLERS
#include <bif.h>
#include <stdlib.h>

Bool FMouseEvent::IsButtonDown(int b) const {
        LONG lKeyState;
        if(b==1) lKeyState=WinGetKeyState(HWND_DESKTOP,VK_BUTTON1);
        if(b==2) lKeyState=WinGetKeyState(HWND_DESKTOP,VK_BUTTON2);
        if(b==3) lKeyState=WinGetKeyState(HWND_DESKTOP,VK_BUTTON3);
        if(lKeyState&0x8000)
                return True;
        else
                return False;
}

Bool FMouseEvent::IsCtrlDown() const {
        //we can't use the fsFlags item in the message because FMouseEvent 
        //also includes the extended and translated mouse events which dont 
        //have that item
        if(WinGetKeyState(HWND_DESKTOP,VK_CTRL)&0x8000)
                return True;
        else
                return False;
}

Bool FMouseEvent::IsShiftDown() const {
        if(WinGetKeyState(HWND_DESKTOP,VK_SHIFT)&0x8000)
                return True;
        else
                return False;
}



int FButtonDownEvent::GetButton() const {
        if(msg==WM_BUTTON1DOWN) return 1;
        if(msg==WM_BUTTON2DOWN) return 2;
        return 3;
}

int FButtonUpEvent::GetButton() const {
        if(msg==WM_BUTTON1UP) return 1;
        if(msg==WM_BUTTON2UP) return 2;
        return 3;
}

int FButtonDblClkEvent::GetButton() const {
        if(msg==WM_BUTTON1DBLCLK) return 1;
        if(msg==WM_BUTTON2DBLCLK) return 2;
        return 3;
}


void FControlPointerEvent::SetPointer(FPointer& p) {
        if(GetID()==WM_CONTROLPOINTER)
                SetResult((MRESULT)p.GetHandle());
        else {
                WinSetPointer(HWND_DESKTOP,p.GetHandle());
        }
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
                        FControlPointerEvent &cpev=(FControlPointerEvent &)ev;
                        FMouseMoveEvent& mmev=(FMouseMoveEvent&)ev;
                        Bool b1=ControlPointer(cpev);
                        Bool b2=MouseMove(mmev);
                        if(b1) {
                                //if the pointer shape was set we cannot 
                                //pass the event onto WinDefWindowProc
                                GetWnd()->CallHandlerChain(ev,this,False);
                                return True;
                        } else
                                return b2;
                }
                case WM_CONTROLPOINTER: {
                        FControlPointerEvent &cpev=(FControlPointerEvent &)ev;
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
        if(WinQueryCapture(HWND_DESKTOP))
                return False;   //cannot take capture from other window
        WinSetCapture(HWND_DESKTOP,GetWnd()->GetHwnd());
        hasCapture++;
        return True;
}

Bool FMouseHandler::ReleaseCapture() {
        if(hasCapture==0)
                return False;
        if(--hasCapture==0)
                ::WinSetCapture(HWND_DESKTOP,NULLHANDLE);
        return True;
}

Bool FMouseHandler::GetCurrentMousePosition(int *x, int *y) {
        HWND hwndCapture=WinQueryCapture(HWND_DESKTOP);
        if(hwndCapture!=NULL && hwndCapture!=GetWnd()->GetHwnd()) {
                //mouse position cannot be queryied while another window has the capture
                if(x) *x=0;
                if(y) *y=0;
                return False;
        } else {
                FPoint p;
                ::WinQueryMsgPos(GetWndMan()->GetHAB(0), &p);
                ::WinMapWindowPoints(HWND_DESKTOP, GetWnd()->GetHwnd(), &p, 1);
                if(x) *x=p.GetX();
                if(y) *y=p.GetY();
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
FExtendedMouseHandler::FExtendedMouseHandler(FWnd *pwnd)
  : FMouseHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FExtendedMouseHandler::Dispatch));
}

Bool FExtendedMouseHandler::Dispatch(FEvent& ev) {
        FMouseEvent &mev=(FMouseEvent &)ev;
        switch(ev.GetID()) {
                case WM_CHORD:
                        return ButtonChord((FMouseEvent&)ev);
                case WM_BUTTON1MOTIONSTART:
                        return Button1MotionStart((FMouseEvent&)ev);
                case WM_BUTTON1MOTIONEND:
                        return Button1MotionEnd((FMouseEvent&)ev);
                case WM_BUTTON1CLICK:
                        return Button1Click((FMouseEvent&)ev);
                case WM_BUTTON2MOTIONSTART:
                        return Button2MotionStart((FMouseEvent&)ev);
                case WM_BUTTON2MOTIONEND:
                        return Button2MotionEnd((FMouseEvent&)ev);
                case WM_BUTTON2CLICK:
                        return Button2Click((FMouseEvent&)ev);
                case WM_BUTTON3MOTIONSTART:
                        return Button3MotionStart((FMouseEvent&)ev);
                case WM_BUTTON3MOTIONEND:
                        return Button3MotionEnd((FMouseEvent&)ev);
                case WM_BUTTON3CLICK:
                        return Button3Click((FMouseEvent&)ev);
                default:
                        return FMouseHandler::Dispatch(ev);
        }
}



//FTranslatedMouseHandler-------------------------------------------------------
FTranslatedMouseHandler::FTranslatedMouseHandler(FWnd *pwnd)
  : FExtendedMouseHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FTranslatedMouseHandler::Dispatch));
}


Bool FTranslatedMouseHandler::Dispatch(FEvent& ev) {
        FTranslatedMouseEvent &tme=(FTranslatedMouseEvent &)ev;
        //Note: We ought to check for tme.IsFromMouse, but at least in Warp the fPointer (mp2) flag is always false
        switch(ev.GetID()) {
                case WM_BEGINDRAG:
                        return MouseDragStart(tme); 
                case WM_ENDDRAG:
                        return MouseDragEnd(tme); 
                case WM_BEGINSELECT:
                        return MouseSelectStart(tme); 
                case WM_ENDSELECT:
                        return MouseSelectEnd(tme); 
                case WM_SINGLESELECT:
                        return MouseSingleSelect(tme); 
                case WM_OPEN:
                        return MouseOpen(tme); 
                case WM_CONTEXTMENU:
                        return MouseContextMenu(tme); 
                case WM_TEXTEDIT:
                        return MouseTextEdit(tme); 
                default: 
                        return FExtendedMouseHandler::Dispatch(ev);
        }
}

