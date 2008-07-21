#ifndef __CONTHRD_HPP
#define __CONTHRD_HPP

#define BIFINCL_THREAD
#include <bif.h>

class ServerIOPipe;
class Job;

class ConnectionThread : public FThread {
        ServerIOPipe *pipe;
        int connection_closed;
public:
        ConnectionThread(ServerIOPipe *p);
        ~ConnectionThread();
        void Go();
protected:
        void handleMessage(void *com_buf, Job *job);
};

#endif

