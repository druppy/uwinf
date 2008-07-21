#include "sysqueue.hpp"

SystemEventQueue::SystemEventQueue()
  : notEmpty(0),
    notFull(queueSize),
    queueModify(),
    msgs(0)
{
}

SystemEventQueue::~SystemEventQueue() {
}


int SystemEventQueue::get(SystemEvent *se, int timeout) {
        if(timeout==0 && msgs==0) return -1;
        if(notEmpty.Wait(timeout)==0) {
                queueModify.Request();
                *se = q[0];
                for(int i=0; i<msgs-1; i++)
                        q[i] = q[i+1];
                msgs--;
                if(msgs==0) notEmpty.Reset();
                notFull.Signal();
                queueModify.Release();
                return 0;
        } else
                return -1;
}

int SystemEventQueue::put(const SystemEvent *se, int timeout) {
        if(timeout==0 && msgs==queueSize)
                return -1;
        if(timeout==-1 && msgs==queueSize && se->type==SystemEvent::mouse) {
                //try to compact the queue
                queueModify.Request();
                for(int i=0; i<msgs-1; i++) {
                        if(q[i].type==SystemEvent::mouse) {
                                if(q[i+1].type==SystemEvent::mouse &&
                                   q[i].keyboardShiftState==q[i+1].keyboardShiftState &&
                                   q[i].u.sme.buttonState==q[i+1].u.sme.buttonState)
                                {
                                        for(int j=i; j<msgs-1; j++)
                                                q[j]=q[j+1];
                                        msgs--;
                                        notFull.Signal();
                                }
                        }
                }
                queueModify.Release();
        }
        if(notFull.Wait(timeout)==0) {
                queueModify.Request();
                for(int i=msgs; i>0 && q[i-1].timestamp>se->timestamp; i--)
                        q[i]=q[i-1];
                q[i] = *se;
                msgs++;
                if(msgs==1) notEmpty.Post();
                queueModify.Release();
                return 0;
        } else
                return -2;
}

