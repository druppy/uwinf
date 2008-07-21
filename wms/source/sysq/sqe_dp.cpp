#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#define WMSINCL_DESKTOP
#define WMSINCL_FOCUS
#define WMSINCL_CAPTURE
#define WMSINCL_BW
#define WMSINCL_MSGS
#define WMSINCL_SYSVALUE
#include <wms.hpp>
#include "sqe_dp.hpp"
#include "sysqueue.hpp"
#include "logkey.hpp"
#include <stdlib.h>

static void DispatchKeyboardEvent(SystemEvent &se);
static void DispatchMouseEvent(SystemEvent &se);
static WmsHWND DoHitTesting(WmsHWND hwnd, FPoint p);


SystemQueueEventDispatcher::SystemQueueEventDispatcher()
  : FThread(),
    please_terminate(0)
{
}

void SystemQueueEventDispatcher::Go() {
        WmsHAB hab=WmsInitialize(0);
        WmsHMQ hmq=WmsCreateMsgQueue(hab,0,0);
        while(!please_terminate) {
                SystemEvent se;
                if(systemEventQueue_internal.get(&se,-1)==0) {
                        if(se.type==se.keyboard)
                                DispatchKeyboardEvent(se);
                        else if(se.type==se.mouse)
                                DispatchMouseEvent(se);
                }
        }
        WmsDestroyMsgQueue(hmq);
        WmsTerminate(hab);
}

void SystemQueueEventDispatcher::PleaseTerminate() {
        please_terminate=1;
        SystemEvent se;
        systemEventQueue_internal.put(&se,0);
}


static void DispatchKeyboardEvent(SystemEvent &se) {
        WmsMPARAM mp1 = MPFROM4UINT8(se.u.ske.scancode, se.u.ske.character, se.u.ske.logicalkey, se.keyboardShiftState);
        WmsPostMsg(WmsQueryFocus(HWND_DESKTOP),
                   WM_KEYBOARD,
                   mp1,
                   MPFROMUINT16(se.u.ske.flags)
                  );
}

static void DispatchMouseEvent(SystemEvent &se) {
        static unsigned prevButtonState=0;
        static int prevColumn=0;
        static int prevRow=0;
        //
        static int lastDownButton=0;
        static TimeStamp lastDownTime=0;
        static int lastDownX=0;
        static int lastDownY=0;

        WmsHWND hwnd_desktop=HWND_DESKTOP;
        unsigned cxdblclk=(unsigned)WmsQuerySysValue(hwnd_desktop,SV_CXDBLCLK);
        unsigned cydblclk=(unsigned)WmsQuerySysValue(hwnd_desktop,SV_CYDBLCLK);
        unsigned dblclktime=(unsigned)WmsQuerySysValue(hwnd_desktop,SV_DBLCLKTIME);
#define IsDblClk(db,dt,dx,dy) (lastDownButton==db && lastDownTime>dt-dblclktime && abs(lastDownX-dx)<cxdblclk && abs(lastDownY-dy)<cydblclk)
#define MarkDown(db,dt,dx,dy) lastDownButton=db,lastDownTime=dt,lastDownX=dx,lastDownY=dy


        FPoint p(se.u.sme.column,se.u.sme.row);
        WmsHWND hwnd=WmsQueryCapture(hwnd_desktop);
        if(!hwnd)
                hwnd=WmsWindowFromPoint(hwnd_desktop,&p,True,HWND_TOP);
        if(!hwnd) goto thatsIt;
        WmsMapWindowPoints(HWND_DESKTOP,hwnd,&p,1);
        hwnd=DoHitTesting(hwnd,p);
        if(!hwnd) goto thatsIt;
        {
                p.Set(se.u.sme.column,se.u.sme.row);
                WmsMapWindowPoints(hwnd_desktop,hwnd,&p,1);
                WmsMPARAM mp1 = MPFROM2UINT16(p.x,p.y);
                WmsMPARAM mp2 = MPFROM4UINT8(se.keyboardShiftState,se.u.sme.buttonState,0,0);

                if(prevColumn!=se.u.sme.column ||
                   prevRow!=se.u.sme.row) {
                        WmsPostMsg(hwnd,WM_MOUSEMOVE,mp1,mp2);
                }
                if((prevButtonState&1)==0 &&
                   (se.u.sme.buttonState&1)==1) {
                        if(IsDblClk(1,se.timestamp,se.u.sme.column,se.u.sme.row))
                                WmsPostMsg(hwnd,WM_BUTTON1DBLCLK,mp1,mp2);
                        else
                                WmsPostMsg(hwnd,WM_BUTTON1DOWN,mp1,mp2);
                        MarkDown(1,se.timestamp,se.u.sme.column,se.u.sme.row);
                }
                if((prevButtonState&1)==1 &&
                   (se.u.sme.buttonState&1)==0) {
                        WmsPostMsg(hwnd,WM_BUTTON1UP,mp1,mp2);
                }
                if((prevButtonState&2)==0 &&
                   (se.u.sme.buttonState&2)==2) {
                        if(IsDblClk(2,se.timestamp,se.u.sme.column,se.u.sme.row))
                                WmsPostMsg(hwnd,WM_BUTTON2DBLCLK,mp1,mp2);
                        else
                                WmsPostMsg(hwnd,WM_BUTTON2DOWN,mp1,mp2);
                        MarkDown(2,se.timestamp,se.u.sme.column,se.u.sme.row);
                }
                if((prevButtonState&2)==2 &&
                   (se.u.sme.buttonState&2)==0) {
                        WmsPostMsg(hwnd,WM_BUTTON2UP,mp1,mp2);
                }
                if((prevButtonState&4)==0 &&
                   (se.u.sme.buttonState&4)==4) {
                        if(IsDblClk(3,se.timestamp,se.u.sme.column,se.u.sme.row))
                                WmsPostMsg(hwnd,WM_BUTTON3DBLCLK,mp1,mp2);
                        else
                                WmsPostMsg(hwnd,WM_BUTTON3DOWN,mp1,mp2);
                        MarkDown(3,se.timestamp,se.u.sme.column,se.u.sme.row);
                }
                if((prevButtonState&4)==4 &&
                   (se.u.sme.buttonState&4)==0) {
                        WmsPostMsg(hwnd,WM_BUTTON3UP,mp1,mp2);
                }
        }
thatsIt:
        prevButtonState=se.u.sme.buttonState;
        prevColumn=se.u.sme.column;
        prevRow=se.u.sme.row;
}

static WmsHWND DoHitTesting(WmsHWND hwnd, FPoint p) {
        for(;;) {
                WmsMPARAM mp1 = MPFROM2UINT16(p.x,p.y);
                WmsMRESULT mr=WmsSendMsgTimeout(hwnd,WM_HITTEST,mp1,0,1000);
                switch(UINT161FROMMR(mr)) {
                        case HT_NORMAL:
                                return hwnd;
                        case HT_TRANSPARENT: {
                                WmsHWND w=WmsWindowFromPoint(WmsQueryWindowParent(hwnd),&p,True,hwnd);
                                if(w==0)
                                        return 0;
                                WmsMapWindowPoints(hwnd,w,&p,1);
                                hwnd = w;
                                break;
                        }
                        case HT_ERROR:
                                //todo: beep
                                //FALLTHROUGH
                        case HT_DISCARD:
                        default:
                                return 0;
                }
        }
}

