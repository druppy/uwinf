/***
Filename: npserver.cpp
Description:
  Implementation of FNPServer
Host:
  Watcom 11.0
History:
  ISJ 97-09-?? - 97-10-18 Creation
***/

#define INCL_DOSNMPIPES
#define INCL_DOSFILEMGR
#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_SERVER
#define BIFINCL_NPSERVER
#include <bif.h>
#include <string.h>

int  FNPServerConnection::Read(void *buf, int maxbytes) {
        if(hpipe==(HPIPE)-1) return -1;
        APIRET rc;
        ULONG bytesRead;
        rc = DosRead(hpipe,buf,maxbytes,&bytesRead);
        if(rc) {
                DosDisConnectNPipe(hpipe);
                DosClose(hpipe);
                hpipe = (HPIPE)-1;
                return -1;
        } else
                return (int)bytesRead;
}

int  FNPServerConnection::Write(const void *buf, int bytes) {
        if(hpipe==(HPIPE)-1) return -1;
        APIRET rc;
        ULONG bytesWritten;
        rc = DosWrite(hpipe,(PVOID)buf,bytes,&bytesWritten);
        if(rc) {
                DosDisConnectNPipe(hpipe);
                DosClose(hpipe);
                hpipe = (HPIPE)-1;
                return -1;
        } else
                return (int)bytesWritten;
}

void FNPServerConnection::Close() {
        if(hpipe==(HPIPE)-1) return;
        DosDisConnectNPipe(hpipe);
        DosClose(hpipe);
        hpipe = (HPIPE)-1;
}

int FNPServerConnection::Fail() const {
        return hpipe==(HPIPE)-1;
}



//////////////////////////////////////////////////////////////////////////////
FNPServer_nontemplate::FNPServer_nontemplate(const char *parms)
  : FServer()
{
        if(strnicmp(parms,"\\\\",2)!=0 &&
           strnicmp(parms,"\\pipe",5)!=0)
        {
                strcpy(pipename,"\\pipe\\");
                while(*parms=='\\') parms++;
                strcat(pipename,parms);
        } else
                strcpy(pipename,parms);
}

void FNPServer_nontemplate::FreeConnection(FConnection *con) {
        delete con;
}

FBroker *FNPServer_nontemplate::AllocBroker() {
        return new FNPBroker(this);
}

void FNPServer_nontemplate::StartShutdown() {
        FServer::StartShutdown();
}

void FNPServer_nontemplate::WaitShutdown() {
        {
                //1: make sure the broker thread wakes up so it notices the
                //shutdown flag
                //We wake the thread by making a connection
                APIRET rc;
                HFILE hf;
                ULONG action;
                rc = DosOpen(pipename,&hf,&action,0,0,
                             OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                             OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE,
                             0
                            );
                if(rc==0) DosClose(hf);
        }
        //The remaining steps are up to the FServer class
        FServer::WaitShutdown();
}

void FNPServer_nontemplate::ForceShutdown() {
        shuttingDown = 1;
        
        //run through all live connections and disconnect them
        aliveMutex.Request();
        for(entry *e=aliveChain; e; e=e->next)
                e->connection->Close();
        aliveMutex.Release();

        //Now, every thread should terminate within milliseconds
        WaitShutdown();
}


//////////////////////////////////////////////////////////////////////////////
static HPIPE createpipe(const char *pipename) {
        APIRET rc;
        HPIPE hpipe;
        rc = DosCreateNPipe(pipename,
                            &hpipe,
                            NP_ACCESS_DUPLEX,
                            NP_WAIT|NP_TYPE_BYTE|NP_READMODE_BYTE|255,
                            4096,
                            4096,
                            0
                           );
        if(rc)
                return (HPIPE)-1;
        else
                return hpipe;
}

void FNPBroker::Go() {
        /* We have to keep a pipe instance available at all times
         * Otherwise the connection thread may be scheduled and close the pipe
         * before we can create another instance. This would result in clients
         * being rejected because of "invalid pipe name"
         */
        HPIPE hpipe=createpipe(server->pipename);
        if(hpipe==(HPIPE)-1)
                return;
        while(!IsShuttingDown()) {
                APIRET rc;
                rc = DosConnectNPipe(hpipe);
                if(rc)
                        break;
                if(IsShuttingDown()) {
                        DosDisConnectNPipe(hpipe);
                        break;
                }
                //alloc stuff for connection
                FNPServerConnection *con=new FNPServerConnection(hpipe);
                FThread *t = server->AllocThread(con);

                //create pipe _before_ connection thread is started
                hpipe = createpipe(server->pipename);
                if(hpipe==(HPIPE)-1)
                        break;
                //start the connection thread
                RegisterConnection(con,t);
        }
        if(hpipe!=(HPIPE)-1)
                DosClose(hpipe);
}

