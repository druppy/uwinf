/***
Filename: wthread.cpp
Description:
  Implementation of FWindowThread
Host:
  Watcom 9.5-10.5 OS/2
History:
  ISJ 94-06-16 Creation
**/

#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define INCL_WINMESSAGEMGR
#include <bif.h>
#include "dbglog.h"


/***
Visibility: public
Parameters:
  queueSize  the size of the queue, 0 if it should have the default size
Description:
***/
FWindowThread::FWindowThread(unsigned stackNeeded, int queueSize)
  : FThread(stackNeeded)
{
        TRACE_METHOD1("FWindowThread::FWindowThread");

        rememberedQS = queueSize;
        if(stackNeeded<16384)
                FThread::stackNeeded = 16384;        //PM threads need a larger stack
        //safety init:
        hab = 0;
        hmq = 0;
}


/***
Visibility: public
Description:
  Nothing needs to be done
  enumerated.
***/
FWindowThread::~FWindowThread() {
        TRACE_METHOD1("FWindowThread::~FWindowThread");
}




int FWindowThread::PreGo() {
        if(FThread::PreGo()!=0) return -1;
        if(_PreGo()!=0) { FThread::PostGo(); return -1; }
        return 0;
}

int FWindowThread::_PreGo() {
        hab = WinInitialize(0);
        if(!hab) return -1;
        hmq = WinCreateMsgQueue(hab,rememberedQS);
        if(!hmq) {
                WinTerminate(hab);
                hab=0;
                return -1;
        }
        return 0;
}

void FWindowThread::PostGo() {
        _PostGo();
        FThread::PostGo();
}

void FWindowThread::_PostGo() {
        WinDestroyMsgQueue(hmq); hmq=0;
        WinTerminate(hab); hab=0;
}


/***
Visibility: protected
Description:
  Do a standard messageloop
***/
void FWindowThread::MessageLoop() {
        TRACE_METHOD1("FWindowThread::MessageLoop");

        QMSG qmsg;
        while(WinGetMsg(hab,&qmsg,0,0,0)) {
                WinDispatchMsg(hab,&qmsg);
        }
}
