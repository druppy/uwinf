#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#define WMSINCL_MSGS
#define WMSINCL_TIMER
#include "wms.hpp"

#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>

#include "sysq/hwmouse.hpp"
#include "sysq/hwkeybrd.hpp"
#include "sysq/sysqueue.hpp"
#include "sysq/sqe_dp.hpp"
#include "paint_dp.hpp"
#include "timer.hpp"

#include <stdio.h>

HWMouse                         hwmouse_internal;
HWKeyboard                      hwkeyboard_internal;
SystemEventQueue                systemEventQueue_internal;
SystemQueueEventDispatcher      sqed;
PaintDispatcherThread           pdt;
WmsTimerThread                  timerthread;

class MyMain:public FMainThread {
public:
        MyMain(int argc, char **argv) :FMainThread(argc,argv) {}

        int Main(int,char**);
};


static void InitializeWMS() {
        //
        //initialize WMS
        sqed.Start();
        hwkeyboard_internal.Initialize();
        hwmouse_internal.Initialize();
        pdt.Start();
        timerthread.Start();
}

static void TerminateWMS() {
        timerthread.PleaseTerminate();
        pdt.PleaseTerminate();
        sqed.PleaseTerminate();
        hwkeyboard_internal.Terminate();
        hwmouse_internal.Terminate();
        pdt.Wait();
        sqed.Wait();
        timerthread.Wait();
}

int MyMain::Main(int,char**) {
        InitializeWMS();
                                       
        WmsHAB hab=WmsInitialize(0);
        if(!hab) return -1;

        WmsHMQ hmq=WmsCreateMsgQueue(hab,0,0);
        if(!hmq) {
                WmsTerminate(hab);
                return -2;
        }

        WmsStartTimer((WmsHWND)hmq,1,1000,False);
        WmsQMSG qmsg;
        while(WmsGetMsg(hmq,&qmsg)) {
                //nothing
                printf("%ld\n",(long)qmsg.mp1);
        }

        WmsDestroyMsgQueue(hmq);
        WmsTerminate(hab);

        TerminateWMS();

        return 0;
}

DEFBIFTHREADMAIN(MyMain);

