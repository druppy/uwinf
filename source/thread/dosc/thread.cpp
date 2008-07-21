/***
Filename: thread.cpp
Description:
  Implementation of FThread
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-07 Creation
  ISJ 94-07-11 Split up SP computaion into smaller bits to support the
               Symantic compiler
               Changed register-manipulation in ThreadEntryPoint() into
               inline assembler to support the Symantic compiler
  ISJ 94-07-17 Added stack checking
               Moved suspend/resume and setpriority/getpriority into
               seperate modules.
  ISJ 94-12-09 Added __WINDOWS__ stuff to support Watcom
               Made Get_SP() function to support Watcom, and some
               ifdefs in ThreadEntryPoint
  ISJ 94-12-13 Implemented unusable mutexes (if an owner thread dies without releasing it)
  ISJ 95-04-25 Made Get_SP work with Symantec++
  ISJ 96-04-16 Moved stack stuff into its own module
  ISJ 96-05-31 Implemented TIDs
**/

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include "clktck.h"

#include "dbglog.h"

//define STACK_IN_DS to allocate thread stack i data segment
//This is only possible when using the static link library version of BIF/Thread
//#define STACK_IN_DS

void *allocateThreadStack(unsigned stackSize);
void freeThreadStack(void *stack);

static unsigned TID_sequencer=0;

/***
Visibility: public
Description:
        Initialize all the thread variables (quite a lot)
***/
FThread::FThread(unsigned stackNeeded) {
        TRACE_METHOD1("FThread::FThread");
        //stack:
        stack = 0;
        FThread::stackNeeded=stackNeeded;
        //states:
        theState=initialized;
        isStarted=0;
        suspendCount=0;
        isSleeping=0; timeoutEndTime=0;
        //priority:
        thePriority=0; tmpPriority=0;
        //thread manager data
        prevThread = nextThread = 0;
        //wait thing
        waitsFor = 0;
        nextWaiter=prevWaiter=0;
        firstOwnedMutex=0;
        //TID:
        TID = ++TID_sequencer;
}


/***
Visibility: public
Description:
        Unlink the thread from all relations and free the stack (if any)
***/
FThread::~FThread() {
        TRACE_METHOD1("FThread::~FThread");
        if(GetState()==running) {
                NOTE_TEXT1("FThread::~FThread: thread is still running, killing");
                terminateThread();
        }
        if(stack) {
                //free stack
                freeThreadStack(stack);
                stack=0;
        }
}


/***
Visibility: public
Description:
        Return wether or not the thread will accept to be started
***/
int FThread::OkToGo() {
        TRACE_METHOD1("FThread::OkToGo()");
        return 1;       //by default the thread doesn't mind being started
}




/***
Visibility: public
Description:
        Return the current state of the thread (init, run, terminated)
        This function is a inline candidate.
***/
FThread::state FThread::GetState() const {
        TRACE_METHOD1("FThread::GetState");
        return theState;
}



/***
Visibility: protected
Description:
        Block the thread for the specified number of milliseconds
        Only the thread itself should call Sleep()
***/
void FThread::Sleep(long ms) {
        TRACE_METHOD1("FThread::Sleep");
        if(ms<=0) return;
        timeoutEndTime = GetCurrentTickCount() + ms2ClockTicks(ms);
        isSleeping=1;
        FThreadManager::ChangeThread(this);
        Yield();
}





/***
Visibility: public
Return:
        0 if the thread has been suspended
Description:
        Wait for the thread to terminate.
***/
int FThread::Wait() {
        TRACE_METHOD1("FThread::Wait");
        if(FThreadManager::GetCurrentThread()==this) {
                NOTE_TEXT1("FThread::Wait: the thread attempted to wait for itself");
                return -1;
        }
        switch(GetState()) {
                case initialized:
                        NOTE_TEXT1("FThread::Wait: attempt to wait for non-started thread");
                        return -1;
                case running:
                        GenericWait();
                        return 0;
                case terminated:
                        return 0;
                default:
                        FATAL("FThread::Wait: internal error");
                        return -1;
        }
}



/***
Visibility: public
Return:
        0 if the thread has been killed
Description:
        Kill the thread. Suicide is illegal
***/
int FThread::Kill() {
        TRACE_METHOD1("FThread::Kill");
        if(FThreadManager::GetCurrentThread()==this) {
                //suicide is illegal
                WARN_TEXT("FThread::Kill: Attempt to commit suicide");
                return -1;
        }
        if(GetState()==initialized) {
                WARN_TEXT("FThread::Kill: Attempt to kill a non-started thread");
                return -1;
        }

        terminateThread();
        return 0;
}



//dos-common stuff: (the funny part) ------------------------------------------
/***
Visibility: private
Description:
        Unlink the thread from all relations (thread manager, wait things, ...)
***/
void FThread::terminateThread() {
        TRACE_METHOD0("FThread::terminateThread");
        //thread is terminated
        theState=terminated;
        //remove thread from any chain
        FThreadManager::RemoveThread(this);
        //dead threads cannot wait for anything
        if(waitsFor)
                waitsFor->RemoveWaiter(this);
        //wake all threads waiting for this thread to terminate
        WakeAllWaiters(0);
        //Mark all owned mutex'es as unusable and wake waiting threads with an error code
        for(FMutexSemaphore *ms=firstOwnedMutex; ms; ms=ms->nextOwned) {
                ms->failed=1;
                ms->WakeAllWaiters(-3);
        }
}


/***
Visibility: public
Return:
        0 if the thread has been started
Description:
        Start the thread by allocating stack and adding the thread to the
        thread manager's ready chain
Note:
        A thread cannot be started twice
        Stack is first reclaimed when the thread object is deleted
***/
int FThread::Start() {
        TRACE_METHOD1("FThread::Start");
        if(GetState()!=initialized) {
                WARN_TEXT("FThread::Start: attempt to start already started thread");
                return -1;
        }

        if(!OkToGo()) {
                NOTE_TEXT1("FThread::Start: OkToGo() refused starting the thread");
                return -1;
        }

        //allocate stack (if not already done by a subclass)
        if(!stack) {
                stack = allocateThreadStack(stackNeeded);
                if(!stack) {
                        ERROR("FThread::Start: cannot allocate stack");
                        return -1;
                }
        }

        if(FThreadManager::AddThread(this)) {
                ERROR("FThread::Start: thread manager could not accept the thread");
                return -1;
        }

        theState=running;

        return 0;
}


/***
Visibility: private (FThreadManager is friend)
Description:
        Return the sub-state of the thread
        This function is used by the thread manager to determine which chain
        to put the thread in
***/
FThread::runningSubState FThread::GetSubState() const {
        TRACE_METHOD0("FThread::GetSubState");

        if(suspendCount)
                return rst_waiting;     //thread is suspended
        if(isSleeping)
                return rst_timeout;     //sleeping
        if(waitsFor) {
                //waiting for something
                if(timeoutEndTime)
                        return rst_timeout;
                else
                        return rst_waiting;
        }
        return rst_ready;
}
