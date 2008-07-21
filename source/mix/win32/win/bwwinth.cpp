/***
Filename: bwwinth.cpp
Description:
  Implementation of FWWindowThread
Host:
  Watcom 10.0a OS/2
History:
  ISJ 95-02-03 Creation
  ISJ 96-04-26 Modified for PreGo() stuff
  ISJ 96-09-05 Copied from os2 and modified for win32
**/
#define BIFINCL_WINDOW
#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#include <bif.h>
#include <dbglog.h>


int FWWindowThread::PreGo() {
        if(FWindowThread::PreGo()!=0) return -1;
        if(_PreGo()!=0) {FWindowThread::PostGo(); return -1; }
        return 0;
}

int FWWindowThread::_PreGo() {
        if(!GetWndMan()->RegisterThread(GetTID()))
                return -1;
        return 0;
}

void FWWindowThread::PostGo() {
        _PostGo();
        FWindowThread::PostGo();
}

void FWWindowThread::_PostGo() {
        GetWndMan()->DeregisterThread(GetTID());
}

