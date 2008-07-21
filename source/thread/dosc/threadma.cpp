/***
Filename: threadma.cpp
Description:
  Implementation of FThreadManager
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-07 Creation
  ISJ 94-12-09 Added __WINDOWS__ stuff to support Watcom
  ISJ 96-06-02 Removed DLL support
  ISJ 96-08-22 A 5-hour debug session showed that forgetting which thread is
               the current is a bad idea.
**/

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#if BIFOS_ != BIFOS_WIN16_
#  include "../dos1632/dosyield.h"
#  include <dos.h>
#endif

#include "clktck.h"

#include <dbglog.h>



/*
 * The thread manager keeps the threads in three seperate chains:
 *   readyThreads    Threads that are ready to be run
 *   timeoutThreads  Threads that are waiting for something with a timeout
 *   waitingThreads  Threads that are waiting (with no timeout)
 * Whenever a thread should be moved from one chain to another it is the
 * responsibility of the "actor" to tell the thread manager to do so
 */
static FThread *mainThread=0;                 //pointer to the FMainThread
static FThread *readyThreads=0;               //circular list
static FThread *timeoutThreads=0;             //circular list
static FThread *waitingThreads=0;             //circular list
static FExternalWaitThing *theWaitThing=0;
#if BIFOS_ == BIFOS_WIN16_
static CATCHBUF threadManagerJump;            //thread manager context
#else
static jmp_buf threadManagerJump;             //thread manager context
#endif

/***
Visibility: public
Description:
        Return the current running thread
***/
FThread *FThreadManager::GetCurrentThread() {
        TRACE_PROC1("FThreadManager::GetCurrentThread");
        return readyThreads;
}




//enumeration ---------------------------------------------------------------
/***
Visibility: public
Return:
        A thread
Description:
        Return any thread
***/
FThread *FThreadManager::GetFirstThread() {
        TRACE_PROC1("FThreadManager::GetFirstThread");
        //Since we are called from a thread, at least one thread must be in
        //the ready chain. Thus no check for readyThreads being NULL
        return readyThreads;
}

/***
Visibility: public
Parameter:
        prev    a pointer to a thread previously returned by getfirstthread
                or getnextthread
Return:
        a thread!=prev, or NULL if no more threads
***/
FThread *FThreadManager::GetNextThread(FThread *prev) {
        TRACE_PROC1("FThreadManager::GetNextThread");
        if(!prev) {
                NOTE_TEXT1("FThreadManager::GetNextThread: prev==NULL");
                return 0;
        }

        if(findInChain(readyThreads,prev)) {
                if(prev->nextThread==readyThreads) {
                        //no more ready threads
                        if(timeoutThreads)
                                return timeoutThreads;
                        else if(waitingThreads)
                                return waitingThreads;
                        else
                                return 0;
                } else
                        return prev->nextThread;
        }
        if(findInChain(timeoutThreads,prev)) {
                if(prev->nextThread==timeoutThreads) {
                        //no more timeout
                        if(waitingThreads)
                                return waitingThreads;
                        else
                                return 0;
                } else
                        return prev->nextThread;
        }
        if(findInChain(waitingThreads,prev)) {
                if(prev->nextThread==waitingThreads) {
                        //no more waiting threads
                        return 0;
                } else
                        return prev->nextThread;
        }
        return 0;       //error
}


//win16 only: ---------------------------------------------------------------

/***
Visibility: private (FExternalWaitThing is friend)
***/
void FThreadManager::SetExternalWaitThing(FExternalWaitThing *wt) {
        theWaitThing = wt;
}

/***
Visibility: private (FExternalWaitThing is friend)
***/
void FThreadManager::UnsetExternalWaitThing() {
        theWaitThing = 0;
}



/***
Visibility: private (FThread is friend)
Parameters:
        tp      thread to insert into the ready chain
Description:
        Insert the thread into the ready chain so that the thread will
        eventually be run
Q: thread is ready
R: readyThreads' = readyThreads + {tp}
***/
int FThreadManager::AddThread(FThread *tp) {
        TRACE_PROC0("FThreadManager::AddThread");
        if(!tp) {
                ERROR("FThreadManager::AddThread: tp==NULL");
                return -1;
        }

        insertIntoChain(readyThreads,tp);
        return 0;
}


/***
Visibility: private (FThread is friend)
Parameters:
        tp      thread to remove from any chains
Description:
        Remove the thread from any threadmanager chain. This will cause the
        thread to no longer receive timeslices
***/
int FThreadManager::RemoveThread(FThread *tp) {
        TRACE_PROC0("FThreadManager::RemoveThread");
        if(!tp) {
                ERROR("FThreadManager::RemoveThread: tp==NULL");
                return -1;
        }

        if(findInChain(readyThreads,tp)) {
                removeFromChain(readyThreads,tp);
                return 0;
        }
        if(findInChain(timeoutThreads,tp)) {
                removeFromChain(timeoutThreads,tp);
                return 0;
        }
        if(findInChain(waitingThreads,tp)) {
                removeFromChain(waitingThreads,tp);
                return 0;
        }
        WARN("FThreadManager::RemoveThread: thread not found in any chains");
        return -1;
}


/***
Visibility: private (FThread and FWaitThing is friend)
Parameters:
        tp      Thread that are to be switched to another chain
Description:
        Called when a thread needs to be moved from one chain to another.
        This is usually caused by the thread waiting for a semaphore
        (or when the thread has been woken up)
***/
int FThreadManager::ChangeThread(FThread *tp) {
        TRACE_PROC0("FThreadManager::ChangeThread");
        if(!tp) {
                ERROR("FThreadManager::ChangeThread: tp==NULL");
                return -1;
        }

        //The current thread should only be changed if it is going to sleep
        if(tp==readyThreads &&
           tp->GetSubState()==FThread::rst_ready)
                return 0;

        //remove the thread from whichever chain it is in
        RemoveThread(tp);

        //and insert it into another one
        switch(tp->GetSubState()) {
                case FThread::rst_ready:
                        insertIntoChain(readyThreads,tp);
                        return 0;
                case FThread::rst_timeout:
                        insertIntoChain(timeoutThreads,tp);
                        return 0;
                case FThread::rst_waiting:
                        insertIntoChain(waitingThreads,tp);
                        return 0;
                default:
                        //internal error
                        FATAL("FThreadManager::ChangeThread: internal error");
                        return -1;
        }
}




// circular list management ------------------------------------------------
/***
Visibility: private
Parameters:
        base    reference to pointer to the first thread in the chain
        tp      pointer to thread to find in the chain
Description:
        find the thread in the chain
R: retval == tp in chain
***/
int FThreadManager::findInChain(FThread *base, FThread *tp) {
        if(!base) return 0;
        FThread *p=base;
        while(p!=tp && p->nextThread!=base)
                p=p->nextThread;
        if(p==tp)
                return 1;
        else
                return 0;
}



/***
Visibility: private
Parameters:
        base    reference to pointer to the first thread in the chain
        tp      pointer to thread to remove from the chain
Q: tp is in the chain
R: chain' = chain \ {tp}
***/
void FThreadManager::removeFromChain(FThread *&base, FThread *tp) {
        if(tp->nextThread==tp) {
                //only thread
                base=0;
        } else {
                if(base==tp)
                        base=tp->nextThread;
                tp->nextThread->prevThread = tp->prevThread;
                tp->prevThread->nextThread = tp->nextThread;
        }
        tp->nextThread = tp->prevThread = 0;
}


/***
Visibility: private
Parameters:
        base    reference to pointer to the first thread in the chain
        tp      pointer to thread to insert into the chain
R: chain' = chain + {tp}
***/
void FThreadManager::insertIntoChain(FThread *&base, FThread *tp) {
        if(base) {
                tp->nextThread = base;
                tp->prevThread = base->prevThread;
                tp->nextThread->prevThread = tp;
                tp->prevThread->nextThread = tp;
        } else {
                tp->nextThread = tp->prevThread = tp;
                base = tp;
        }
}




//--------------------------------------------------------------------------
/***
Visibility: private
Description:
        Scan through all threads waiting for an external event and move any
        woken up threads to the ready chain
        This function is an inline candidate
***/
void FThreadManager::changeWaitingThreads() {
        if(theWaitThing) theWaitThing->Check();
}




/***
Visibility: private
Description:
        Scan through all timeout threads moving any timed out threads to the
        ready chain
***/
void FThreadManager::changeTimeoutThreads() {
        if(!timeoutThreads) return;

        long currentTime = GetCurrentTickCount();

        FThread *tp=timeoutThreads;
        FThread *np=0;
        while(timeoutThreads && np!=timeoutThreads)
        {
                np=tp->nextThread;
                if(tp->timeoutEndTime<=currentTime) {
                        //Timed out - unlink and insert in ready-chain
                        //unlink:
                        removeFromChain(timeoutThreads,tp);
                        //insert in ready-chain:
                        insertIntoChain(readyThreads,tp);
                        //tell the thread that it timed out
                        tp->waitReturnCode = -1;
                }
                tp=np;
        }
}






/***
Visibility: private
Description:
        Wait until one of the threads can be run
        it (them) to the ready chain
Q: none (but usually: no ready threads exist)
R: at least one thread in the ready-chain
***/
void FThreadManager::waitForAnyThread() {
        TRACE_PROC0("FThreadManager::waitForAnyThread");
/*
This is tricky!
We can have 6 states under Windows:

                     Timeouts:
                None    Short   Long
 Threads
 waiting   None  1        2       3
 for
 message:  Some  4        5       6

Cases and what to do:
1)  No threads waiting with timeout exist and no threads are waiting for
    messages.
    The application is d-e-a-d - dead!
    The only sensible thing is to exit with an error message.
2)  At least one thread is waiting for something with a "short" timeout.
    We have have to wait until the timeout expires. While we do that we
    can tell the system that we are idle, ei. give up the rest of the
    timeslice.
3)  Only timeout-threads with a "long" timeout exist
    See 2)
4)  No timeout-threads exist and at least one thread is waiting for a
    message.
    We have to wait until a message arrives. Since a message is the only
    thing that will make the application active again, we can tell the
    system first to wake us when a message has arrived.
5)  At least one thread is waiting for a message and a timeout-thread
    with a "short" timeout exist.
    We have to wait until either a message arrives or the timeout expires
6)  At least one thread is waiting for a message and all timeout-threads
    have a "long" timeout.
    We have to wait until either a message arrives or the timeout expires
    Windows, however, does not like this. Because Windows relies on tasks
    to make blocking calls to WaitMessage() or GetMessage() in order to
    detect that the system idle, the system would never be idle if a
    thread was waiting with a 100-years-timeout. So there are two
    sollutions to this problem:
      a) "the correct one" (seen from the thread's point of view)
         Wait for either a message arrives og the timeout expires
         This is the same as 5)
      b) "the nice one" (seen from Windows' point of view)
         Wait for a message hoping for message to arrive before the
         timeout expires.
         This is the same as 4)
    Since we have never promised that a thread is woken up immidiatly
    when its timeout expires we choose b)
*/
        // Enough talk - lets do some work :-)
        // We have to compute three things:
        //  - if there exist threads waiting for messages
        //  - if there exist threads waiting with timeouts
        //  - "closest" timeout expiration

        int timeoutThreadsExist=0;
        int msgWaitingThreadsExist=0;
        long closestTimeout;
        long shortestTimeout;
        //compute the number of timeout threads and closest timeout
        if(timeoutThreads) {
                timeoutThreadsExist=1;
                //find the closest timeout
                closestTimeout=timeoutThreads->timeoutEndTime;
                for(FThread* closescan=timeoutThreads;
                    closescan->nextThread!=timeoutThreads;
                    closescan=closescan->nextThread)
                {
                        if(closescan->timeoutEndTime<closestTimeout)
                                closestTimeout=closescan->timeoutEndTime;
                }
                shortestTimeout= closestTimeout-GetCurrentTickCount();
        }

        if(theWaitThing) {
                msgWaitingThreadsExist = theWaitThing->AnyWaiters();
        }

        //now find out which case it is
        if(!msgWaitingThreadsExist) {
                //case 1, 2 and 3
                if(!timeoutThreadsExist) {
                        //case 1: dead
                        FATAL("FThreadManager::waitForAnyThread: deadlock detected");
                } else {
                        //case 2: yield-loop until timeout
                        //case 3: yield-loop until timeout
                        while(GetCurrentTickCount()<closestTimeout)
#if BIFOS_ == BIFOS_WIN16_
                                ::Yield();
#else
                                _DosYield(ClockTicks2ms(closestTimeout-GetCurrentTickCount()));
#endif
                }
        } else {
                //case 4, 5 and 6
                const long shortTimeout= ms2ClockTicks(5000);
                if(!timeoutThreadsExist) {
                        //case 4: wait for message to arrive
                        theWaitThing->Wait();
                } else if(shortestTimeout<shortTimeout) {
                        //case 5: PeekMessage-loop
                        theWaitThing->PollLoop(closestTimeout);
                } else {
                        //case 6: wait for message and hope for it to arrive
                        //        before the timeout
                        theWaitThing->Wait();
                }
        }
        changeTimeoutThreads(); //timeouts may have expired while doing all this
}




/***
Visibility: private
Description:
        Choose the the next thread to run and ensure timeout/waiting threads
        are woken up when they should.
Q: readyThreads points to the previous thread
R: readyThreads points to the next thread to run
***/
void FThreadManager::getNextThreadToRun() {
        TRACE_PROC0("FThreadManager::getNextThreadToRun");

        //Move any (now-)ready threads from the timeout- and wait-chain into
        //the ready-chain
        changeTimeoutThreads();
        changeWaitingThreads();

        //If there are no threads in the "ready" chain then wait until there
        //is one
        if(!readyThreads)
                waitForAnyThread();

        //(very) first: move the readyThreads to the next thread so that we
        //       have a "fair" selection
        readyThreads = readyThreads->nextThread;

        //BIF uses "aging"
        //That is, whenever a thread is not selected its temporary
        //priority is raised
        //When a thread is selected its temporary priority is reset
        //to its base priority

        //first: find the thread with the highest temporary priority
        //and reaise the temporary priority of the threads
        FThread *nextThreadToRun=readyThreads;
                
        FThread *maxscan=readyThreads;
        do {
                if(maxscan->tmpPriority>nextThreadToRun->tmpPriority)
                        nextThreadToRun=maxscan;
                maxscan->tmpPriority++;
                maxscan=maxscan->nextThread;
        } while(maxscan!=readyThreads);

        //second:reset the selected thread's temporary priority to its
        //       base priority
        nextThreadToRun->tmpPriority = nextThreadToRun->thePriority;

        readyThreads = nextThreadToRun;
}


/***
Visibility: private (FThread is friend)
Description:
        Called when a thread has called Yield()
        longjump to the ServiceThreads() loop.
Note:
        This function never returns
***/
void FThreadManager::switchTo() {
        TRACE_TEXT0("FThreadManager::switchTo : longjumping");
#if BIFOS_ == BIFOS_WIN16_
        Throw(threadManagerJump,1);
#else
        longjmp(threadManagerJump,1);
#endif
}


/***
Visibility: private (_BIFMain() is friend)
Description:
        Set the thread manager's main thread pointer.
        The main thread pointer is needed to detect when the main
        thread has terminated.
***/
void FThreadManager::SetMainThread(FThread *mt) {
        TRACE_PROC0("FThreadManager::SetMainThread");
        mainThread=mt;
}



/***
Visibility: private (_BIFMain() is friend)
Description:
        This is the main loop in the multitasking kernel
        Do some looping giving the threads some time
        Stop when the main thread has terminated
***/
void FThreadManager::ServiceThreads() {
        TRACE_PROC0("FThreadManager::ServiceThreads");

        //loop until the main thread has terminated
        while(mainThread->GetState()!=FThread::terminated) {
                //find next thread to run
                getNextThreadToRun();
                FThread *ct= GetCurrentThread();

#if BIFOS_ == BIFOS_WIN16_
                if(Catch(threadManagerJump)==0) {
#else
                if(setjmp(threadManagerJump)==0) {
#endif
                        //Switch to the thread
                        TRACE_FORMAT0("FThreadManager::ServiceThreads : switching to thread (%p)",ct);
                        if(ct->isStarted) {
                                //continue where left
                                TRACE_TEXT0("FThreadManager::ServiceThreads : longjumping");
#if BIFOS_ == BIFOS_WIN16_
                                Throw(ct->threadJump,1);
#else
                                longjmp(ct->threadJump,1);
#endif
                        } else {
                                //thread has not been started - kickstart it
                                TRACE_TEXT0("FThreadManager::ServiceThreads : kickstarting");
                                FThread::ThreadEntryPoint();
                        }
                } else {
                        //returned from thread
                        TRACE_TEXT0("FThreadManager::ServiceThreads : setjmp returned");
                }
        }
}

