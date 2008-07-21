/***
Filename: bwwinth.cpp
Description:
  Implementation of FWWindowThread
Host:
  Watcom 10.0a-10.5 OS/2
History:
  ISJ 95-02-03 Creation
  ISJ 96-04-25 Creation (copied from os2 sources)
**/
#define BIFINCL_WINDOW
#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#include <bif.h>
#include <dbglog.h>



int FWWindowThread::PreGo() {
        if(FWindowThread::PreGo()!=0) return -1;
        if(_PreGo()!=0) { FWindowThread::PostGo(); return -1; }
        return 0;
}
int FWWindowThread::_PreGo() {
        if(!GetWndMan()->RegisterThread(0,GetHAB(),GetHMQ()))
                return -1;
        return 0;
}

void FWWindowThread::PostGo() {
        _PostGo();
}

void FWWindowThread::_PostGo() {
        GetWndMan()->DeregisterThread(0);
}

