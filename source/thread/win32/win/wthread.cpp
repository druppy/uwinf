/***
Filename: wthread.cpp
Description:
  Implementation of FWindowThread
Host:
  Watcom 10.0a
History:
  ISJ 95-07-18 Creation
  ISJ 97-11-05 Removed Run() since it was unnecessary
**/

#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define INCL_WINMESSAGEMGR
#include <bif.h>
#include "dbglog.h"


/***
Visibility: public
Description:
***/
FWindowThread::FWindowThread(unsigned stackNeeded)
  : FThread(stackNeeded)
{
        TRACE_METHOD1("FWindowThread::FWindowThread");

        if(stackNeeded<16384)
                FThread::stackNeeded = 16384;        //GUI threads need a larger stack
}


/***
Visibility: public
Description:
  Nothing needs to be done
  enumerated.
***/
FWindowThread::~FWindowThread() {
        TRACE_METHOD1("FWindowThread::~FWindowThread");
}


/***
Visibility: protected
Description:
  Do a standard messageloop
***/
void FWindowThread::MessageLoop() {
        TRACE_METHOD1("FWindowThread::MessageLoop");

        MSG msg;
        while(GetMessage(&msg,0,0,0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
}
