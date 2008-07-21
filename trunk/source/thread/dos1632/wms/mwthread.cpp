/***
Filename: mwthread.cpp
Description:
  Implementation of FMainWindowThread
Host:
  Watcom 10.6
History:
  ISJ 96-08-24 Copied from ../../os2/wms/mwthread.cpp
**/

#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>
#include "dbglog.h"


FMainWindowThread::FMainWindowThread(int argc, char *argv[], int queueSize)
  : FThread(),
    FMainThread(argc,argv),
    FWindowThread(0,queueSize)
{
        TRACE_METHOD1("FMainWindowThread::FMainWindowThread");
}

FMainWindowThread::~FMainWindowThread() {
        TRACE_METHOD1("FMainWindowThread::~FMainWindowThread");
}

