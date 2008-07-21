#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#define WMSINCL_BW
#define WMSINCL_MSGS
#define WMSINCL_TIMESTAMP
#include <wms.hpp>

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include "../anchor/anchor.hpp"
#include "../wmsmutex.hpp"
#include "hmqmap.hpp"

#include "../sysq/hwmouse.hpp"

struct MsgQueue {
        //ownership
        unsigned long const tid;
        AnchorBlock * const anchorBlock;
        WmsHAB        const hab;
        //identification
        WmsHMQ              hmq;

        //structure for messages sent by other threads
        struct ReplyStruct {
                FEventSemaphore replyReady;
                WmsMRESULT reply;
        };
        //internal queue message
        struct InternalQMSG {
                WmsQMSG qmsg;
                enum { post,send } op;
                ReplyStruct *reply;
        };
        enum { defaultQueueSize=50 };
        int           queueSize;
        InternalQMSG *msg;
        int           msgs;

        //last msg retrieved (for insendmsg, msgpos, msgtime)
        InternalQMSG last_msg;

        //flags for WM_QUIT
        Bool accept_wm_quit;
        Bool wm_quit_posted;

        //dispatch function
        WMSDISPATCHFUNC dfunc;

        FSemaphore notFull,notEmpty;
        WmsMutex queueModify;

        MsgQueue(AnchorBlock *ab, int qs, WMSDISPATCHFUNC dp);
        ~MsgQueue();
};

MsgQueue::MsgQueue(AnchorBlock *ab, int qs, WMSDISPATCHFUNC dp)
  : tid(ab->tid),
    anchorBlock(ab),
    hab(ab->hab),
    queueSize(qs>10?qs:defaultQueueSize),
    msg(new InternalQMSG[queueSize]),
    msgs(0),
    notFull(queueSize),
    notEmpty(0),
    queueModify(),
    accept_wm_quit(True),
    wm_quit_posted(False),
    dfunc(dp)
{
}

MsgQueue::~MsgQueue() {
        delete[] msg;
}

class QueueModifyLock {
        MsgQueue *mq;
public:
        QueueModifyLock(MsgQueue *m) : mq(m) { mq->queueModify.Request(); }
        ~QueueModifyLock() { mq->queueModify.Release(); }
};

static WmsMutex MsgQueueMutex;
class MsgQueueLock {
public:
        MsgQueueLock() { MsgQueueMutex.Request(); }
        ~MsgQueueLock() { MsgQueueMutex.Release(); }
};

static WmsHMQ hmq_sequencer=0;

static HmqMap hmqMap;

WmsHMQ WmsCreateMsgQueue(WmsHAB hab, int queueSize, WMSDISPATCHFUNC dp) {
        if(WmsQueryAnchorBlockThread(hab) != FThreadManager::GetCurrentThread()->GetTID())
                return 0;
        if(WmsQueryAnchorBlockMsgQueue(hab)!=0)
                return 0;

        MsgQueue *mq=new MsgQueue(WmsQueryAnchorBlockAnchorBlock_internal(hab),queueSize,dp);
        if(!mq)
                return 0;

        MsgQueueLock mql;
        hmq_sequencer = (WmsHMQ) (((uint32)hmq_sequencer)+1);
        mq->hmq = hmq_sequencer;
        hmqMap.Insert(mq->hmq,mq);
        WmsSetAnchorBlockMsgQueue_internal(hab,mq->hmq);
        return mq->hmq;
}

Bool WmsDestroyMsgQueue(WmsHMQ hmq) {
        MsgQueueMutex.Request();
        MsgQueue *mq=hmqMap.Find(hmq);
        if((!mq) || mq->tid!=FThreadManager::GetCurrentThread()->GetTID()) {
                MsgQueueMutex.Release();
                return False;
        } else {
                //destruction is ok
                hmqMap.Remove(hmq);     //remove from map
                MsgQueueMutex.Release(); //noone can find the msgq, so we can release the mutex
                //release all waiters
                for(int i=0; i<mq->msgs; i++) {
                        if(mq->msg[i].op == MsgQueue::InternalQMSG::send) {
                                mq->msg[i].reply->reply=0;
                                mq->msg[i].reply->replyReady.Post();
                        }
                }
                //zap the msgq
                delete mq;

                return True;
        }
}

static void WmsStampQMSG_internal(WmsQMSG *qmsg) {
        int c,r;
        hwmouse_internal.QueryPosition(&c,&r);
        qmsg->pt.x = c;
        qmsg->pt.y = r;
        qmsg->time = WmsQueryTimeStamp();
}

static void compactQueue(MsgQueue *mq) {
        //compact a queue
        // - multiple WM_PAINT to the same window is reduced to one
        // - sequences of WM_MOUSEMOVE to the same window is reduced to one
        MsgQueue::InternalQMSG *m = mq->msg;
        for(unsigned b=0; b<mq->msgs; b++) {
                if(m[b].qmsg.msg==WM_PAINT) {
                        for(unsigned i=b+1; i<mq->msgs; i++) {
                                if(m[i].qmsg.msg==WM_PAINT && m[i].qmsg.hwnd==m[b].qmsg.hwnd && m[i].op==MsgQueue::InternalQMSG::post) {
                                        for(unsigned j=i; j<mq->msgs-1; j++)
                                                m[j] = m[j+1];
                                        mq->msgs--;
                                        mq->notFull.Signal();
                                        mq->notEmpty.Wait();
                                        i--;
                                }
                        }
                }
                if(m[b].qmsg.msg==WM_MOUSEMOVE && m[b].op==MsgQueue::InternalQMSG::post) {
                        for(unsigned i=b+1; i<mq->msgs && m[i].qmsg.msg==WM_MOUSEMOVE && m[i].qmsg.hwnd==m[b].qmsg.hwnd && m[i].qmsg.mp2==m[b].qmsg.mp2 && m[i].op==MsgQueue::InternalQMSG::post; i++) {
                                for(unsigned j=b; j<mq->msgs-1; j++)
                                        m[j] = m[j+1];
                                mq->msgs--;
                                mq->notFull.Signal();
                                mq->notEmpty.Wait();
                                i--;
                        }
                }
        }
}

static Bool WmsPostMsg_internal(WmsHMQ hmq, WmsHWND hwnd, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2)
{
        MsgQueue::InternalQMSG iqmsg;
        iqmsg.qmsg.hwnd=hwnd;
        iqmsg.qmsg.msg=msg;
        iqmsg.qmsg.mp1=mp1;
        iqmsg.qmsg.mp2=mp2;
        iqmsg.op=MsgQueue::InternalQMSG::post;
        iqmsg.reply=0;
        WmsStampQMSG_internal(&iqmsg.qmsg);

        MsgQueueLock mql;
        MsgQueue *mq=hmqMap.Find(hmq);
        if(!mq)
                return False;
        if(msg==WM_QUIT) {
                QueueModifyLock qml(mq);
                if((!mq->accept_wm_quit) || mq->wm_quit_posted) {
                        return True;
                }
        }
        if(mq->notFull.Wait(1) == 0) {
                QueueModifyLock qml(mq);
                mq->msg[mq->msgs++] = iqmsg;
                mq->notEmpty.Signal();
                if(mq->msgs==mq->queueSize) {
                        compactQueue(mq);
                }
                return True;
        } else
                return False;
}


Bool WmsPostMsg(WmsHWND hwnd, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2) {
        return WmsPostMsg_internal(WmsQueryWindowMsgQueue(hwnd),
                                   hwnd,
                                   msg,
                                   mp1,
                                   mp2
                                  );
}


Bool WmsPostQueueMsg(WmsHMQ hmq, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2) {
        return WmsPostMsg_internal(hmq,
                                   0,
                                   msg,
                                   mp1,
                                   mp2
                                  );
}

WmsMRESULT WmsSendMsg(WmsHWND hwnd, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2) {
        return WmsSendMsgTimeout(hwnd,msg,mp1,mp2,-1);
}



WmsMRESULT WmsSendMsgTimeout(WmsHWND hwnd, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2, int timeout) {
        MsgQueue::InternalQMSG iqmsg;
        iqmsg.qmsg.hwnd=hwnd;
        iqmsg.qmsg.msg=msg;
        iqmsg.qmsg.mp1=mp1;
        iqmsg.qmsg.mp2=mp2;
        iqmsg.op=MsgQueue::InternalQMSG::send;
        WmsStampQMSG_internal(&iqmsg.qmsg);
        WmsHMQ hmq=WmsQueryWindowMsgQueue(hwnd);
        MsgQueueMutex.Request();
        MsgQueue *mq=hmqMap.Find(hmq);
        if(!mq) {
                MsgQueueMutex.Release();
                return 0;
        }
        if(mq->tid == FThreadManager::GetCurrentThread()->GetTID()) {
                MsgQueueMutex.Release();
                return WmsDispatchMsg(&iqmsg.qmsg);        //bypass queue
        }
        if(mq->notFull.Wait(timeout)==0) {
                mq->queueModify.Request();
                MsgQueue::ReplyStruct rs;
                iqmsg.reply=&rs;
                mq->msg[mq->msgs++] = iqmsg;
                mq->queueModify.Release();
                mq->notEmpty.Signal();

                MsgQueueMutex.Release();
                rs.replyReady.Wait();
                return rs.reply;
        } else {
                MsgQueueMutex.Release();
                return 0;
        }
}


Bool WmsGetMsg(WmsHMQ hmq, WmsQMSG *qmsg) {
        for(;;) {
                MsgQueue *mq;
                {
                        MsgQueueLock mql;
                        mq=hmqMap.Find(hmq);
                        if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                                return False;
                        if(mq->wm_quit_posted)
                                return False;
                }
                mq->notEmpty.Wait();
                MsgQueue::InternalQMSG iqmsg;
                {
                        QueueModifyLock qml(mq);
                        //todo: prioritize
                        iqmsg = mq->msg[0];
                        for(int i=0; i<mq->msgs-1; i++)
                                mq->msg[i] = mq->msg[i+1];
                        mq->msgs--;
                }
                mq->notFull.Signal();
                if(iqmsg.op==MsgQueue::InternalQMSG::send) {
                        iqmsg.reply->reply = WmsDispatchMsg(&iqmsg.qmsg);
                        iqmsg.reply->replyReady.Post();
                } else {
                        *qmsg = iqmsg.qmsg;
                        mq->last_msg = iqmsg;
                        return qmsg->msg==WM_QUIT?False:True;
                }
        }
}

void WmsWaitMsg(WmsHMQ hmq) {
        MsgQueue *mq;
        {
                MsgQueueLock mql;
                mq=hmqMap.Find(hmq);
                if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                        return;
        }
        mq->notEmpty.Wait();
        mq->notEmpty.Signal();
}

Bool WmsPeekMsg(WmsHMQ hmq, WmsQMSG *qmsg, WmsHWND hwndFilter, WmsMSG msgFirst, WmsMSG msgLast, uint32 fl) {
        MsgQueue *mq;
        {
                MsgQueueLock mql;
                mq=hmqMap.Find(hmq);
                if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                        return False;
                if(mq->wm_quit_posted)
                        return False;
        }

        QueueModifyLock qml(mq);
        for(int m=0; m<mq->msgs; m++) {
                if((mq->msg[m].op==MsgQueue::InternalQMSG::post) &&
                   (hwndFilter==0 || hwndFilter==mq->msg[m].qmsg.hwnd) &&
                   ((msgFirst==0 && msgLast==0) || (msgFirst<=mq->msg[m].qmsg.msg && msgLast>=mq->msg[m].qmsg.msg))
                  )
                {
                        *qmsg = mq->msg[m].qmsg;
                        if(fl&PM_REMOVE) {
                                for(int i=m; i<mq->msgs-1; i++)
                                        mq->msg[i] = mq->msg[i+1];
                                mq->msgs--;
                                mq->notEmpty.Wait(); //will never block
                                mq->notFull.Signal();
                        }
                        return True;
                }
        }
        return False;
}


uint32 WmsQueryQueueStatus(WmsHMQ hmq) {
        MsgQueue *mq;
        {
                MsgQueueLock mql;
                mq=hmqMap.Find(hmq);
                if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                        return 0;
        }

        uint32 fl=0;
        QueueModifyLock qml(mq);
        for(int m=0; m<mq->msgs; m++) {
                if(mq->msg[m].op==MsgQueue::InternalQMSG::post)
                        fl |= QQS_POSTMSG;
                else
                        fl |= QQS_SENDMSG;
                WmsMSG msg=mq->msg[m].qmsg.msg;
                if(msg==WM_KEYBOARD)
                        fl |= QQS_KEYBOARD | QQS_INPUT;
                else if(msg>=WM_MOUSEFIRST && msg<=WM_MOUSELAST)
                        fl |= QQS_MOUSE | QQS_INPUT;
                else if (msg==WM_TIMER)
                        fl |= QQS_TIMER;
                else if(msg==WM_PAINT)
                        fl |= QQS_PAINT;
        }
        return fl;
}

Bool WmsInsendMsg(WmsHMQ hmq) {
        MsgQueue *mq;
        {
                MsgQueueLock mql;
                mq=hmqMap.Find(hmq);
                if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                        return False;
        }
        if(mq->last_msg.reply != 0)
                return True;
        else
                return False;
}

Bool WmsQueryMsgPos(WmsHMQ hmq, FPoint *p) {
        MsgQueue *mq;
        {
                MsgQueueLock mql;
                mq=hmqMap.Find(hmq);
                if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                        return False;
        }
        *p = mq->last_msg.qmsg.pt;
        return True;
}

Bool WmsQueryMsgTime(WmsHMQ hmq, TimeStamp *ts) {
        MsgQueue *mq;
        {
                MsgQueueLock mql;
                mq=hmqMap.Find(hmq);
                if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                        return False;
        }
        *ts = mq->last_msg.qmsg.time;
        return True;
}

WmsMRESULT WmsDispatchMsg(const WmsQMSG *qmsg) {
        if(qmsg->hwnd) {
                MsgQueueMutex.Request();
                MsgQueue *mq=hmqMap.Find(WmsQueryWindowMsgQueue(qmsg->hwnd));
                if((!mq) || mq->tid != FThreadManager::GetCurrentThread()->GetTID()) {
                        MsgQueueMutex.Release();
                        return 0;
                } else {
                        WMSDISPATCHFUNC dp=mq->dfunc;
                        MsgQueueMutex.Release();
                        if(!dp) return 0;
                        WmsMRESULT mr;
                        mr = (*dp)(qmsg);
                        return mr;
                }
        } else {
                //special processing?
                return 0;
        }
}


Bool WmsCancelShutdown(WmsHMQ hmq, Bool always) {
        MsgQueue *mq;
        {
                MsgQueueLock mql;
                mq=hmqMap.Find(hmq);
                if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                        return False;
        }
        QueueModifyLock qml(mq);
        mq->wm_quit_posted=False;
        mq->accept_wm_quit = (Bool)!always;
        //remove all wm_quit from the queue
        for(int i=0; i<mq->msgs; i++) {
                if(mq->msg[i].qmsg.msg==WM_QUIT) {
                        mq->notEmpty.Wait(); //will never block
                        for(int j=i; j<mq->msgs-1; j++)
                                mq->msg[j]=mq->msg[j+1];
                }
        }
        return True;
}

WMSDISPATCHFUNC WmsQueryMsgQueueDispatchFunction(WmsHMQ hmq) {
        MsgQueueLock mql;
        MsgQueue *mq=hmqMap.Find(hmq);
        if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                return 0;
        return mq->dfunc;
}

Bool WmsSetMsgQueueDispatchFunction(WmsHMQ hmq, WMSDISPATCHFUNC dp) {
        MsgQueueLock mql;
        MsgQueue *mq=hmqMap.Find(hmq);
        if((!mq) || (mq->tid!=FThreadManager::GetCurrentThread()->GetTID()))
                return False;
        mq->dfunc=dp;
        return True;
}


