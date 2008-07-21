/***
Filename: mtappmwt.cpp
Description:
  Implementation of FWMainWindowThread and FMTApplication
Host:
  Watcom 10.0a-10.5 OS/2
History:
  ISJ 95-02-03 Creation
  ISJ 96-04-23 Creation (copied from OS/2 source)
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
        return app->Main(argc,argv);
}

int FWMainWindowThread::PreGo() {
        if(FWindowThread::PreGo()!=0) return -1;
        if(_PreGo()!=0) { FWindowThread::PostGo(); return -1; }
        return 0;
}

int FWMainWindowThread::_PreGo() {
        app = pfn();
        if(!app)
                return -1;
        if(!GetWndMan()->RegisterThread(0,GetHAB(),GetHMQ())) {
                delete app; app=0;
                return -1;
        }
        return 0;
}

void FWMainWindowThread::PostGo() {
        _PostGo();
        FWindowThread::PostGo();
}

void FWMainWindowThread::_PostGo() {
        GetWndMan()->DeregisterThread(0);
        delete app; app=0;
}

