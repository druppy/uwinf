/***
Filename: eventsem.cpp
Description:
  Implementation of FEventSem
Host:
  Watcom 10.0a
History:
  ISJ 95-07-18 Creation (copied from OS/2 sources)
***/

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>
#include "dbglog.h"

/***
Visibility: public
Description:
  create an unnamed, private event semaphore
***/
FEventSemaphore::FEventSemaphore() {
        TRACE_METHOD1("FEventSemaphore::FEventSemaphore");

        hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(hEvent==NULL) {
                NOTE1("FEventSemaphore::FEventSemaphore: could not create seamphore");
        }
}



/***
Visibility: public
Parameters:
  posted   if true the event is initially posted
Description:
  create an unnamed, private event semaphore
***/
FEventSemaphore::FEventSemaphore(int posted) {
        TRACE_METHOD1("FEventSemaphore::FEventSemaphore");

        hEvent = CreateEvent(NULL, TRUE, posted?TRUE:FALSE, NULL);
        if(hEvent==NULL) {
                NOTE1("FEventSemaphore::FEventSemaphore: could not create seamphore");
        }
}



/***
Visibility: public
Description:
  destroy the event semaphore
***/
FEventSemaphore::~FEventSemaphore() {
        TRACE_METHOD1("FEventSemaphore::~FEventSemaphore");
        CloseHandle(hEvent);
}


int FEventSemaphore::Fail() {
        TRACE_METHOD1("FEventSemaphore::Fail");
        return hEvent==NULL;
}



/***
Visibility: public
Return:
  0 on success
Description:
  Post the event. It is not an error to post an event already posted
***/
int FEventSemaphore::Post() {
        TRACE_METHOD1("FEventSemaphore::Post");

        if(!hEvent) {
                WARN("FEventSemaphore::post: posting a failed event semaphore");
                return -1;
        }

        if(SetEvent(hEvent))
                return 0;
        else {
                NOTE1("FEventSemaphore::post: failed");
                return -1;
        }
}



/***
Visibility: public
Parameters:
  timeout   maximum number of milliseconds to wait
Return:
  0 on success
Description:
  Wait for the event to be posted
***/
int FEventSemaphore::Wait(long timeout) {
        TRACE_METHOD1("FEventSemaphore::Wait");

        if(!hEvent) {
                WARN("FEventSemaphore::Wait: wating for failed semaphore");
                return -1;
        }

        DWORD rc;
        if(timeout==-1)
                rc = WaitForSingleObject(hEvent, INFINITE);
        else
                rc = WaitForSingleObject(hEvent, (DWORD)timeout);
        if(rc==WAIT_OBJECT_0)
                return 0;
        else {
                NOTE1("FEventSemaphore::Wait: failed/timeout");
                return -1;
        }
}



/***
Visibility: public
Return:
  0 on success
Description:
  reset the event. It is not an error to reset an event that is not posted.
***/
int FEventSemaphore::Reset() {
        TRACE_METHOD1("FEventSemaphore::Reset");

        if(!hEvent) {
                WARN("FEventSemaphore::Reset: resetting failed semaphore");
                return -1;
        }

        if(ResetEvent(hEvent))
                return 0;
        else {
                NOTE1("FEventSemaphore::Reset: failed");
                return -1;
        }
}
