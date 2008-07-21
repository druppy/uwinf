/***
Filename: waitthing.cpp
Description:
  Implementation of FWaitThing
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-08 Creation
  ISJ 94-12-09 Added __WINDOWS__ stuff to support Watcom
  ISJ 96-04-14 Changed to use clktck.h
**/
#define BIFINCL_SEMAPHORES
#define BIFINCL_THREAD
#include <bif.h>

#include "clktck.h"


#include "dbglog.h"


/*
 * The waiting threads are kept in a circular list. The links are in the
 * threads themselves.
 * We could just use a simple list, but some algorithms expect in very subtle
 * ways that semaphores are FIFO structures. We could legally use (almost)
 * any kind of structure, but we should provide the following semantics:
 *   - a thread waiting for a semaphore will eventually get it
 *   - even if a thread is always preceeded by other threads it should
 *     eventually get the semaphore anyway
 * So a FIFO structure (queue) is really the easiest way to do it.
 */



FWaitThing::FWaitThing() {
        TRACE_METHOD0("FWaitThing::FWaitThing");
        firstWaiter = 0;
}


/***
Visibility: protected
Description:
        Wake all waiter with an error return code telling them that the
        wait thing died in the meantime.
***/
FWaitThing::~FWaitThing() {
        TRACE_METHOD0("FWaitThing::~FWaitThing");
        WakeAllWaiters(-1);
}



/***
Visibility: protected
Parameters:
        tp      thread to add to the wait chain
Description:
        Insert the thread into the wait chain
***/
void FWaitThing::AddWaiter(FThread *tp) {
        TRACE_METHOD0("FWaitThing::AddWaiter");
        if(!firstWaiter) {
                //the only waiter
                tp->nextWaiter = tp->prevWaiter = tp;
                firstWaiter = tp;
        } else {
                //put the thread in the end of the queue
                tp->nextWaiter = firstWaiter;
                tp->prevWaiter = firstWaiter->prevWaiter;
                tp->prevWaiter->nextWaiter = tp;
                tp->nextWaiter->prevWaiter = tp;
        }
        tp->waitsFor = this;
        FThreadManager::ChangeThread(tp);
}


/***
Visibility: protected
Parameters:
        tp      thread to remove
Description:
        Remove a waiter from the wait chain
***/
void FWaitThing::RemoveWaiter(FThread *tp) {
        TRACE_METHOD0("FWaitThing::RemoveWaiter");

        if(!firstWaiter) return;
        if(tp->waitsFor!=this) {
                WARN_TEXT("FWaitThing::RemoveWaiter: thread does not wait for us ?!?");
                return;
        }
        if(tp==tp->nextWaiter) {
                //only waiter
                firstWaiter = 0;
        } else {
                if(tp==firstWaiter)
                        firstWaiter=tp->nextWaiter;
                tp->nextWaiter->prevWaiter = tp->prevWaiter;
                tp->prevWaiter->nextWaiter = tp->nextWaiter;
        }
        tp->nextWaiter = tp->prevWaiter = 0;
        tp->waitsFor=0;
        FThreadManager::ChangeThread(tp);
}



/***
Visibility: protected
Parameters:
        timeout timeout on waiting (-1 if none)
Description:
        Perform a generic wait. The current thread is blocked until it is
        either woken up or timed out
***/
int FWaitThing::GenericWait(long timeout) {
        TRACE_METHOD0("FWaitThing::GenericWait");
        if(timeout==0)
                return -1;

        FThread *ct=FThreadManager::GetCurrentThread();

        if(timeout!=-1) {
                ct->timeoutEndTime = GetCurrentTickCount() + ms2ClockTicks(timeout);
        } else
                ct->timeoutEndTime = 0;
        AddWaiter(ct);
        ct->Yield();
        RemoveWaiter(ct);

        return ct->waitReturnCode;
}


/***
Visibility: protected
Parameters:
        retcode   return code to return to the waiter
Description:
        Wake one waiter with return code = retcode
***/
FThread *FWaitThing::WakeOneWaiter(int retcode) {
        TRACE_METHOD0("FWaitThing::WakeOneWaiter");
        if(firstWaiter) {
                FThread *tp=firstWaiter;
                RemoveWaiter(tp);
                tp->waitReturnCode = retcode;
                return tp;
        } else
                return 0;
}


/***
Visibility: protected
Parameters:
        retcode   return code to return to the waiters
Description:
        Wake all waiters with return code = retcode
***/
void FWaitThing::WakeAllWaiters(int retcode) {
        TRACE_METHOD0("FWaitThing::WakeAllWaiters");
        while(firstWaiter) {
                FThread *tp=firstWaiter;
                RemoveWaiter(tp);
                tp->waitReturnCode = retcode;
        }
}



/***
Visibility: protected
Description:
        Return wether or not there are more waiters
***/
int FWaitThing::MoreWaiters() const {
        TRACE_METHOD0("FWaitThing::MoreWaiters");
        return firstWaiter!=0;
}




// FExternalWaitThing ------------------------------------------------------

/***
Visibility: protected
Description:
        Tell the thread manager that an external wait thing exist
***/
FExternalWaitThing::FExternalWaitThing()
  : FWaitThing()
{
        TRACE_METHOD0("FExternalWaitThing::FExternalWaitThing");
        FThreadManager::SetExternalWaitThing(this);
}


/***
Visibility: protected
Description:
        Tell the thread manager that an external wait thing no longer exist
***/
FExternalWaitThing::~FExternalWaitThing() {
        TRACE_METHOD0("FExternalWaitThing::~FExternalWaitThing");
        FThreadManager::UnsetExternalWaitThing();
}


/***
Visibility: private (FThreadManager is friend)
Return:
        0 if any threads are waiting for the thing, non-zero otherwise.
Description:
        Return wether or not any threads are waiting for this external thing
***/
int FExternalWaitThing::AnyWaiters() const {
        return MoreWaiters();
}
