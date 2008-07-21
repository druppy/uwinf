/***
Filename: mutexsem.cpp
Description:
  Implementation of FMutexSemaphore
Host:
  Watcom 10.0a
History:
  ISJ 95-07-18 Creation
***/

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>
#include "dbglog.h"


/***
Visibility: public
Description:
  Create an unnamed, private semaphore
***/
FMutexSemaphore::FMutexSemaphore() {
        TRACE_METHOD1("FMutexSemaphore::FMutexSemaphore");
        hMutex = CreateMutex(NULL, FALSE, NULL);
        if(hMutex==NULL) {
                NOTE1("FMutexSemaphore::FMutexSemaphore: CreateMutex() failed");
        }
}


/***
Visibility: public
Parameters:
  owned   If true the calling thread will own the mutex
Description:
  Create an unnamed, private semaphore
***/
FMutexSemaphore::FMutexSemaphore(int owned) {
        TRACE_METHOD1("FMutexSemaphore::FMutexSemaphore");
        hMutex = CreateMutex(NULL, owned?TRUE:FALSE, NULL);
        if(hMutex==NULL) {
                NOTE1("FMutexSemaphore::FMutexSemaphore: CreateMutex() failed");
        }
}


/***
Visibility: public
Description:
  destroy the semaphore
***/
FMutexSemaphore::~FMutexSemaphore() {
        TRACE_METHOD1("FMutexSemaphore::~FMutexSemaphore");
        CloseHandle(hMutex);
}


int FMutexSemaphore::Fail() {
        TRACE_METHOD1("FMutexSemaphore::Fail");
        return hMutex==NULL;
}



/***
Visibility: public
Parameters:
  timeout    maximum number of milliseconds to wait. if -1 then wait forever
Return:
  0 on success
***/
int FMutexSemaphore::Request(long timeout) {
        TRACE_METHOD1("FMutexSemaphore::Request");

        if(!hMutex) {
                WARN("FMutexSemaphore::Request(): requesting failed semaphore");
                return -1;
        }

        DWORD rc;
        if(timeout!=-1)
                rc = WaitForSingleObject(hMutex, (DWORD)timeout);
        else
                rc = WaitForSingleObject(hMutex, INFINITE);
        if(rc==WAIT_OBJECT_0)
                return 0;
        else {
                NOTE1("FMutexSemaphore::Request: failed");
                return -1;
        }
}



/***
Visibility: public
Return:
  0 on success
Description:
  release the mutex previously requested by the current thread
***/
int FMutexSemaphore::Release() {
        TRACE_METHOD1("FMutexSemaphore::Release");

        if(!hMutex) return -1;

        if(ReleaseMutex(hMutex))
                return 0;
        else {
                NOTE1("FMutexSemaphore::Release: failed");
                return -1;
        }
}
