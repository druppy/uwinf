#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#define WMSINCL_MSGS
#include "wms.hpp"

#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>

#include "sysq/hwmouse.hpp"
#include "sysq/hwkeybrd.hpp"
#include "sysq/sysqueue.hpp"
#include "sysq/sqe_dp.hpp"
#include "paint_dp.hpp"

#include <stdio.h>

HWMouse                         hwmouse_internal;
HWKeyboard                      hwkeyboard_internal;
SystemEventQueue                systemEventQueue_internal;
SystemQueueEventDispatcher      sqed;
PaintDispatcherThread           pdt;

static WmsHMQ m_hmq;
class MyThread : public FThread {
public:
        MyThread() : FThread() {}
        void Go();
};

void MyThread::Go() {
        WmsHAB hab=WmsInitialize(0);
        if(!hab) return;

        for(int i=0; i<10000; i++)
                WmsPostQueueMsg(m_hmq,1234,(WmsMPARAM)i,0);
        WmsPostQueueMsg(m_hmq,WM_QUIT,0,0);

        WmsTerminate(hab);
}

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
}

static void TerminateWMS() {
        printf("Starting shutdown of paint dispatcher\n");
        pdt.PleaseTerminate();
        printf("Shutdown of paint dispatcher started\n");

        printf("Starting shutdown of system event dispatcher\n");
        sqed.PleaseTerminate();
        printf("Shutdown of system event dispatcher started\n");

        printf("Shutting down keyboard\n");
        hwkeyboard_internal.Terminate();
        printf("Keyboard shut down\n");

        printf("Shutting down mouse\n");
        hwmouse_internal.Terminate();
        printf("Mouse shut down\n");

        printf("Waiting for paint dispatcher to terminate\n");
        pdt.Wait();
        printf("Paint dispatcher terminated\n");

        printf("Waiting for system event dispatcher to terminate\n");
        sqed.Wait();
        printf("system event dispatcher terminated\n");
}

int MyMain::Main(int,char**) {
        InitializeWMS();
                                       
        hwmouse_internal.SetScreenSize(80,25);
        hwmouse_internal.ShowPointer();
/*
        WmsHAB hab=WmsInitialize(0);
        if(!hab) return -1;

        m_hmq=WmsCreateMsgQueue(hab,0,0);
        if(!m_hmq) {
                WmsTerminate(hab);
                return -2;
        }
        MyThread mt;
        mt.Start();

        WmsQMSG qmsg;
        while(WmsGetMsg(m_hmq,&qmsg)) {
                //nothing
                //printf("%ld\n",(long)qmsg.mp1);
        }

        WmsDestroyMsgQueue(m_hmq);
        WmsTerminate(hab);

        mt.Wait();
*/

        Sleep(5000);

        hwmouse_internal.HidePointer();
        TerminateWMS();

        return 0;
}

DEFBIFTHREADMAIN(MyMain);

