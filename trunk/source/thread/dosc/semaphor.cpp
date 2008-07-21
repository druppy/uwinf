/***
Filename: semaphore.cpp
Description:
  Implementation of FSemaphore
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-08 Creation
  ISJ 94-07-17 Changed count to unsigned long
**/

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include <limits.h>
#include "dbglog.h"

FSemaphore::FSemaphore(long initialCount) {
        TRACE_METHOD1("FSemaphore::FSemaphore");
        count = initialCount;
}

FSemaphore::~FSemaphore() {
        TRACE_METHOD1("FSemaphore::~FSemaphore");
        //FWaitThing wakes all waiters with return code = -1
}

int FSemaphore::Fail() {
        TRACE_METHOD1("FSemaphore::Fail");
        //FSemaphore never fails
        return 0;
}

/***
Visibility: public
Return:
        0 if succesful, nonzero otherwise
Description:
        Increment the count or wake a waiter
***/
int FSemaphore::Signal() {
        TRACE_METHOD1("FSemaphore::Signal");
        if(MoreWaiters())
                WakeOneWaiter(0);
        else {
                if(count==LONG_MAX) {
                        NOTE_TEXT1("FSemaphore::Signal: LONG_MAX reached");
                        return -1;
                }
                count++;
        }
        return 0;
}

/***
Visibility: private protected public
Parameters:
        timeout the maximum number of milliseconds to wait
Return:
        0 on success
Description:
        Decrement the count if it is positive, if it is zero then go to sleep
***/
int FSemaphore::Wait(long timeout) {
        TRACE_METHOD1("FSemaphore::Wait");
        if(count>0) {
                count--;
                return 0;
        } else {
                int rc=GenericWait(timeout);
                return rc;
        }
}
