/***
Filename: commgc.cpp
Description:
  Implementation of garbage collection thread
Host:
  Watcom 11.0
History:
  ISJ 97-109-?? - 97-10-18 Creation
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

void CommGCThread::Go() {
        while(!pleaseTerminate) {
                //wait until something has to be done
                workToDo.Wait();
                workToDo.Reset();
                
                server->gcMutex.Request();
                while(server->gcChain) {
                        FServer::entry *e = server->gcChain;
                        server->gcChain = e->next;
                        server->gcMutex.Release();
                        //wait for thread to finish
                        e->commThread->Wait();
                        server->gcDeleteEntry(e);
                        server->gcMutex.Request();
                }
                server->gcMutex.Release();
        }
}

