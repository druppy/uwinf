/***
Filename: threadman.cpp
Description:
  Implementation of spinning semaphores
Host:
  Watcom 10.6
  VC 4.2
History:
  ISJ 96-??-?? Creation
***/
#define BIFINCL_THREAD
#define BIFINCL_SPINSEMAPHORES
#include <bif.h>

static const spin_retries=100;

///////////////////////////////////////////////////////////////////////////////
//FSpinMutex

void FSpinMutexSemaphore::Request() {
        for(unsigned spin=0; spin<spin_retries; spin++) {
                if(InterlockedExchange(&toggle,1)==0)
                        return;
        }
        InterlockedIncrement(&waiters);
        for(;;) {
                if(InterlockedExchange(&toggle,1)==0) {
                        InterlockedDecrement(&waiters);
                        return;
                }
                try_again.Wait();
                try_again.Reset();
        }
}

void FSpinMutexSemaphore::Release() {
        toggle=0;
        if(waiters!=0)
                try_again.Post();
}

///////////////////////////////////////////////////////////////////////////////
//SpinEvent

void FSpinEventSemaphore::Post() {
        modify.Request();
        if(InterlockedExchange(&toggle,1)==0)
                wakeup.Post();
        modify.Release();
}

void FSpinEventSemaphore::Reset() {
        modify.Request();
        if(InterlockedExchange(&toggle,0)==1)
                wakeup.Reset();
        modify.Release();
}

void FSpinEventSemaphore::Wait() {
        for(unsigned spin=0; spin<spin_retries; spin++) {
                if(toggle)
                        return;
        }
        wakeup.Wait();
}

///////////////////////////////////////////////////////////////////////////////
//SpinSemaphore

void FSpinSemaphore::Signal() {
        change_mutex.Request();
        InterlockedIncrement(&count);
        if(count==1)
                notzero.Post();
        change_mutex.Release();
}

void FSpinSemaphore::Wait() {
        wait_mutex.Request();
        for(unsigned spin=0; spin<spin_retries; spin++) {
                if(count>0) goto OkeyDokey;
        }
        notzero.Wait();
OkeyDokey:
        change_mutex.Request();
        InterlockedDecrement(&count);
        if(count==0)
          notzero.Reset();
        change_mutex.Release();
        wait_mutex.Release();
}
