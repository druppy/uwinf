#ifndef __MSGQUEUE_HPP
#define __MSGQUEUE_HPP

// QMSG structure
struct WmsQMSG {
        WmsHWND         hwnd;
        WmsMSG          msg;
        WmsMPARAM       mp1;
        WmsMPARAM       mp2;
        TimeStamp       time;
        FPoint          pt;
        uint32          reserved;
};

//flags for WmsPeekMsg
#define PM_NOREMOVE     0x0000
#define PM_REMOVE       0x0001

//flags from WmsQueryQueueStatus
#define QQS_POSTMSG     0x0001
#define QQS_SENDMSG     0x0002
#define QQS_KEYBOARD    0x0010
#define QQS_MOUSE       0x0020
#define QQS_INPUT       0x0040
#define QQS_TIMER       0x0100
#define QQS_PAINT       0x0200

typedef WmsMRESULT (*WMSDISPATCHFUNC)(const WmsQMSG *qmsg);

WmsHMQ          WmsCreateMsgQueue(WmsHAB hab, int queueSize, WMSDISPATCHFUNC dp);
Bool            WmsDestroyMsgQueue(WmsHMQ hmq);
Bool            WmsPostMsg(WmsHWND hwnd, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2);
Bool            WmsPostQueueMsg(WmsHMQ hmq, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2);
WmsMRESULT      WmsSendMsg(WmsHWND hwnd, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2);
WmsMRESULT      WmsSendMsgTimeout(WmsHWND hwnd, WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2, int timeout);
Bool            WmsGetMsg(WmsHMQ hmq, WmsQMSG *qmsg);
void            WmsWaitMsg(WmsHMQ hmq);
Bool            WmsPeekMsg(WmsHMQ hmq, WmsQMSG *qmsg, WmsHWND hwndFilter, WmsMSG msgFirst, WmsMSG msgLast, uint32 fl);
uint32          WmsQueryQueueStatus(WmsHMQ hmq);
Bool            WmsInsendMsg(WmsHMQ hmq);
Bool            WmsQueryMsgPos(WmsHMQ hmq, FPoint *p);
Bool            WmsQueryMsgTime(WmsHMQ hmq, TimeStamp *ts);
WmsMRESULT      WmsDispatchMsg(const WmsQMSG *qmsg);
Bool            WmsCancelShutdown(WmsHMQ hmq, Bool always);
WMSDISPATCHFUNC WmsQueryMsgQueueDispatchFunction(WmsHMQ hmq);
Bool            WmsSetMsgQueueDispatchFunction(WmsHMQ hmq, WMSDISPATCHFUNC dp);

#endif

