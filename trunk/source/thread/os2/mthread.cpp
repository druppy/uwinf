/***
Filename: mthread.cpp
Description:
  Implementation of MainThread
Host:
  Watcom 9.5-10.6
History:
  ISJ 94-06-16 Creation
**/

#define INCL_NOPMAPI
#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>
#include "dbglog.h"


FMainThread::FMainThread(int argc, char *argv[]) {
        TRACE_METHOD1("FMainThread::FMainThread");
        FMainThread::argc = argc;
        FMainThread::argv = argv;
}


FMainThread::~FMainThread() {
        TRACE_METHOD1("FMainThread::~FMainThread");
}


void FMainThread::Go() {
        TRACE_METHOD0("FMainThread::Go");
        retcode = Main(argc,argv);
}
