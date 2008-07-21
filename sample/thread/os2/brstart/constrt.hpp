#ifndef __CONSTRT_HPP
#define __CONSTRT_HPP

#define BIFINCL_THREAD
#include <bif.h>

class ConnectionStarterThread : public FThread {
        char basePipeName[256];
public:
        ConnectionStarterThread(const char *p);
        void Go();
};

#endif

