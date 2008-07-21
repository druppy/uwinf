/***
Filename : thread3.cpp
Description:
  Implementation of FThread class, suspend/resume part only
Host:
  Watcom 9.5 OS/2
History:
  ISJ 1994-05-14 Creation
***/

#define INCL_NOPMAPI
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

        // Note: the documentation for DosSuspendThread states that:
        // "DosSuspendThread permits the suspension of only
        //  one other thread within the current process."
        // However, this is very strange, since there are no limit on how
        // many threads that can be created initially suspended. So either
        // the documentation is incorrect or we have a problem here. If the
        // documentation is correct it is impossible for us to suspend more
        // that one thread at a time and achieve the semantics described by
        // FThread::suspend. This is ignored because suspend() and resume()
        // is not recommended anyway

        APIRET rc;
        rc = DosSuspendThread(threadID);
        if(rc!=0) {
                NOTE_TEXT0("FThread::Suspend(), DosSuspendThread() failed");
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

        APIRET rc = DosResumeThread(threadID);
        if(rc!=0) {
                NOTE_TEXT0("FThread::Resume(), DosResumeThread() failed");
        }

        return rc!=0;
}



