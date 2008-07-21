/***
Filename: wthread.cpp
Description:
  Implementation of FWindowThread and (internal) FWindowsMessageQueue
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-09 Creation
  ISJ 94-12-11 Added fix for Watcom bug
  ISJ 95-03-11 Made DLLable
  ISJ 95-03-15 added operator new
**/

#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>
#include <stddef.h>

#include <dbglog.h>

// FWindowsMessageQueue -----------------------------------------------------

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Note: this class is _very_ sensitive. Only one thread should be using it
 * Otherwise messages can be lost or duplicated, or wndprocs called when there
 * are no current thread
 */

/* Since Check(), Wait() and PollLoop() is called in the context of the thread
 * manager they cannot call ::GetMessage or ::PeekMessage because Windows
 * could then call the wndproc of one of the applications windows without any
 * active thread. This is handled by telling the thread (in GetMessage()) what
 * to do
 */
 
class BIFCLASS FWindowsMessageQueue : private FExternalWaitThing {
public:
        FWindowsMessageQueue();
        ~FWindowsMessageQueue();

        void GetMessage(MSG *pmsg);
protected:
        virtual void Check();
        virtual void Wait();
        virtual void PollLoop(long pollEndTime);
private:
        enum {
            block,
            peek_and_wait,
            wait
        } getmsg_op;
};

static FWindowsMessageQueue *msgQueue=0;

FWindowsMessageQueue::FWindowsMessageQueue() {
        LOG_METHOD("FWindowsMessageQueue::FWindowsMessageQueue");
        getmsg_op = peek_and_wait;
}

FWindowsMessageQueue::~FWindowsMessageQueue() {
        LOG_METHOD("FWindowsMessageQueue::~FWindowsMessageQueue");
        //nothing
}

/***
Visibility: public
Description:
        Retrieve a message from the message queue. If none is available then
        yield to other threads
***/
void FWindowsMessageQueue::GetMessage(MSG *pmsg) {
        for(;;) {
                switch(getmsg_op) {
                        case block:
                                //do a blocking ::GetMessage call
                                ::GetMessage(pmsg,NULL,0,0);
                                getmsg_op=peek_and_wait;
                                return;
                        case peek_and_wait:
                                //peek for message and wait if none
                                if(PeekMessage(pmsg,NULL,0,0,PM_REMOVE)) {
                                        //message retreived
                                        getmsg_op=peek_and_wait;
                                        return;
                                }
                                //*fall-through*
                        case wait:
                                //wait
                                GenericWait();
                }
        }
}


/***
Visibility: protected (FThreadManager is friend)
Description:
        Check for incoming messages. If a message arrives then wake the
        waiting thread (if any)
***/
void FWindowsMessageQueue::Check() {
        if(!MoreWaiters())
                return;
        DWORD qs=GetQueueStatus(QS_KEY|QS_MOUSE|QS_PAINT|QS_POSTMESSAGE|QS_SENDMESSAGE|QS_TIMER);
        if(HIWORD(qs)) {
                //something is in the queue
                getmsg_op=peek_and_wait;        //... but is doesn't necessarily mean getmessage would return a message
                WakeOneWaiter(0);
        }
}

/***
Visibility: protected (FThreadManager is friend)
Description:
        Wait for a message to arrive. This function MUST block until a
        message arrives and MUST wake a thread.
***/
void FWindowsMessageQueue::Wait() {
        TRACE_METHOD0("FWindowsMessageQueue::Wait");
        
        //block until a message arrives
        if(!MoreWaiters()) {
                FATAL("wndmsgQ::Wait: no waiters (1)");
        }

        //wake the thread and let it block
        getmsg_op=block;
        WakeOneWaiter(0);
}


static void CALLBACK TimerProc(HWND hwnd, UINT, UINT idTimer, DWORD) {
        KillTimer(hwnd,idTimer);
}

/***
Visibility: protected (FThreadManager is friend)
Parameters:
        pollEndTime     Maximum polling time
Description:
        Check for messages until pollEndTime. The function returns when
        either a threads has been woken up or pollEndTime has been reached.
        This function is used by the thread manager to wait until a thread
        can be woken up either by a message or a timeout.
***/
void FWindowsMessageQueue::PollLoop(long pollEndTime) {
        TRACE_METHOD0("FWindowsMessageQueue::PollLoop");

        SetTimer(NULL, 0, (UINT)(pollEndTime-GetTickCount()), (TIMERPROC)TimerProc);
        getmsg_op=block;
        if(MoreWaiters())
                WakeOneWaiter(0);
}



// FWindowThread ------------------------------------------------------------
FWindowThread::FWindowThread(unsigned stackNeeded)
  : FThread(stackNeeded)
{
        TRACE_METHOD1("FWindowThread::FWindowThread");

        if(!msgQueue) msgQueue = new FWindowsMessageQueue;
}

FWindowThread::~FWindowThread() {
        TRACE_METHOD1("FWindowThread::~FWindowThread");
        //note: the msgqueue is not destructed
}


/***
Visibility: protected
Description:
        Do a "normal" message-loop. The special thing about this message-
        loop is that GetMessage yields to other threads when there is no
        message.
***/
void FWindowThread::MessageLoop() {
        TRACE_METHOD1("FWindowThread::MessageLoop");
        MSG msg;
        while(GetMessage(&msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }
}


/***
Visibility: protected
Parameters:
        pmsg    where to put the retrieved message
Return:
        FALSE if the message retieved is WM_QUIT, TRUE otherwise
Description:
        Act like the normal GetMessage(). Filtering is not supported.
***/
BOOL FWindowThread::GetMessage(MSG *pmsg) {
        //logging here _really_ degrades performance
        //somehow I feel that I keep repeating myself :-)
        msgQueue->GetMessage(pmsg);
        return pmsg->message!=WM_QUIT;
}
