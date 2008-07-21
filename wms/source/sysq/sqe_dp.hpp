#ifndef __SQE_DP_HPP
#define __SQE_DP_HPP

#define BIFINCL_THREAD
#include <bif.h>

class SystemQueueEventDispatcher : public FThread {
        volatile int please_terminate;
public:
        SystemQueueEventDispatcher();

        void Go();

        void PleaseTerminate();
};

#endif

