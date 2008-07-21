/***
Filename: tcpclien.cpp
Description:
  Implementation of FTCPClientConnection
Host:
  Watcom 11.0
History:
  ISJ 97-109-?? - 97-10-18 Creation
***/

#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_CLIENT
#define BIFINCL_TCPCLIENT
#include <bif.h>
#include <string.h>
#include <stdlib.h>

#include <winsock.h>

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

static u_long host_to_inetaddr(const char *s) {
        //if the string only contains digits and dots we assume it is a 
        //direct ip address
        for(const char *p=s; *p; p++){
                if(!((*p>='0' && *p<'9') || *p=='.')) {
                        hostent *he=gethostbyname((char*)s);
                        if(he)
                                return ((in_addr*)(he->h_addr_list[0]))->s_addr;
                        else
                                return 0;
                }
        }
        return inet_addr((char*)s);
}

FTCPClientConnection::FTCPClientConnection(const char *parm)
  : s(INVALID_SOCKET)
{
        WSADATA wsadata;
        if(WSAStartup(MAKEWORD(1,1),&wsadata)!=0)
                return;

        //parm is "host:port"
        char buf[256];
        strcpy(buf,parm);
        char *host=strtok(buf,":");
        char *service=strtok(0,":");
        if(!(host || service)) return;
        u_long hostaddr;
        if(*host=='\0')
                hostaddr = inet_addr((char*)"127.0.0.1");
        else        
                hostaddr = host_to_inetaddr(host);
        if(!hostaddr) return;
        unsigned short port;
        port=service_to_port(service);
        if(!port) return;

        s = socket(PF_INET, SOCK_STREAM, 0);
        if(s==INVALID_SOCKET) return;

        sockaddr_in server;
        memset(&server,0,sizeof(server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = hostaddr;
        server.sin_port = htons(port);

        if(connect(s, (sockaddr*)&server,sizeof(server))<0) {
                closesocket(s);
                s=INVALID_SOCKET;
                return;
        }
}

int FTCPClientConnection::Read(void *buf, int maxbytes) {
        if(s==INVALID_SOCKET) return -1;
        int bytesRead=recv(s,(char*)buf,maxbytes,0);
        if(bytesRead<0) {
                closesocket(s);
                s=INVALID_SOCKET;
                return -1;
        } else
                return bytesRead;
}

int FTCPClientConnection::Write(const void *buf, int bytes) {
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

void FTCPClientConnection::Close() {
        if(s==INVALID_SOCKET) return;
        closesocket(s);
        s = INVALID_SOCKET;
}

int FTCPClientConnection::Fail() const {
        return s==INVALID_SOCKET;
}

