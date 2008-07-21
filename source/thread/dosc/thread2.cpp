/***
Filename: thread2.cpp
Description:
  SetPriority/GetPriority implementation of FThread
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-17 Creation (moved from thread.cpp)
***/

#define BIFINCL_THREAD
#include <bif.h>

#include "dbglog.h"

/***
Visibility: protected
Description:
        Set the priority to the new priority and tell the thread manager
        that priorities are being used
***/
void FThread::SetPriority(int p) {
        TRACE_METHOD1("FThread::SetPriority");
        if(p<-15) {
                WARN("FThread::SetPriority: wanted priority(%d) < -15",p);
                p=-15;
        }
        if(p>15) {
                WARN("FThread::SetPriority: wanted priority(%d) > 15",p);
                p=15;
        }
        thePriority=p;
}

/***
Visibility: protected
Description:
        Return the current priority of the thread. Note that this is the
        base priority - not the temporary priority given by the thread manager
***/
int FThread::GetPriority() const {
        TRACE_METHOD1("FThread::GetPriority");
        return thePriority;
}




