/***
Filename: wthread.cpp
Description:
  Implementation of FWindowThread for WMS
Host:
  Watcom 9.5 OS/2, WC10.5
History:
  ISJ 94-06-16 Creation
  ISJ 96-04-20 Copied to this file and modified for WMS
  ISJ 96-04-26 Modified for PreGo() stuff
**/

#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
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
                FThread::stackNeeded = 16384;   //WMS threads need a larger stack
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
        hab = WmsInitialize(0);
        if(!hab) return -1;
        hmq = WmsCreateMsgQueue(hab,rememberedQS,0);
        if(!hmq) {
                WmsTerminate(hab);
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
        WmsDestroyMsgQueue(hmq); hmq=0;
        WmsTerminate(hab); hab=0;
}



/***
Visibility: protected
Description:
  Do a standard messageloop
***/
void FWindowThread::MessageLoop() {
        TRACE_METHOD1("FWindowThread::MessageLoop");

        WmsQMSG qmsg;
        while(WmsGetMsg(hmq,&qmsg)) {
                WmsDispatchMsg(&qmsg);
        }
}
