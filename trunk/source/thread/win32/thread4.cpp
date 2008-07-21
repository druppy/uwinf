/***
Filename : thread4.cpp
Description:
  Implementation of FThread class , thread start&run stuff
History:
  ISJ 1995-07-18 Creation
  ISJ 1996-07-20 Fixed a bug (Wait() didn't work)
  ISJ 1996-11-15 Added support for BC 4.52
  ISJ 1997-10-14 Fixed a rarely occurring bug
***/

#define BIFINCL_THREAD
#include <bif.h>
#include "dbglog.h"

#include <process.h>

struct FThreadActivationRecord {
        FThread *that;
        HANDLE hEvent;
};


/***
Visibility: public
Return:
  The win32 thread id of the thread.
***/
DWORD FThread::GetTID() const {
        TRACE_METHOD0("FThread::GetTID");
        return threadID;
}

/***
Visibility: public
Return:
  The handle to the thread.
***/
HANDLE FThread::GetHandle() const {
        TRACE_METHOD0("FThread::GetHandle");
        return threadHandle;
}


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
        DWORD rc = WaitForSingleObject(GetHandle(), INFINITE);
        if(rc==WAIT_OBJECT_0)
                return 0;
        else
                return -1;
}




/***
Visibility: public (but should only be called from the functions below.)
Return:
  currently always 0
Description:
  This function is called by the compiler-specific and win32 thread
  entry point. It will call go()
***/
int FThread::Run(FThreadActivationRecord *tar) {
        TRACE_METHOD0("FThread::Run");
        //copy tar to a local copy so creator can terminate any time
        FThreadActivationRecord local_tar;
        if(tar)
                local_tar=*tar;
                
        threadID = GetCurrentThreadId();
        
        _state = running;

        //we are now up and running
        if(tar)
                SetEvent(local_tar.hEvent);
        
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
//static void WatcomThreadStartHelper(void *that) {
//        TRACE_PROC0("WatcomThreadStartHelper");
//        ((FThread*)that)->Run();
//        TRACE_FORMAT0("_endthread'ing WatcomThreadStartHelper");
//        _endthread();
//}
static unsigned __stdcall WatcomThreadStartHelper(void *threadarg) {
        TRACE_PROC0("WatcomThreadStartHelper");
        FThreadActivationRecord *tar=(FThreadActivationRecord*)threadarg;
        tar->that->Run(tar);
        TRACE_FORMAT0("_endthread'ing WatcomThreadStartHelper");
        _endthread();
        return 0;
}
#elif defined(__BORLANDC__)
static void _USERENTRY BorlandThreadStartHelper(void *threadarg) {
        TRACE_PROC0("BorlandThreadStartHelper");
        FThreadActivationRecord *tar=(FThreadActivationRecord*)threadarg;
        tar->that->Run(tar);
        TRACE_FORMAT0("_endthread'ing BorlandThreadStartHelper");
        _endthread();
}
#elif (_MSC_VER >= 800)
static unsigned __stdcall MSVCThreadStartHelper(void *threadarg) {
        TRACE_PROC0("MSVCThreadStartHelper");
        FThreadActivationRecord *tar=(FThreadActivationRecord*)threadarg;
        tar->that->Run(tar);
        TRACE_FORMAT0("_endthread'ing MSVCThreadStartHelper");
        _endthread();
        return 0;
}
#else
#error Compiler not recognized
#endif


// win32 thread entry point
static DWORD WINAPI Win32ThreadStartHelper(LPVOID lpThreadParameter) {
        TRACE_PROC0("Win32ThreadStartHelper");
        FThreadActivationRecord *tar=(FThreadActivationRecord*)lpThreadParameter;
        tar->that->Run(tar);
        TRACE_FORMAT0("ExitThread'ing Win32ThreadStartHelper");

        ExitThread(0);

        return 0; //never reached
}



/***
Visibility: public
  private/protected/public/this module
Return:
  0 if the thread has been started, non-0 otherwise
Description:
  Create an OS-thread
***/
int FThread::Start() {
        TRACE_METHOD1("FThread::Start");

        if(GetState()==running) {
                WARN("Error: FThread::Start: already running");
                return -1;        //cannot start thread if it is already running
        }

        FThreadActivationRecord tar;
        tar.that = this;
        tar.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if(tar.hEvent==NULL) {
                WARN_TEXT("FThread::Start: could not create event semaphore");
                return -1;
        }

        switch(startMethod) {
                case CompilerStart: {
#if defined(__WATCOMC__)
                        //int r = _beginthread(WatcomThreadStartHelper, NULL, stackNeeded, this);
                        //if(r!=-1) {
                        //        threadID=(DWORD)r;
                        //        _state = running;
                        //}
                        _state = running;
                        threadHandle = (HANDLE)_beginthreadex(0,
                                                              stackNeeded,
                                                              WatcomThreadStartHelper,
                                                              (void*)&tar,
                                                              0,
                                                              (unsigned*)&threadID
                                                             );
                        if(threadHandle) {
                                //ok
                        }
#elif defined(__BORLANDC__)
                        _state = running;
                        threadHandle = (HANDLE)_beginthreadNT(BorlandThreadStartHelper,
                                                              stackNeeded,
                                                              (void*)&tar,
                                                              0,
                                  0,
                                                              (unsigned long*)&threadID
                                                             );
                        if(threadHandle!=(HANDLE)-1) {
                                //ok
                        }
#elif (_MSC_VER >= 800)
                        _state = running;
                        threadHandle = (HANDLE)_beginthreadex(0,
                                                              stackNeeded,
                                                              MSVCThreadStartHelper,
                                                              (void*)&tar,
                                                              0,
                                                              (unsigned*)&threadID
                                                             );
                        if(threadHandle) {
                                //ok
                        }
#else
#error Compiler not recognized
#endif
                        else {
                                _state = terminated;
                                NOTE_TEXT1("FThread::Start: compiler-specific _beginthread() failed");
                                CloseHandle(tar.hEvent);
                                return -1;
                        }
                }
                break;
                case Win32Start: {
                        _state = running;
                        threadHandle = CreateThread(NULL,
                                                    stackNeeded,
                                                    (LPTHREAD_START_ROUTINE)Win32ThreadStartHelper,
                                                    (LPVOID)&tar,
                                                    0,
                                                    &threadID
                                                   );
                        if(threadHandle==NULL) {
                                _state = terminated;
                                NOTE_TEXT1("FThread::Start: CreateThread failed");
                                CloseHandle(tar.hEvent);
                                return -1;
                        } else
                                return 0;
                }
                /*break;*/
                default:
                        return -1;
        }

        //wait for thread to initialize
        WaitForSingleObject(tar.hEvent, INFINITE);
        CloseHandle(tar.hEvent);
        
        return 0;
}
