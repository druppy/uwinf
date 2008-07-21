#ifndef __SERVER_H
#define __SERVER_H

class FBroker;

class FServer {
private:
        FBroker *broker;
protected:
        FServer();
        ~FServer();
        
        struct entry {
                entry *next;
                FThread *commThread;
                FConnection *connection;
        };
        
        //chain of existing connections
        entry *aliveChain;
        FMutexSemaphore aliveMutex;     //to protect list
        void RegisterConnection(FConnection *con, FThread *t);
        FEventSemaphore noConnections;
        friend class FBroker;

        //chain of terminated connections but not deleted yet
        entry *gcChain;
        FMutexSemaphore gcMutex;        //to protect list
        friend class CommGCThread;
        CommGCThread *gcThread;
        void gcDeleteEntry(entry *e);

        int shuttingDown;

        virtual void FreeThread(FThread *t) =0;
        virtual void FreeConnection(FConnection *con) =0;
        virtual FBroker *AllocBroker() =0;
public:
        int Start();
        virtual void StartShutdown() =0;
        virtual void WaitShutdown() =0;
        virtual void ForceShutdown() =0;
        void DeleteMe(FThread *t);
};


class FBroker : public FThread {
        FServer *server;
protected:
        FBroker(FServer *s)
          : FThread(),
            server(s)
        {}

        void RegisterConnection(FConnection *con, FThread *t)
          { server->RegisterConnection(con,t); }
        int IsShuttingDown() const
          { return server->shuttingDown; }
};

#endif
