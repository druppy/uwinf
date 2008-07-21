/***
Filename: bifmain.cpp
Description:
  Implementation of _BIFThreadMain()
Host:
  Watcom 10.0a-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/

#define INCL_NOPMAPI
#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>
#include "dbglog.h"

int _BIFThreadMain(int argc, char *argv[], FMainThread *(*pfn)(int,char**)) {
        TRACE_PROC1("_BIFThreadMain");

        //initialize ThreadManager
        if(FThreadManager::Init())
                return -1;

        int r;
        FMainThread *mainThread = pfn(argc,argv);
        if(mainThread) {
                mainThread->Run(0);
                r = mainThread->retcode;
                delete mainThread;
        } else
                r=-1;

        FThreadManager::Done();

        return r;
}
