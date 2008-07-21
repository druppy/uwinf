#ifndef __RMUTEX_HPP
#define __RMUTEX_HPP

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#define BIFINCL_SPINSEMAPHORES
#include <bif.h>

class WmsMutex {
        FSpinMutexSemaphore mutex;
        FThread *owner;
        unsigned locks;
public:
        WmsMutex() : mutex(), owner(0), locks(0) {}
        ~WmsMutex() {}

        int Request();

        int Release();
};

#endif
