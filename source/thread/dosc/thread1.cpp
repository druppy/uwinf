/***
Filename: thread1.cpp
Description:
  Suspend/Resume implementation of FThread
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-17 Creation (moved from thread.cpp)
***/

#define BIFINCL_THREAD
#include <bif.h>

#include "dbglog.h"


/***
Visibility: public (should not be called by the thread itself)
Description:
        Suspend the thread (and tell the thread manager about it)
***/
int FThread::Suspend() {
        TRACE_METHOD1("FThread::Suspend");
        if(FThreadManager::GetCurrentThread()==this) {
                NOTE_TEXT1("FThread::Suspend: attempt to suspend the current thread");
                return -1;
        }
        suspendCount++;
        if(suspendCount==1)
                FThreadManager::ChangeThread(this);
        return 0;
}

/***
Visibility: public
Description:
        Decrement the suspend count and reactivate the thread if it
        reaches zero
***/
int FThread::Resume() {
        TRACE_METHOD1("FThread::Resume");
        if(suspendCount==0) {
                //already resumed
                NOTE_TEXT1("FThread::Resume: thread already running");
                return -1;
        }
        suspendCount--;
        if(suspendCount==0)
                FThreadManager::ChangeThread(this);
        return 0;
}
