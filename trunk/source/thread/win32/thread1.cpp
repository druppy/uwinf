/***
Filename:
  thread1.cpp
Description:
  Implementation of FThread class; ctor, dtor, OkToGo, GetState, Sleep
Host:
  Watcom 10.0a
History:
  ISJ 95-07-18 Creation (copied from OS/2 sources)
***/

#define INCL_DOSERRORS
#define BIFINCL_THREAD
#include <bif.h>
#include "dbglog.h"


/***
Visibility: public
Description:
  Initialize the thread object. Default stacksize is 8192 bytes.
  Default start method is the compiler-specific method.
***/
FThread::FThread(unsigned stackNeeded) {
        TRACE_METHOD1("FThread::FThread");
        _state = initialized;
        startMethod = CompilerStart;
        if(stackNeeded<8192)
                FThread::stackNeeded = 8192;        //minimum stack for non-PM threads
        else
                FThread::stackNeeded = stackNeeded;
        threadID = 0;
        rememberedPriority = 0;
}



/***
Visibility: public
Description:
  Clean up the thread object. If the thread is still running, kill it.
***/
FThread::~FThread() {
        TRACE_METHOD1("FThread::~FThread");
        if(_state==running) {
                NOTE_TEXT1("FThread::~FThread(): thread is still running, killing");
                Kill();
        }
        CloseHandle(threadHandle); 
#ifndef NDEBUG
        //safety clean-up
        stackNeeded = -1;
        threadID = (DWORD)-1;
        threadHandle = NULL;
#endif
}



/***
Visibility: public
Return:
  The return value signals wether or not the thread is ready to run.
Description:
  This function is used by start() among other, to ensure that the thread
  initialized ok. If we just could rely on exceptions being available...
***/
int FThread::OkToGo() {
        TRACE_METHOD1("FThread::Go");
        return 1;        // The thread base class always construct without errors
}



/***
Visibility: public
Return:
  current state of the thread (initialized/running/terminated)
***/
FThread::state FThread::GetState() const {
        TRACE_METHOD1("FThread::GetState");
        return _state;
}



/***
Visibility: protected
Description:
  Release current timeslice so that other thread can run
***/
/* This function is inline
void FThread::Yield() {
        TRACE_METHOD1("FThread::Yield");
        // Since win32 is preemptive we can do two thing:
        //  - do nothing
        //  - release the rest of the current timeslice
        // We do nothing

        //Sleep(0); release rest of timeslice
}
*/


/***
Visibility: protected
Parameters:
  ms  Number of milliseconds to sleep
Description:
  Put the thread to sleep in the number of milliseconds specified. It is not
  guaranteed that the thread will sleep exactly ms milliseconds. If the
  system is busy it may sleep longer. If the system is idle the thread will
  probably not sleep at all.
***/
void FThread::Sleep(long ms) {
        TRACE_METHOD1("FThread::Sleep");
        ::Sleep((DWORD)ms);
}

