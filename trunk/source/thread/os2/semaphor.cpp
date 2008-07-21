/***
Filename: semaphor.cpp
Description:
  Implementation of a general semaphore
Host:
  Watcom 9.5, Watcom 10.0
History:
  ISJ 94-05-25 Creation
  ISJ 94-06-16 Added method logging
***/

#define INCL_DOSERRORS
#define INCL_DOSMISC
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>
#include "dbglog.h"


// define COUNT_PROBING if reading a variable while others write it is safe
// on a multiprocessor system. All that we require is that either the old
// or the new value is read. If COUNT_PROBING is defined wait() will try to
// optimize out of calls to DosWaitEventSem()

//#define COUNT_PROBING



/***
Visibility: public
Parameters:
  intialCount  initial count of the general semaphore
Description:
  Initialize the semaphore
***/
FSemaphore::FSemaphore(long initialCount) {
        TRACE_METHOD1("FSemaphore::FSemaphore");

        APIRET rc;

        wait_mutex=NULL;
        not_zero_event=NULL;
        change_mutex=NULL;
        failed=0;
        count = initialCount;

        if(initialCount<0) {
                //a semaphore cannot be negative
                WARN_TEXT("FSemaphore::FSemaphore : initialCount<0");
                failed=1;
                return;
        }

        rc = DosCreateMutexSem((PSZ)0, &wait_mutex, 0, FALSE);
        if(rc!=0) {
                NOTE_TEXT0("FSemaphore::FSemaphore(), DosCreateMutexSem() failed");
                failed=1;
                return;
        }

        rc = DosCreateEventSem((PSZ)0, &not_zero_event, 0, count?TRUE:FALSE);
        if(rc!=0) {
                NOTE_TEXT0("FSemaphore::FSemaphore(), DosCreateEventSem() failed");
                DosCloseMutexSem(wait_mutex);
                failed=1;
                return;
        }

        rc = DosCreateMutexSem((PSZ)0, &change_mutex, 0, FALSE);
        if(rc!=0) {
                NOTE_TEXT0("FSemaphore::FSemaphore(), DosCreateMutexSem() failed");
                DosCloseEventSem(not_zero_event);
                DosCloseMutexSem(wait_mutex);
                failed=1;
                return;
        }
}



/***
Visibility: public
Description:
  clean-up the semaphore
***/
FSemaphore::~FSemaphore() {
        TRACE_METHOD1("FSemaphore::~FSemaphore");

        if(wait_mutex)
                DosCloseMutexSem(wait_mutex);
        if(not_zero_event)
                DosCloseEventSem(not_zero_event);
        if(change_mutex)
                DosCloseMutexSem(change_mutex);

}


int FSemaphore::Fail() {
        TRACE_METHOD1("FSemaphore::Fail");
        return failed;
}





/*
  The wait/signal are implemented as follows:

wait:
  lock(wait_mutex)                // exclude all other waits

    wait(not_zero_event)        // wait until count<>0

    lock(change_mutex)

      count--
      if count==0
        reset(not_zero_event)   // count==0 -> reset

    unlock(change_mutex)

  unlock(wait_mutex)



signal:
  lock(change_mutex)

    count++
    if count==1
      post(not_zero_event)        //tell waits that count<>0

  unlock(change_mutex)
*/




/***
Visibility: public
Return:             z
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

        if(failed) return -1;

        APIRET rc;

        rc = DosRequestMutexSem(change_mutex, SEM_INDEFINITE_WAIT);
        if(rc!=0) {
                NOTE_TEXT0("FSemaphore::Signal(), DosRequestMutexSem() failed");
                return -1;
        }

        count++;
        if(count==1) {
                rc = DosPostEventSem(not_zero_event);
                if(rc!=0) {
                        NOTE_TEXT0("FSemaphore::Signal, DosPostEventSem() failed");
                }
        }

        DosReleaseMutexSem(change_mutex);        //cannot fail in this context

        return 0;
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

        if(failed) return -1;

        APIRET rc;

        //request(wait_mutex) + wait(not_zero_event)
        //is split into two because of timeouts
        if(timeout==-1) {
                //no timeout
                rc = DosRequestMutexSem(wait_mutex, SEM_INDEFINITE_WAIT);
                if(rc!=0) {
                        NOTE_TEXT0("FSemaphore::Wait(), DosRequestMutexSem() failed");
                        return -1;
                }

#ifdef COUNT_PROBING
                if(count!=0) rc=0;
                else
#endif
                rc = DosWaitEventSem(not_zero_event, SEM_INDEFINITE_WAIT);
                if(rc!=0) {
                        NOTE_TEXT0("FSemaphore::Wait(), DosWaitEventSem() failed");
                        DosReleaseMutexSem(wait_mutex);
                        return -1;
                }
        } else {
                //timeout
                //We can do this in three ways:
                // - pass timeout to the two DosXXXXXXX functions and accept
                //   that we actually will wait twice as long as the
                //   programmer has specified.
                // - calculate how long we have waited in the request and
                //   subtract this from the timeout passed to wait.
                // - request(timeout/2) and wait(timeout/2)
                //The second approach has been chosen
                ULONG startTime;
                DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, (PVOID)&startTime, sizeof(startTime));

                //request(wait_mutex)
                rc = DosRequestMutexSem(wait_mutex, (ULONG)timeout);
                if(rc!=0) {
                        if(rc!=ERROR_SEM_TIMEOUT) {
                                NOTE_TEXT0("FSemaphore::Wait(), DosRequestMutexSem() failed");
                        }
                        return -1;
                }

                ULONG endTime;
                DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, (PVOID)&endTime, sizeof(endTime));
                ULONG timeUsed=endTime-startTime;

                //wait(not_zero_event)
#ifdef COUNT_PROBING
                if(count!=0) rc=0;
                else
#endif
                rc = DosWaitEventSem(not_zero_event, (ULONG)timeout-timeUsed);
                if(rc!=0) {
                        if(rc!=ERROR_SEM_TIMEOUT) {
                                NOTE_TEXT0("FSemaphore::Wait(), DosWaitEventSem() failed");
                        }
                        DosReleaseMutexSem(wait_mutex);
                        return -1;
                }
        }

        //lock(change_mutex)
        rc = DosRequestMutexSem(change_mutex, SEM_INDEFINITE_WAIT);
        if(rc!=0) {
                NOTE_TEXT0("FSemaphore::Wait(), DosRequestMutexSem() failed");
                DosReleaseMutexSem(wait_mutex);
                return -1;
        }

        count--;
        if(count==0) {
                ULONG iDontWantToKnow;
                DosResetEventSem(not_zero_event,&iDontWantToKnow);
        }


        DosReleaseMutexSem(change_mutex);        //cannot fail in this context


        DosReleaseMutexSem(wait_mutex);        //cannot fail either in this context


        return 0;        //phew!
}
