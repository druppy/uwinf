#ifndef __PAINT_DP_HPP
#define __PAINT_DP_HPP

#define BIFINCL_THREAD
#include <bif.h>

class PaintDispatcherThread : public FThread {
public:
        PaintDispatcherThread() : FThread() {}
        void Go();
        void PleaseTerminate();
};

#endif
