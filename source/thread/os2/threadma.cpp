/***
Filename: threadman.cpp
Description:
  Implementation of FThreadManager
Host:
  Watcom 9.5-11.0
History:
  ISJ 94-05-25 Creation
  ISJ 96-08-16 Optimized thread registration from a linked list to an array
               This makes expensive calls to DosGetInfoBlocks() unnecessary in
               FThreadManager::GetCurrentThread
  ISJ 96-08-21 Changed mutex into a spinmutex
  ISJ 97-03-14 Made WC11 shut up
**/

#define INCL_NOPMAPI
#define BIFINCL_THREAD
#define BIFINCL_SPINSEMAPHORES
#define INCL_DOSERRORS
#include <bif.h>
#include "dbglog.h"
#include <os2.h>
#include <new.h>

typedef struct _trr { // Thread registration record
        FThread *thread;
        TID tid;
        PVOID stackbottom;
        PVOID stacktop;
} trr;

//OS/2 allow a maximum of 4096 threads system-wide
#define MAXTHREADS 4096
static trr tr_threadReg[MAXTHREADS];
static unsigned tr_highwatermark=0;
#ifdef __WATCOMC__
//We know sizeof is used on a class which contains compiler-generated
//information and we don't care
#pragma warning 549 5
#endif
static char tr_mutex_buf[sizeof(FSpinMutexSemaphore)];
static FSpinMutexSemaphore *tr_mutex;

/***
Visibility: private (called by FThread)
Parameters:
  tp  pointer to the thread to insert
Return:
  0 on success
Description:
  Insert the thread into the threadmangers list of threads, so it can be
  enumerated.
Note: Must be called in the context of the thread to register
***/
int FThreadManager::Insert(FThread *tp) {
        TRACE_PROC0("FThreadManager::Insert");
        if(!tp) {
                //null pointers are rejected
                ERROR_TEXT("FThreadManager::Insert : tp==NULL");
                return -1;
        }

        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib,&ppib);
        tr_mutex->Request();

        for(unsigned i=0; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].thread==0) break;
        }
        if(i>=tr_highwatermark)
                tr_highwatermark++;

        tr_threadReg[i].thread = tp;
        tr_threadReg[i].tid = ptib->tib_ptib2->tib2_ultid;
        tr_threadReg[i].stackbottom = ptib->tib_pstack;
        tr_threadReg[i].stacktop = ptib->tib_pstacklimit;

        tr_mutex->Release();
        return 0;
}




/***
Visibility: private (called from thread)
Parameters:
  tp  pointer to the thread to remove from the list
Return:
  0 on success
Description:
  Remove the thread from the list
***/
int FThreadManager::Remove(FThread *tp) {
        TRACE_PROC0("FThreadManager::Remove");
        //remove the thread from the list
        if(!tp) {
                ERROR_TEXT("FThreadManager::Remove : tp==NULL");
                return -1;
        }

        tr_mutex->Request();

        int found=0;
        for(unsigned i=0; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].thread==tp) {
                        tr_threadReg[i].thread = 0;
                        tr_threadReg[i].tid = 0;
                        tr_threadReg[i].stacktop = 0;
                        tr_threadReg[i].stackbottom = 0;
                        found=1;
                        break;
                }
        }

        tr_mutex->Release();

        return !found;
}






/***
Visibility: private (called from _BIFMain() )
Return:
  0 on success
Description:
  Initialize the threadmanager(-class). Create the mutex and memory pool
***/
int FThreadManager::Init() {
        TRACE_PROC0("FThreadManager::Init");

        tr_mutex = new(tr_mutex_buf) FSpinMutexSemaphore;
        return 0;
}



/***
Visibility: private
Description:
  clean-up the threadmanager
***/
void FThreadManager::Done() {
        TRACE_PROC0("FThreadManager::Done");
        tr_mutex->~FSpinMutexSemaphore();
}




/***
Visibility: public
Return:
  a pointer to the first thread in the list, 0 if no threads are listed
***/
FThread *FThreadManager::GetFirstThread() {
        TRACE_PROC1("FThreadManager::GetFirstThread");

        tr_mutex->Request();

        FThread *tp;
        for(unsigned i=0; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].thread) {
                        tp = tr_threadReg[i].thread;
                        break;
                }
        }

        tr_mutex->Release();

        return tp;
}



/***
Visibility: public
Parameters:
  prev   pointer to the previous thread
Return:
  A pointer the the thread's successor in the list, 0 if none is listed
***/
FThread *FThreadManager::GetNextThread(FThread *prev) {
        TRACE_PROC1("FThreadManager::GetNextThread");

        if(!prev) {
                WARN_TEXT("FThreadManager::GetNextThread : prev==NULL");
                return 0;
        }


        tr_mutex->Request();

        for(unsigned i=0; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].thread==prev)
                        break;
        }
        FThread *tp=0;
        i++;
        for(; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].thread) {
                        tp = tr_threadReg[i].thread;
                        break;
                }
        }

        tr_mutex->Release();

        return tp;
}



/***
Visibility: public
Return:
  A pointer to the current thread. null if it could not be found.
Description:
  Figure out the current thread by finding the the stack pointer in the thread
  registration array for it in the list.
***/
FThread *FThreadManager::GetCurrentThread() {
        TRACE_PROC1("FThreadManager::GetCurrentThread");

        tr_mutex->Request();

        FThread *tp=0;
        int dummy;
        for(unsigned i=0; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].stackbottom <= &dummy &&
                   &dummy <= tr_threadReg[i].stacktop) {
                        tp = tr_threadReg[i].thread;
                        break;
                }
        }

        tr_mutex->Release();

        return tp;
}

