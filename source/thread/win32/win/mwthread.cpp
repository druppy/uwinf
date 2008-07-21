/***
Filename: mwthread.cpp
Description:
  Implementation of FMainWindowThread
Host:
  Watcom 10.0a
History:
  ISJ 95-07-18 Creation
**/

#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>
#include "dbglog.h"


FMainWindowThread::FMainWindowThread(int argc, char *argv[])
  : FThread(),
    FMainThread(argc,argv),
    FWindowThread(0)
{
        TRACE_METHOD1("FMainWindowThread::FMainWindowThread");
}

FMainWindowThread::~FMainWindowThread() {
        TRACE_METHOD1("FMainWindowThread::~FMainWindowThread");
}

