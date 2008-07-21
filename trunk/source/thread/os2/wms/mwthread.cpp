/***
Filename: mwthread.cpp
Description:
  Implementation of FMainWindowThread
Host:
  Watcom 9.5-10.5 OS/2
History:
  ISJ 94-06-16 Creation
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