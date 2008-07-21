#define INCL_DOSQUEUES
#define INCL_ERRORS
#include "job.hpp"
#include "smthrd.hpp"
#include "joblist.hpp"
#include <stdio.h>
#include "brserver.hpp"

extern FMutexSemaphore sessionstart_mutex;

//declare the structure put by OS/2 in the termination queue
#pragma pack(0);
struct TerminationStruct {
        USHORT sessionID;
        USHORT resultCode;
};
#pragma pack();


/* The wakeup event semaphore is necessary because we need to wait on two 
 * things: a result posted in the queue or the server shutting down
 * the HEV handle in the event semaphore is passed to DosReadQueue(), so 
 * when it is posted by OS/2 or PleaseTerminate() the thread will wake up
 * the event semaphore must be shared, and since BIF/Thread doesn't support 
 * this we have to create the event semaphore with the OS/2 API.
 */
SMThread::SMThread(HQUEUE h) 
  : FThread(),
    hqueue(h), 
    please_terminate(0) 
{
        DosCreateEventSem(NULL, //"\\SEM32\\brserver\\smthrd\\wakeup",
                          &hevWakeup,
                          DC_SEM_SHARED,
                          FALSE
                         );
}

SMThread::~SMThread() {
        DosCloseEventSem(hevWakeup);
}

void SMThread::Go() {
        for(;;) {
                REQUESTDATA requestData;
                ULONG cbData;
                PVOID pbuf;
                BYTE priority;
                APIRET rc;

                //reset
                ULONG dontCare;
                DosResetEventSem(hevWakeup,&dontCare);
                if(please_terminate) break;

                //issue a read queue to setup associate the semaphore with the queue
                rc=DosPeekQueue(hqueue,
                                &requestData,
                                &cbData,
                                &pbuf,
                                0,
                                DCWW_NOWAIT,
                                &priority,
                                hevWakeup
                               );
                if(rc!=0 && rc!=ERROR_QUE_EMPTY) {
                        //peek error ?!? - panic!
                        stdio_mutex.Request();
                        fprintf(stderr,"SMThread: read error from result queue (rc=%ld)\n",(unsigned long)rc);
                        terminate_server=1;
                        stdio_mutex.Release();
                        break;
                }

                //wait for something to happen
                DosWaitEventSem(hevWakeup,SEM_INDEFINITE_WAIT);
                DosResetEventSem(hevWakeup,&dontCare);
                if(please_terminate) break;
                //read the queue to retrive the element
                rc=DosReadQueue(hqueue,
                                &requestData,
                                &cbData,
                                &pbuf,
                                0,
                                DCWW_NOWAIT,
                                &priority,
                                hevWakeup
                               );
                if(rc!=0) {
                        //read error ?!? - panic!
                        stdio_mutex.Request();
                        fprintf(stderr,"SMThread: read error from result queue (rc=%ld)\n",(unsigned long)rc);
                        terminate_server=1;
                        stdio_mutex.Release();
                        break;
                }
                if(requestData.ulData==0) {
                        //the element is a termination notification
                        TerminationStruct *ts = ((TerminationStruct*)pbuf);
                        USHORT sessionId = ts->sessionID;

                        //if the session terminated very fast, we could 
                        //potentially recive the termination notification
                        //before JobStart() has a chance to add the job to the
                        //job list. This is avoided by requesting a releasing
                        //the session mutex.
                        sessionstart_mutex.Request();
                        sessionstart_mutex.Release();

                        //find the job in our joblist
                        jobList.StartTraversal();
                        Job *j=jobList.GetNext();
                        while(j) {
                                if(j->idSession==sessionId) {
                                        j->resultCode = ts->resultCode;
                                        j->terminated = 1;
                                        j->event_terminated.Post();
                                        break;
                                }
                                j=jobList.GetNext();
                        }
                        jobList.EndTraversal();
                }
                DosFreeMem(pbuf);
        }
}

void SMThread::PleaseTerminate() {
        please_terminate=1;
        DosPostEventSem(hevWakeup);
}

