/***
Filename: applicat.cpp
Description:
  Implementation of FApplication
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define INCL_DOSPROCESS
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#include <bif.h>

static FApplication *pGlobalApp=0; // Pointer to main system class


/***
Visibility: local to this module
Return:
        The HMODULE of the running EXE
***/
static HMODULE GetEXEModule() {
        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib,&ppib);
        return (HMODULE)ppib->pib_hmte;
}

FApplication::FApplication()
  : FModule(GetEXEModule()),
    windowManager()
{
        pGlobalApp = this;        // init the global object holder
}


FApplication::~FApplication()
{
        pGlobalApp = NULL;
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
        return (Bool)WinGetMsg(windowManager.GetHAB(), &event, NULL, 0, 0 );
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
        WinPostQueueMsg(windowManager.GetHMQ(1),WM_QUIT,NULL,NULL);
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
        HAB hab=WinInitialize(0);
        if(!hab) {
                r=-1;
        } else {
                HMQ hmq=WinCreateMsgQueue(hab,0);
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
                        WinDestroyMsgQueue(hmq);
                }
                WinTerminate(hab);
        }
        return r;
}

