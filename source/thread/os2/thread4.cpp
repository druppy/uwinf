/***
Filename : thread4.cpp
Description:
  Implementation of FThread class , thread start&run stuff
Host:
  Watcom 9.5-10.6
History:
  ISJ 1994-05-14 Creation
  ISJ 1996-04-28 Introduced PreGo() and PostGo()
  ISJ 1996-06-02 Made GetTID() inline
  ISJ 1997-10-14 Fixed a rarely occurring bug
***/

#define INCL_NOPMAPI
#define BIFINCL_THREAD
#include <bif.h>
#include "dbglog.h"

#include <process.h>

struct FThreadActivationRecord {
        FThread *that;
        HEV hev;
};



/***
Visibility: public
Return:
  The OS/2 thread id of the thread.
***/
/*This function is inline
TID FThread::GetTID() const {
        TRACE_METHOD0("FThread::GetTID");
        return threadID;
}
*/

/*** 
Visibility: public
Description:
  Block the current thread until the thread terminates
Return:
  0  thread has terminated
  -1 the thread was not started or an attempt to wait for oneself was made
***/
int FThread::Wait() {
        TRACE_METHOD1("FThread::Wait");
        if(GetState()==initialized)
                return -1;      //thread not started
        TID tid=GetTID();
        APIRET rc = DosWaitThread(&tid, DCWW_WAIT);
        if(rc==0)
                return 0;
        else
                return -1;
}


/***
Visibility: public (but should only be called from the functions below.
Return:
  currently always 0
Description:
  This function is called by the compiler-specific and OS/2 thread
  entry point. It will call go()
***/
int FThread::Run(FThreadActivationRecord *tar) {
        TRACE_METHOD0("FThread::Run");
        //copy tar to a local copy so creator can terminate any time
        FThreadActivationRecord local_tar;
        if(tar)
                local_tar=*tar;
        
        //calculate thread id
        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib, &ppib);
        threadID = ptib->tib_ptib2->tib2_ultid;
        
        _state = running;

        //we are now up and running
        if(tar)
                DosPostEventSem(local_tar.hev);
        
        if(PreGo()==0) {
                Go();
                PostGo();
        }
        _state = terminated;

        return 0;
}


int FThread::PreGo() {
        return _PreGo();
}

int FThread::_PreGo() {
        if(FThreadManager::Insert(this)!=0)
                return -1;
        return 0;
}

void FThread::PostGo() {
        _PostGo();
}

void FThread::_PostGo() {
        FThreadManager::Remove(this);
}

//compiler-specific thread entry point
#if defined(__WATCOMC__)
static void WatcomThreadStartHelper(void *threadarg) {
        TRACE_PROC0("WatcomThreadStartHelper");
        FThreadActivationRecord *tar=(FThreadActivationRecord*)threadarg;
        tar->that->Run(tar);
        TRACE_TEXT0("_endthread'ing WatcomThreadStartHelper");
        _endthread();
}
#elif defined(__BORLANDC__)
#error not yet
#elif defined(__IBMCPP__)
static void _Optlink CSetThreadStartHelper(void *threadarg) {
        TRACE_PROC0("CSetThreadStartHelper");
        FThreadActivationRecord *tar=(FThreadActivationRecord*)threadarg;
        tar->that->Run(tar);
        TRACE_TEXT0("_endthread'ing CSetThreadStartHelper");
        _endthread();
}
#else
#error Compiler not recognized
#endif


// OS/2 thread entry point
static VOID APIENTRY OS2ThreadStartHelper(ULONG ThreadArg) {
        TRACE_PROC0("OS2ThreadStartHelper");
        FThreadActivationRecord *tar = (FThreadActivationRecord*)ThreadArg;
        tar->that->Run(tar);
        TRACE_TEXT0("DosExiting OS2ThreadStartHelper");

        DosExit(EXIT_THREAD,0);
}



/***
Visibility: public
Return:
  0 if the thread has been started, non-0 otherwise
Description:
  Create a OS-thread
***/
int FThread::Start() {
        TRACE_METHOD1("FThread::Start");

        if(GetState()==running) {
                WARN_TEXT("Error: FThread::Start: already running");
                return -1;        //cannot start thread if it is already running
        }

        FThreadActivationRecord tar;
        tar.that = this;
        APIRET rc=DosCreateEventSem(NULL, &tar.hev, 0, FALSE);
        if(rc) {
                WARN_TEXT("FThread::Start: could not create event semaphore");
                return -1;
        }
        
        switch(startMethod) {
                case CompilerStart: {
                        int r;
#if defined(__WATCOMC__)
                        r = _beginthread(WatcomThreadStartHelper, NULL, stackNeeded, &tar);
#elif defined(__BORLANDC__)
#error not yet
#elif defined(__IBMCPP__)
                        r = _beginthread(CSetThreadStartHelper, NULL, stackNeeded, &tar);
#else
#error Compiler not recognized
#endif
                        if(r==-1) {
                                NOTE_TEXT0("FThread::Start(), compiler-specific _beginthread() failed");
                                DosCloseEventSem(tar.hev);
                                return -1;
                        }
                }
                break;
                case OS2Start: {
                        TID tid;
                        APIRET rc;
                        rc = DosCreateThread(&tid,
                                             (PFNTHREAD)OS2ThreadStartHelper,
                                             (ULONG)&tar,
                                             0x2,        //precommit the stack
                                             stackNeeded);
                        if(rc!=0) {
                                NOTE_TEXT0("FThread::Start:DosCreateThread() failed");
                                DosCloseEventSem(tar.hev);
                                return -1;
                        }
                }
                break;
                default:
                        return -1;
        }
        
        //wait for thread to initialize
        DosWaitEventSem(tar.hev, SEM_INDEFINITE_WAIT);
        DosCloseEventSem(tar.hev);
        
        return 0;
}
