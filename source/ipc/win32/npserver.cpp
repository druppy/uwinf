/***
Filename: npserver.cpp
Description:
  Implementation of FNPServerConnection, FNPServer
Host:
  Watcom 11.0
History:
  ISJ 97-109-?? - 97-10-18 Creation
***/

#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_SERVER
#define BIFINCL_NPSERVER
#include <bif.h>
#include <string.h>

int  FNPServerConnection::Read(void *buf, int maxbytes) {
        if(hpipe==INVALID_HANDLE_VALUE) return -1;

        BOOL b;
        DWORD bytesRead=0;
        b = ReadFile(hpipe, buf, maxbytes, &bytesRead, NULL);
        if(b)
                return (int)bytesRead;
        else {
                DisconnectNamedPipe(hpipe);
                CloseHandle(hpipe);
                hpipe=INVALID_HANDLE_VALUE;
                return -1;
        }
}

int  FNPServerConnection::Write(const void *buf, int bytes) {
        if(hpipe==INVALID_HANDLE_VALUE) return -1;

        BOOL b;
        DWORD bytesWritten=0;
        b = WriteFile(hpipe, buf, bytes, &bytesWritten, NULL);
        if(b)
                return (int)bytesWritten;
        else {
                DisconnectNamedPipe(hpipe);
                CloseHandle(hpipe);
                hpipe=INVALID_HANDLE_VALUE;
                return -1;
        }
}

void FNPServerConnection::Close() {
        if(hpipe!=INVALID_HANDLE_VALUE) {
                DisconnectNamedPipe(hpipe);
                CloseHandle(hpipe);
                hpipe = INVALID_HANDLE_VALUE;
        }
}

int FNPServerConnection::Fail() const {
        return hpipe==INVALID_HANDLE_VALUE;
}



//////////////////////////////////////////////////////////////////////////////
FNPServer_nontemplate::FNPServer_nontemplate(const char *parms)
  : FServer()
{
        if(strnicmp(parms,"\\\\",2)!=0)
        {
                strcpy(pipename,"\\\\.\\pipe\\");
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
                HANDLE hf=CreateFile(pipename,
                                     GENERIC_READ|GENERIC_WRITE,
                                     0,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL
                                    );
                if(hf!=INVALID_HANDLE_VALUE);
                        CloseHandle(hf);
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
static HANDLE createpipe(const char *pipename) {
        HANDLE hpipe;
        hpipe = CreateNamedPipe(pipename,
                                PIPE_ACCESS_DUPLEX,
                                PIPE_TYPE_BYTE|PIPE_READMODE_BYTE,
                                PIPE_UNLIMITED_INSTANCES,
                                4096,
                                4096,
                                500, //150,  //default timeout
                                NULL
                               );
        return hpipe;
}

void FNPBroker::Go() {
        HANDLE hpipe=createpipe(server->pipename);
        if(hpipe==INVALID_HANDLE_VALUE)
                return;
        while(!IsShuttingDown()) {
                if(!ConnectNamedPipe(hpipe,NULL)) {
                        //Sometimes when the client closes the pipe immidiatly
                        //connectnamedpipe() fails with error_no_data. We treat
                        //this as a connection anyway since nobody says that
                        //we have to read/write anything on a pipe.
                        //
                        //error_pipe_connected can also be the error ?!? (this got to be bug in NT!)
                        DWORD rc=GetLastError();
                        if(rc!=ERROR_NO_DATA && rc!=ERROR_PIPE_CONNECTED)
                        {
                                break;
                        }
                }
                if(IsShuttingDown()) {
                        DisconnectNamedPipe(hpipe);
                        break;
                }
                FNPServerConnection *con=new FNPServerConnection(hpipe);
                FThread *t = server->AllocThread(con);

                hpipe=createpipe(server->pipename);
                if(hpipe==INVALID_HANDLE_VALUE)
                        break;
                        
                RegisterConnection(con,t);
        }
        if(hpipe!=INVALID_HANDLE_VALUE)
                CloseHandle(hpipe);
}

