/***
Filename: mutexsem.cpp
Description:
  Implementation of FMutexSemaphore
Host:
  Watcom 9.5  OS/2
History:
  ISJ 94-05-25 Creation
***/

#define INCL_NOPMAPI
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
        APIRET rc;
        rc = DosCreateMutexSem((PSZ)0, &hmtx, 0, FALSE);
        if(rc!=0) {
                NOTE_TEXT0("FMutexSemaphore::FMutexSemaphore, DosCreateMutexSem() failed");
                hmtx=0;
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
        APIRET rc;
        rc = DosCreateMutexSem((PSZ)0, &hmtx, 0, owned?TRUE:FALSE);
        if(rc!=0) {
                NOTE_TEXT0("FMutexSemaphore::FMutexSemaphore, DosCreateMutexSem() failed");
                hmtx=0;
        }
}


/***
Visibility: public
Description:
  destroy the semaphore
***/
FMutexSemaphore::~FMutexSemaphore() {
        TRACE_METHOD1("FMutexSemaphore::~FMutexSemaphore");
        if(!hmtx)
                DosCloseMutexSem(hmtx);
}


int FMutexSemaphore::Fail() {
        TRACE_METHOD1("FMutexSemaphore::Fail");
        return hmtx!=0;
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

        if(!hmtx) {
                WARN_TEXT("FMutexSemaphore::Request(): requesting failed semaphore");
                return -1;
        }

        APIRET rc;
        if(timeout!=-1)
                rc = DosRequestMutexSem(hmtx, (ULONG)timeout);
        else
                rc = DosRequestMutexSem(hmtx, SEM_INDEFINITE_WAIT);
        if(rc==0)
                return 0;
        else {
                NOTE_TEXT0("FMutexSemaphore::Request(), DosRequestMutexSem() failed");
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

        if(!hmtx) return -1;

        APIRET rc = DosReleaseMutexSem(hmtx);
        if(rc==0)
                return 0;
        else {
                NOTE_TEXT0("FMutexSemaphore::Release(), DosReleaseMutexSem() failed");
                return -1;
        }
}
