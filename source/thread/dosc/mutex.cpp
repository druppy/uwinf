/***
Filename: mutex.cpp
Description:
  Implementation of FMutexSemaphore
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-08 Creation
  ISJ 94-07-10 Fixed bug in Release()
  ISJ 94-12-13 Implemented unusable mutexes (if an owner thread dies without
               releasing it)
  ISJ 96-08-21 Allowed recursive requests
**/

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include "dbglog.h"

/***
Visibility: public
Description:
        Create an unowned mutex
***/
FMutexSemaphore::FMutexSemaphore() {
        TRACE_METHOD1("FMutexSemaphore::FMutexSemaphore");
        failed=0;
        owner=0;
        requests=0;
}

/***
Visibility: private protected public
Parameters:
        owned   "owned" state
Description:
        Create a mutex (usually) owned by the current thread
***/
FMutexSemaphore::FMutexSemaphore(int owned) {
        TRACE_METHOD1("FMutexSemaphore::FMutexSemaphore");
        failed=0;
        if(owned) {
                owner=FThreadManager::GetCurrentThread();
                requests=1;
        } else {
                owner=0;
                requests=0;
        }
}

FMutexSemaphore::~FMutexSemaphore() {
        TRACE_METHOD1("FMutexSemaphore::~FMutexSemaphore");
        //the FWaitThing takes care of waking all threads with retcode = -1
}


int FMutexSemaphore::Fail() {
        TRACE_METHOD1("FMutexSemaphore::Fail");
        return failed;
}


/***
Visibility: public
Parameters:
        timeout maximum wait time in milliseconds, -1 if none
Return:
        0 if the calling thread now owns the mutex, nonzero otherwise
Description:
        Try to take the mutex. if not possible now then go to sleep
***/
int FMutexSemaphore::Request(long timeout) {
        TRACE_METHOD1("FMutexSemaphore::Request");
        if(failed) return -1;

        FThread *ct=FThreadManager::GetCurrentThread();
        if(owner==0) {
                owner=ct;
                requests=1;
                //link into ownedMutex list
                nextOwned=owner->firstOwnedMutex;
                owner->firstOwnedMutex=this;
                return 0;
        }
        if(owner==ct) {
                //The thread already owns the mutex
                requests++;
                return 0;
        }

        int rc=GenericWait(timeout);

        //owner is set in Release()
        return rc;
}


/***
Visibility: public
Return:
        0 if the mutex is succesfully released, nonzero otherwise
Description:
        Release the mutex owned by the current thread.
***/
int FMutexSemaphore::Release() {
        TRACE_METHOD1("FMutexSemaphore::Release");
        if(owner!=FThreadManager::GetCurrentThread()) {
                //not owner
                NOTE_TEXT1("FMutexSemaphore::Release: Attempt to release Mutex not owned by current thread");
                return -1;
        }
        requests--;
        if(requests>0)
                return 0;

        //unlink from the thread's owned-mutex list
        FMutexSemaphore *prev=0, *cur=owner->firstOwnedMutex;
        while(cur!=this) {
                prev=cur;
                cur=cur->nextOwned;
        }
        if(prev)
                prev->nextOwned=cur->nextOwned;
        else
                owner->firstOwnedMutex=cur->nextOwned;
        //
        owner=0;

        //wake a waiter if any
        if(MoreWaiters()) {
                FThread *wt=WakeOneWaiter(0);
                owner=wt;
                requests=1;
                //link into ownedMutex list
                nextOwned=owner->firstOwnedMutex;
                owner->firstOwnedMutex=this;
        }
        return 0;
}
