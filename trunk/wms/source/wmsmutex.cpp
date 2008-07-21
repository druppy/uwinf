/***
filename: wmsmutex.cpp
Description:
  implementation of a mutex semaphore used throughout WMS. It is based on
  FSpinMutexSemaphore and it allows nested requests
History:
  ISJ 96-??-?? Creation
  ISJ 96-08-21 Changed to be based on a FSpinMutexSemaphore.
***/

#include "wmsmutex.hpp"

int WmsMutex::Request() {
        FThread *ct=FThreadManager::GetCurrentThread();
        if(owner==ct) {
                locks++;
                return 0;
        } else {
                //if(mutex.Request()!=0) return -1;
                mutex.Request();
                owner=ct;
                locks=1;
                return 0;
        }
}

int WmsMutex::Release() {
        if(owner==FThreadManager::GetCurrentThread()) {
                if(--locks==0) {
                        owner=0;
                        mutex.Release();
                }
                return 0;
        } else
                return -1;
}

