/***
Filename: tcpserv.cpp
Description:
  Implementation of FTCPServerConnection, FTCPServer
Host:
  Watcom 11.0
History:
  ISJ 97-109-?? - 97-10-18 Creation
***/

#define INCL_DOSNMPIPES
#define INCL_DOSFILEMGR
#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_SERVER
#define BIFINCL_TCPSERVER
#include <bif.h>

#include <winsock.h>

#include <string.h>
#include <stdlib.h>

int FTCPServerConnection::Read(void *buf, int maxbytes) {
        if(s==INVALID_SOCKET) return -1;
        int bytesRead=recv(s,(char*)buf,maxbytes,0);
        if(bytesRead<0) {
                closesocket(s);
                s=INVALID_SOCKET;
                return -1;
        } else
                return bytesRead;
}

int FTCPServerConnection::Write(const void *buf, int bytes) {
        if(s==INVALID_SOCKET) return -1;
        int bytesleft=bytes;
        while(bytesleft>0) {
                int bytesWritten=send(s,(char*)buf,bytes,0);
                if(bytesWritten<0) {
                        closesocket(s); s=INVALID_SOCKET;
                        return -1;
                }
                bytesleft -= bytesWritten;
                buf = (void*)((char*)buf+bytesWritten);
        }
        return bytes;
}

void FTCPServerConnection::Close() {
        if(s==INVALID_SOCKET) return;
        closesocket(s);
        s = INVALID_SOCKET;
}

int FTCPServerConnection::Fail() const {
        return s==INVALID_SOCKET;
}



//////////////////////////////////////////////////////////////////////////////
static unsigned short service_to_port(const char *s) {
        char *endptr;
        unsigned short port=(unsigned short)strtol(s,&endptr,0);
        if(endptr && *endptr) {
                //not digits
                servent *se=getservbyname((char*)s,"tcp");
                if(se)
                        return ntohs(se->s_port);
                else
                        return 0;
        } else
                return port;
}

FTCPServer_nontemplate::FTCPServer_nontemplate(const char *parms)
  : FServer(),
    port(service_to_port(parms))
{
}

void FTCPServer_nontemplate::FreeConnection(FConnection *con) {
        delete con;
}

FBroker *FTCPServer_nontemplate::AllocBroker() {
        return new FTCPBroker(this);
}

void FTCPServer_nontemplate::StartShutdown() {
        FServer::StartShutdown();
}

void FTCPServer_nontemplate::WaitShutdown() {
        {
                //1: make sure the broker thread wakes up so it notices the
                //shutdown flag
                //We wake the thread by making a connection
                int s = socket(PF_INET, SOCK_STREAM, 0);
                if(s==INVALID_SOCKET)
                        return;

                sockaddr_in server;
                memset(&server,0,sizeof(server));
                server.sin_family = AF_INET;
                server.sin_addr.s_addr = inet_addr((char*)"127.0.0.1");
                server.sin_port = htons(port);

                if(connect(s, (sockaddr*)&server,sizeof(server))>=0)
                        closesocket(s);
        }
        //The remaining steps are up to the FServer class
        FServer::WaitShutdown();
}

void FTCPServer_nontemplate::ForceShutdown() {
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
void FTCPBroker::Go() {
        WSADATA wsadata;
        if(WSAStartup(MAKEWORD(1,1),&wsadata)!=0)
                return;

        SOCKET s = socket(PF_INET, SOCK_STREAM, 0);
        if(s==INVALID_SOCKET) 
                return;

        sockaddr_in myname;
        memset(&myname,0,sizeof(myname));
        myname.sin_family = AF_INET;
        //myname.sin_addr = 0;
        myname.sin_port = htons(server->port);
        if(bind(s,(sockaddr*)&myname,sizeof(myname))) {
                closesocket(s);
                return;
        }

        if(listen(s,10)) {
                closesocket(s);
                return;
        }

        while(!IsShuttingDown()) {
                sockaddr client;
                int namelen=sizeof(client);
                SOCKET c;
                if((c=accept(s,&client,&namelen))==INVALID_SOCKET) {
                        closesocket(s);
                        break;
                }
                if(IsShuttingDown()) {
                        closesocket(s);
                        closesocket(c);
                        break;
                }
                FTCPServerConnection *con=new FTCPServerConnection(c);
                FThread *t = server->AllocThread(con);
                RegisterConnection(con,t);
        }

        closesocket(s);
}

