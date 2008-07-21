/***
Filename: thread4.cpp
Description:
  PreGo/PostGo/_PreGo/_PostGo() members of FThread
Host:
  WC 10.6
History:
  ISJ 96-08-24 Creation
***/

#define BIFINCL_THREAD
#include <bif.h>

int FThread::PreGo() {
        return _PreGo();
}

void FThread::PostGo() {
        _PostGo();
}

int FThread::_PreGo() {
        //Start() has already added the thread so this is unnecessary
        //if(FThreadManager::AddThread(this)!=0)
        //        return -1;
        return 0;
}

void FThread::_PostGo() {
        FThreadManager::RemoveThread(this);
}

