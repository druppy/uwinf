#define WMSINCL_TIMER
#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#define WMSINCL_BW
#define WMSINCL_MSGS
#define WMSINCL_TIMESTAMP
#include <wms.hpp>

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

#include "wmstimer.hpp"


struct WmsTimer {
        TimeStamp start;
        uint32    interval;
        TimeStamp nextShot;
        Bool      oneShot;
        WmsHWND   hwnd;
        uint32    id;
};

#define MAXTIMERS 64
static WmsTimer t[MAXTIMERS];
static unsigned timers=0;
static FMutexSemaphore timerMutex;
class TimerLock {
public:
        TimerLock() { timerMutex.Request(); }
        ~TimerLock() { timerMutex.Release(); }
};

static FEventSemaphore wakeup;
static Bool please_terminate=False;
void WmsTimerThread::Go() {
        WmsHAB hab=WmsInitialize(0);

        while(!please_terminate) {
                long nearestTimer;
                if(timers>0) {
                        TimerLock tl;
                        nearestTimer=t[0].nextShot;
                        for(unsigned i=1; i<timers; i++)
                                if(t[i].nextShot<nearestTimer)
                                        nearestTimer=t[i].nextShot;
                        nearestTimer -= WmsConvertTicks2Ms(WmsQueryTimeStamp());
                } else
                        nearestTimer=-1;
                wakeup.Wait(nearestTimer);
                wakeup.Reset();
                if(please_terminate) break;
                TimerLock tl;
                for(unsigned i=0; i<timers; i++) {
                        if(!WmsIsWindow(t[i].hwnd)) {
                                for(unsigned j=i; j<timers-1; j++)
                                        t[j]=t[j+1];
                                timers--;
                        } else if(t[i].nextShot<=WmsConvertTicks2Ms(WmsQueryTimeStamp())) {
                                WmsPostMsg(t[i].hwnd, WM_TIMER, MPFROMUINT32(t[i].id), 0);
                                if(t[i].oneShot) {
                                        for(unsigned j=i; j<timers-1; j++)
                                                t[j]=t[j+1];
                                        timers--;
                                } else {
                                        t[i].nextShot = WmsConvertTicks2Ms(WmsQueryTimeStamp())+t[i].interval;
                                }
                        }
                }
        }
        WmsTerminate(hab);
}

void WmsTimerThread::PleaseTerminate() {
        please_terminate=True;
        wakeup.Post();
}

Bool WmsStartTimer(WmsHWND hwnd, uint32 id, uint32 interval, Bool oneShot) {
        TimerLock tl;
        //
        if(timers==MAXTIMERS)
                return False;
        //
        if(!WmsIsWindow(hwnd))
                return False;
        //ensure it's not already there
        for(unsigned i=0; i<timers; i++)
                if(t[i].hwnd==hwnd && t[i].id==id)
                        return False;
        if(interval<100)
                interval=100; //intervals less than 100ms is not allowed
        t[timers].start = WmsConvertTicks2Ms(WmsQueryTimeStamp());
        t[timers].interval = interval;
        t[timers].nextShot = t[timers].start+t[timers].interval;
        t[timers].oneShot = oneShot;
        t[timers].hwnd = hwnd;
        t[timers].id = id;
        timers++;

        wakeup.Post();

        return True;
}

Bool WmsStopTimer(WmsHWND hwnd, uint32 id) {
        TimerLock tl;
        for(unsigned i=0; i<timers; i++) {
                if(t[i].hwnd==hwnd && t[i].id==id) {
                        for(;i<timers-1; i++)
                                t[i]=t[i+1];
                        timers--;
                        return True;
                }
        }
        return False;
}

