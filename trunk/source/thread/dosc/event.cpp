/***
Filename: event.cpp
Description:
  Implementation of FEventSemaphore
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-08 Creation
**/

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include "dbglog.h"

FEventSemaphore::FEventSemaphore() {
        TRACE_METHOD1("FEventSemaphore::FEventSemaphore");
        posted=0;
}

FEventSemaphore::FEventSemaphore(int posted) {
        TRACE_METHOD1("FEventSemaphore::FEventSemaphore");
        FEventSemaphore::posted = posted;
}

FEventSemaphore::~FEventSemaphore() {
        TRACE_METHOD1("FEventSemaphore::~FEventSemaphore");
        //FWaitThing will wake all waiting threads with return code = -1
}


int FEventSemaphore::Fail() {
        TRACE_METHOD1("FEventSemaphore::Fail");
        //FEventSemaphore never fails
        return 0;
}


/***
Visibility: public
Return:
        0 (always)
Description:
        Set the state to "posted" and wake all waiters.
***/
int FEventSemaphore::Post() {
        TRACE_METHOD1("FEventSemaphore::Post");
        if(!posted) {
                posted=1;
                WakeAllWaiters(0);
        }
        return 0;
}


/***
Visibility: private protected public
Parameters:
        timeout  the maximum wait time in milliseconds
Return:
        0 if the event has happened, nonzero if the timeout expired
Description:
        Wait until the event has been posted or the timeout expires
***/
int FEventSemaphore::Wait(long timeout) {
        TRACE_METHOD1("FEventSemaphore::Wait");
        if(posted) return 0;    //already posted

        int rc=GenericWait(timeout);

        return rc;
}


/***
Visibility: public
Return:
        0 (always)
Description:
        Reset the state to "non-posted".
***/
int FEventSemaphore::Reset() {
        TRACE_METHOD1("FEventSemaphore::Reset");
        posted = 0;
        return 0;
}

