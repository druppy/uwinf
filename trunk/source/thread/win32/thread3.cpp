/***
Filename : thread3.cpp
Description:
  Implementation of FThread class, suspend/resume part only
Host:
  Watcom 10.0a
History:
  ISJ 95-07-18 Creation
***/

#define BIFINCL_THREAD
#include <bif.h>
#include "dbglog.h"



/***
Visibility: public
Return:
  0 if an error occurred while suspending the thread, non-0 otherwise
Description:
  Put the thread in a state so it will not get any timeslices.
***/
int FThread::Suspend() {
        TRACE_METHOD1("FThread::Suspend");
        if(_state!=running)
                return -1;        //we can only suspend a thread that is running

        DWORD rc;
        rc = SuspendThread(threadHandle);
        if(rc==0xFFFFFFFF) {
                NOTE_TEXT1("FThread::Suspend: could not suspend thread");
                return -1;
        } else
                return 0;
}



/***
Visibility: public
Return:
  0 if an error occurred while resuming the thread, non-0 otherwise
Description:
  Resume the thread previously suspended
***/
int FThread::Resume() {
        TRACE_METHOD1("FThread::Resume");
        if(_state!=running)
                return -1;

        DWORD rc = ResumeThread(threadHandle);
        if(rc==0xFFFFFFFF) {
                NOTE_TEXT1("FThread::Resume: could not resume thread");
        }

        return rc!=0;
}



