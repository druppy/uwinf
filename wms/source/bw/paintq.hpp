#ifndef __PAINTDP_HPP
#define __PAINTDP_HPP

#include "wms.hpp"
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#define BIFINCL_SPINSEMAPHORES
#include <bif.h>

class PaintQueue {
        enum { MAXHWNDSINQUEUE=100 };
        WmsHWND e[MAXHWNDSINQUEUE];
        volatile int head,tail;
        FSpinEventSemaphore notEmpty;
        volatile Bool please_terminate;
public:
        PaintQueue()
          : head(0),tail(0),please_terminate(False)
          {}

        void Insert(WmsHWND hwnd);
        void Remove(WmsHWND hwnd);

        WmsHWND Get();

        void PleaseTerminate();
};

#endif
