/***
Filename: eventsem.cpp
Description:
  Implementation of FEventSem
Host:
  Watcom 9.5-10.6
History:
  ISJ 94-05-25 Creation
  ISJ 94-06-16 Added method logging
***/

#define BIFINCL_THREAD
#define INCL_DOSERRORS
#define BIFINCL_SEMAPHORES
#include <bif.h>
#include "dbglog.h"

/***
Visibility: public
Description:
  Create an unnamed, private event semaphore
***/
FEventSemaphore::FEventSemaphore() {
        TRACE_METHOD1("FEventSemaphore::FEventSemaphore");

        APIRET rc;
        rc = DosCreateEventSem((PSZ)0, &hev, 0, FALSE);
        if(rc!=0) {
                NOTE_TEXT0("FEventSemaphore::FEventSemaphore(), DosCreateEventSem() failed");
                hev = 0;
        }
}



/***
Visibility: public
Parameters:
  posted   if true the event is initially posted
Description:
  Create an unnamed, private event semaphore
***/
FEventSemaphore::FEventSemaphore(int posted) {
        TRACE_METHOD1("FEventSemaphore::FEventSemaphore");

        APIRET rc;
        rc = DosCreateEventSem((PSZ)0, &hev, 0, posted?TRUE:FALSE);
        if(rc!=0) {
                NOTE_TEXT0("FEventSemaphore::FEventSemaphore(), DosCreateEventSem() failed");
                hev = 0;
        }
}



/***
Visibility: public
Description:
  Destroy the event semaphore
***/
FEventSemaphore::~FEventSemaphore() {
        TRACE_METHOD1("FEventSemaphore::~FEventSemaphore");
        if(hev)
                DosCloseEventSem(hev);
}


int FEventSemaphore::Fail() {
        TRACE_METHOD1("FEventSemaphore::Fail");
        return hev==NULL;
}



/***
Visibility: public
Return:
  0 on success
Description:
  Post the event. It is not an error to post an event already posted
***/
int FEventSemaphore::Post() {
        TRACE_METHOD1("FEventSemaphore::Post");

        if(!hev) {
                WARN_TEXT("FEventSemaphore::post: posting a failed event semaphore");
                return -1;
        }

        APIRET rc = DosPostEventSem(hev);
        // Since we just want to put the eventsem into a posted state
        // it is not an error if it is already posted
        if(rc==0 ||
           rc==ERROR_ALREADY_POSTED ||
           rc==ERROR_TOO_MANY_POSTS)
                return 0;
        else {
                NOTE_TEXT0("FEventSemaphore::Post(), DosPostEventSem() failed");
                return -1;
        }
}



/***
Visibility: public
Parameters:
  timeout   maximum number of milliseconds to wait
Return:
  0 on success
Description:
  Wait for the event to be posted
***/
int FEventSemaphore::Wait(long timeout) {
        TRACE_METHOD1("FEventSemaphore::Wait");

        if(!hev) {
                WARN_TEXT("FEventSemaphore::Wait: wating for failed semaphore");
                return -1;
        }

        APIRET rc;
        if(timeout==-1)
                rc = DosWaitEventSem(hev, SEM_INDEFINITE_WAIT);
        else
                rc = DosWaitEventSem(hev, (ULONG)timeout);
        if(rc==0)
                return 0;
        else {
                NOTE_TEXT0("FEventSemaphore::Wait(), DosWaitEventSem() failed");
                return -1;
        }
}



/***
Visibility: public
Return:
  0 on success
Description:
  reset the event. It is not an error to reset an event that is not posted.
***/
int FEventSemaphore::Reset() {
        TRACE_METHOD1("FEventSemaphore::Reset");

        if(!hev) {
                WARN_TEXT("FEventSemaphore::Reset: resetting failed semaphore");
                return -1;
        }

        ULONG postCount;        //we dont care
        APIRET rc = DosResetEventSem(hev,&postCount);
        // Since we just want to put the event semaphore into non-posted
        // stated (what OS/2 calls 'set'), it is not an error if it is
        // already non-posted ('set')
        if(rc==0 ||
           rc==ERROR_ALREADY_RESET)
                return 0;
        else {
                NOTE_TEXT0("FEventSemaphore::Reset(), DosResetEventSem() failed");
                return -1;
        }
}
