/***
Filename: semaphor.cpp
Description:
  Implementation of a general semaphore
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
Parameters:
  intialCount  initial count of the general semaphore
Description:
  Initialize the semaphore
***/
FSemaphore::FSemaphore(long initialCount) {
        TRACE_METHOD1("FSemaphore::FSemaphore");

        if(initialCount<0) {
                //a semaphore cannot be negative
                WARN("FSemaphore::FSemaphore : initialCount<0");
                hSem=NULL;
                return;
        }

        hSem = CreateSemaphore(NULL,
                               initialCount,
                               0x7FFFFFFF,    //max count
                               NULL
                              );
        if(hSem==NULL) {
                NOTE1("FSemaphore::FSemaphore: CreateSemaphore() failed");
        }
}



/***
Visibility: public
Parameters:
Description:
  clean-up the semaphore
***/
FSemaphore::~FSemaphore() {
        TRACE_METHOD1("FSemaphore::~FSemaphore");
        CloseHandle(hSem);
}


int FSemaphore::Fail() {
        TRACE_METHOD1("FSemaphore::Fail");
        return hSem==NULL;
}





/***
Visibility: public
Return:             
  0 on success
Q: true
R:
     count' = count+1
  or count==0 and <#thread waiting>' = <#thread waiting>-1
Description:
  Increment the semaphore count or wake a waiting thread if any
***/
int FSemaphore::Signal() {
        TRACE_METHOD1("FSemaphore::Signal");

        if(ReleaseSemaphore(hSem,1,NULL))
                return 0;
        else {
                NOTE1("FSemaphore::signal: ReleaseSemaphore failed");
                return -1;
        }
}




/***
Visibility: public
Parameters:
  timeout   Maximum number of milliseconds to wait
Return:
  0 on success, non-0 otherwise
Description:
  decrement the semaphore count. Wait if it is 0
***/
int FSemaphore::Wait(long timeout) {
        TRACE_METHOD1("FSemaphore::Wait");

        DWORD rc;
        if(timeout==-1) 
                rc = WaitForSingleObject(hSem,INFINITE);
        else
                rc = WaitForSingleObject(hSem,timeout);

        if(rc==WAIT_OBJECT_0)
                return 0;
        else {
                NOTE1("FSemaphore::wait: WaitForSingleObject() failed");
                return -1;
        }
}
