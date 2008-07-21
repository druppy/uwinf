/***
Filename: threadman.cpp
Description:
  Implementation of FThreadManager
Host:
  Watcom 10.0a - 10.6
  Borland 4.52
  VC 4.2
History:
  ISJ 95-07-18 Creation
  ISJ 96-08-26 Changed thread registrations to be an array instead of a linked
               list, which eliminates the need of BIFMmemoryPool class
               Changed critical section into a spinmutex, since sampling under
               NT 3.51 showed that critical sections are just as slow as mutex
               semaphores
  ISJ 96-11-15 Added support for BC 4.52
***/

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#define BIFINCL_SPINSEMAPHORES
#include <bif.h>
#include "dbglog.h"

typedef struct _trr { // Thread registration record
        FThread *thread;
        DWORD tid;
} trr;

//The number of threads under WindowsNT is limited by available memory only
//BIF/Thread impose a limit of max 4096 threads
#define MAXTHREADS 4096
static trr tr_threadReg[MAXTHREADS];
static unsigned tr_highwatermark=0;
#ifdef __WATCOMC__
#pragma warning 549 9
#endif
static char tr_mutex_buf[sizeof(FSpinMutexSemaphore)];
static FSpinMutexSemaphore *tr_mutex;

#if defined(__BORLANDC__) || defined(_MSC_VER)
inline void *operator new(size_t, void *p) { return p; }
#endif

/***
Visibility: private (called by FThread)
Parameters:
  tp  pointer to the thread to insert
Return:
  0 on success
Description:
  Insert the thread into the threadmanager's list of threads, so it can be
  enumerated.
***/
int FThreadManager::Insert(FThread *tp) {
        TRACE_PROC0("FThreadManager::Insert");
        if(!tp) {
                //null pointers are rejected
                ERROR("FThreadManager::Insert : tp==NULL");
                return -1;
        }

        DWORD tid = GetCurrentThreadId();
        
        tr_mutex->Request();

        unsigned i;
        for(i=0; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].thread==0) break;
        }
        if(i>=tr_highwatermark)
                tr_highwatermark++;

        tr_threadReg[i].thread = tp;
        tr_threadReg[i].tid = tid;

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
                ERROR("FThreadManager::Remove : tp==NULL");
                return -1;
        }

        tr_mutex->Request();

        int found=0;
        for(unsigned i=0; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].thread==tp) {
                        tr_threadReg[i].thread = 0;
                        tr_threadReg[i].tid = 0;
                        found=1;
                        if(i == tr_highwatermark-1)
                                tr_highwatermark--;
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
  Initialize the threadmanager(-class). Create the spinmutex
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
                WARN("FThreadManager::GetNextThread : prev==NULL");
                return 0;
        }

        tr_mutex->Request();

        unsigned i;
        for(i=0; i<tr_highwatermark; i++) {
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
  Figure out the current thread by finding the current TID and search
  for it in the list.
***/
FThread *FThreadManager::GetCurrentThread() {
        TRACE_PROC1("FThreadManager::GetCurrentThread");

        tr_mutex->Request();

        FThread *tp=0;
        DWORD currentTID=GetCurrentThreadId();
        for(unsigned i=0; i<tr_highwatermark; i++) {
                if(tr_threadReg[i].tid == currentTID) {
                tp = tr_threadReg[i].thread;
                        break;
                }
        }

        tr_mutex->Release();

        return tp;
}

