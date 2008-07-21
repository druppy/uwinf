/***
Filename:
  thread1.cpp
Description:
  Implementation of FThread class; thread killing
Host:
  WC 10.0 10.5 10.6, BC 4.02, VC 4.2
History:
  ISJ 95-07-18 Creation
***/

#define BIFINCL_THREAD
#include <bif.h>
#include "dbglog.h"

/***
Visibility: public
Description:
  Stop the thread. This is different from suspension because killing
  terminates the thread
***/
int FThread::Kill() {
        TRACE_METHOD1("FThread::Kill");
        if(_state!=running)
                return -1;        //no need to kill at thread that is not running
        if(TerminateThread(threadHandle,1))
                return 0;
        else
                return -1;
}
