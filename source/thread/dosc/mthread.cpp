/***
Filename: mthread.cpp
Description:
  Implementation of FMainhread
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-07 Creation
  ISJ 96-06-02 Removed DLL support
**/
#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>
#include "dbglog.h"

FMainThread::FMainThread(int argc, char *argv[]) {
        FMainThread::argc = argc;
        FMainThread::argv = argv;
}


FMainThread::~FMainThread() {
}


/***
Visibility: public (could be protected)
Description:
        Call Main() and save return code for later retrieval by _BIFMain()
***/
void FMainThread::Go() {
        TRACE_METHOD0("FMainThread::Go");
        retcode=Main(argc,argv);
}
