/***
Filename: mtappmwt.cpp
Description:
  Implementation of FWMainWindowThread and FMTApplication
Host:
  Watcom 10.0a OS/2
History:
  ISJ 95-02-03 Creation
**/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#define BIFINCL_WINDOWTHREAD
#include <bif.h>
#include <dbglog.h>


//FMTApplication has all its members inline


int FWMainWindowThread::Main(int argc, char **argv) {
        int r;
        FApplication *app=pfn();
        if(app) {
                if(GetWndMan()->RegisterThread(GetTID(),GetHAB(),GetHMQ())) {
                        r=app->Main(argc,argv);
                        GetWndMan()->DeregisterThread(GetTID());
                } else {
                        NOTE0("FWMainWindowThread::Main: could not register at FWndMan");
                        r=-1;
                }
                delete app;
        } else {
                NOTE0("FWMainWindowThread::Main: application-creation function returned NULL");
                r=-1;
        }
        return r;
}

