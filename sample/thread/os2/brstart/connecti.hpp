#ifndef __CONNECTION_HPP
#define __CONNECTION_HPP

class ServerPipe;
class ConnectionThread;

struct Connection {
        ServerPipe *pipe;
        ConnectionThread *thread;
};

#endif
