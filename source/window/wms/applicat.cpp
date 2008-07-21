/***
Filename: applicat.cpp
Description:
  Implementation of FMainApp
Host:
  Watcom 10.5-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-04-21 Creation. Copied from OS/2 and modified for WMS
***/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define WMSINCL_MSGS
#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#include <bif.h>

static FApplication *pGlobalApp=0; // Pointer to main system class

FApplication::FApplication()
  : FModule(),
    windowManager()
{
        pGlobalApp = this;        // init the global object holder
}


FApplication::~FApplication()
{
        pGlobalApp = 0;
}

FApplication * BIFFUNCTION GetCurrentApp() {
        return pGlobalApp; 
}


/***
Visibility: public (only experts should call this function)
Description:
        Return a message from the messagequeue.
Return:
        False if the event is a WM_QUIT
***/
Bool FApplication::GetEvent( FEvent &event )
{
        Bool b=(Bool)WmsGetMsg(windowManager.GetHMQ(0), &event );
        return b;
}

/***
Visibility: public
Description:
        Standard message-loop
***/
void FApplication::MessageLoop() {
        FEvent e;
        while(GetEvent(e))
                windowManager.PumpEvent(e);
}

/***
Visibility: public
Description:
        Terminate the application. The application is not terminated at once,
        but rather suggested that it should return from its message-loop
***/
void FApplication::TerminateApp() {
        WmsPostQueueMsg(windowManager.GetHMQ(1),WM_QUIT,0,0);
}


/***
Visibility: public
Description:
        Run the application. This is done by calling StartUp(), MessageLoop() and ShutDown()
Return:
        return code to OS
***/
int FApplication::Main(int argc, char **argv) {
        if(StartUp(argc,argv)) {
                MessageLoop();
                return ShutDown();
        } else
                return -1;      //could not initialize
}



//startup---------------------------------------------------------------------
/***
Visibility: public
Description:
        Start and run the BIF/Window application
Parameters:
        (OS-dependent)
        argc      argument count
        argv      argument vector
        pfn       pointer to function returning a FApplication pointer. This 
                  way this module can reside in a DLL
Return:
        return code to OS
***/
int BIFFUNCTION _BIFWindowMain(int argc, char **argv, FApplication *(*pfn)()) {
        int r;

        WmsHAB hab=WmsInitialize(0);
        if(!hab) {
                r=-1;
        } else {
                WmsHMQ hmq=WmsCreateMsgQueue(hab,0,0);
                if(!hmq) {
                        r=-2;
                } else {
                        FApplication *app=pfn();
                        if(!app) {
                                r=-3;
                        } else {
                                if(!GetWndMan()->RegisterThread(0,hab,hmq)) {
                                        r=-4;
                                } else {
                                        r=app->Main(argc,argv);
                                        GetWndMan()->DeregisterThread(0);
                                }
                                delete app;
                        }
                        WmsDestroyMsgQueue(hmq);
                }
                WmsTerminate(hab);
        }
        return r;
}

