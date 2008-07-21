/***
Filename : thread2.cpp
Description:
  Implementation of Thread class under win32, priority part only
History:
  ISJ 95-07-19 Creation (copied from OS/2 sources)
***/

#define BIFINCL_THREAD
#include <bif.h>
#include "dbglog.h"


/***
Visibility: public
Parameters:
  p  a priority in the range -15..15
Description:
  Set the priority of the thread.
***/
void FThread::SetPriority(int p) {
        TRACE_METHOD1("FThread::SetPriority");
        if(_state!=running) {
                NOTE_TEXT0("FThread::SetPriority: thread is not started");
                return;        //the priority can only be changed for running threads
        }
        if(p<-15) {
                WARN("FThread::SetPriority(): prty<-15 (p=%d)",p);
                p=-15;
        }
        if(p>15) {
                WARN("FThread::SetPriority(): prty>15 (p=%d)",p);
                p=15;
        }

        rememberedPriority = p;         // Remember the priority for use in
                                        // getPriority. This is necessary because Win32's
                                        // priority granularity is not as fine a BIF/Thread's

        if(p<-10)
                p = THREAD_PRIORITY_IDLE;
        else if(p<-1)
                p = THREAD_PRIORITY_LOWEST;
        else if(p==-1)
                p = THREAD_PRIORITY_BELOW_NORMAL;
        else if(p==0)
                p = THREAD_PRIORITY_NORMAL;
        else if(p==1)
                p = THREAD_PRIORITY_ABOVE_NORMAL;
        else if(p<=10)
                p = THREAD_PRIORITY_HIGHEST;
        else
                p = THREAD_PRIORITY_TIME_CRITICAL;

        if(!SetThreadPriority(threadHandle,p)) {
                NOTE1("FThread::SetPriority: could not set priority");
        }
}



/***
Visibility: public
Return value:
  the priority previously set with setPriority() or 0 if it has never
  been set.
***/
int FThread::GetPriority() const {
        TRACE_METHOD1("FThread::GetPriority");
        if(_state!=running) return 0;

        return rememberedPriority;
}
