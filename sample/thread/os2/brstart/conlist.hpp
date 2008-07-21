#ifndef __CONLIST_HPP
#define __CONLIST_HPP

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

struct Connection;

class ConnectionList {
public:
        ConnectionList();
        ~ConnectionList();
        
        void AddConnection(Connection *c);
        void RemoveConnection(Connection *c);

        void StartTraversal();
        Connection *GetNext();
        void Endtraversal();

private:
        FMutexSemaphore mutex;                   //mutex to protect the list
        struct entry {
                Connection *c;
                entry *prev, *next;
        } *first;                                //double-linked list of conenctions
        entry *current;                          //traversal pointer
};

extern ConnectionList connectionList;

#endif

