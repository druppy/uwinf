/***
Filename: server.cpp
Description:
  Implementation of FServer. This file is shared among OS/2 and win32
Host:
  Watcom 11.0
History:
  ISJ 97-09-?? - 97-10-18 Creation
***/

#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_SERVER
#include <bif.h>
#if BIFOS_==BIFOS_OS2_
#include <ipc\os2\commgc.h>
#elif BIFOS_==BIFOS_WIN32_
#include <ipc\win32\commgc.h>
#else
#error Unknown OS
#endif

FServer::FServer()
  : gcChain(0),
    gcThread(0),
    gcMutex(),
    aliveChain(0),
    aliveMutex(),
    shuttingDown(0),
    noConnections(1),
    broker(0)
{ }

FServer::~FServer() {
        delete gcThread;
}

void FServer::gcDeleteEntry(entry *e) {
        FreeThread(e->commThread);
        FreeConnection(e->connection);
        delete e;
}        

void FServer::RegisterConnection(FConnection *con, FThread *t) {
        entry *e=new entry;
        e->commThread = t;
        e->connection = con;
        aliveMutex.Request();
        e->next = aliveChain;
        aliveChain = e;
        noConnections.Reset();
        aliveMutex.Release();
        
        e->commThread->Start();
}


void FServer::DeleteMe(FThread *t) {
        aliveMutex.Request();
        entry *e=aliveChain, *prev=0;
        while(e && e->commThread != t) {
                prev = e;
                e = e->next;
        }        
        if(!e) {
                aliveMutex.Release();
                return;
        }
        if(prev)
                prev->next = e->next;
        else
                aliveChain = e->next;
        if(aliveChain==0) noConnections.Post();        
        aliveMutex.Release();        

        gcMutex.Request();
        e->next = gcChain;
        gcChain = e;
        gcMutex.Release();
        gcThread->WakeUp();
}

int FServer::Start() {
        //alloc and start GC thread
        gcThread = new CommGCThread(this);
        if(!gcThread) {
                return -1;
        }
        if(gcThread->Start()!=0) {
                return -1;
        }
        //alloc and start broker
        broker = AllocBroker();
        if(!broker) {
                return -1;
        }
        if(broker->Start()!=0) {
                return -1;
        }
        return 0;
}

void FServer::StartShutdown() {
        //stop further connections
        shuttingDown = 1;
}

void FServer::WaitShutdown() {
        //step 1: stop the broker
        //This is the responsibility of the subclass
        broker->Wait();
        {
                //2: wait for all connections to end
                noConnections.Wait();
        }
        {
                //3: stop the GC thread
                if(gcThread) {
                        gcThread->PleaseTerminate();
                        gcThread->Wait();
                }
        }
        {
                //4: clean up gc chain
                gcMutex.Request();
                while(gcChain) {
                        entry *e=gcChain;
                        gcChain = gcChain->next;
                        gcMutex.Release();
                        e->commThread->Wait();
                        FreeThread(e->commThread);
                        FreeConnection(e->connection);
                        delete e;
                        gcMutex.Request();
                }        
                gcMutex.Release();
        }
}

