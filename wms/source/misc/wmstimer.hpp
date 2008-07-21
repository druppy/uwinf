#ifndef __TIMER_HPP
#define __TIMER_HPP

#define BIFINCL_THREAD
#include <bif.h>

class WmsTimerThread : public FThread {
public:
        WmsTimerThread() : FThread() {}
        void Go();
        void PleaseTerminate();
};

#endif
