/***
Filename: mtappmwt.cpp
Description:
  Implementation of FWMainWindowThread and FMTApplication
Host:
  Watcom 10.0a OS/2
History:
  ISJ 95-02-03 Creation
  ISJ 96-09-05 Copied from os2 and modified for win32
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
        FApplication::SetHINSTANCE(BIFTGet_hInstance(),BIFTGet_hPrevInstance());
        
        FApplication *app=pfn();
        if(app) {
                if(GetWndMan()->RegisterThread(GetTID())) {
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

