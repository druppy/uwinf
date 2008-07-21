/***
Filename: npclient.cpp
Description:
  Implementation of FNPClientConnection
Host:
  Watcom 11.0
History:
  ISJ 97-109-?? - 97-10-18 Creation
***/

#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_CLIENT
#define BIFINCL_NPCLIENT
#include <bif.h>

#include <string.h>

FNPClientConnection::FNPClientConnection(const char *name)
  : hf(INVALID_HANDLE_VALUE)
{
        char buf[256];
        //prefix \.\pipe if necesarry
        if(strncmp(name,"\\\\",2)!=0)        //no \\ prefix
        {
           strcpy(buf,"\\\\.\\pipe\\");
           while(*name=='\\') name++;
           strcat(buf,name);
           name = buf;
        }

        hf = CreateFile(name,
                        GENERIC_READ|GENERIC_WRITE,
                        0,      //no sharing
                        NULL,   //no security
                        OPEN_EXISTING,
                        0, //no flags
                        NULL
                       );
        if(hf==INVALID_HANDLE_VALUE)
        {
                DWORD rc=GetLastError();
                if(rc==ERROR_PIPE_BUSY)
                {
                        if(WaitNamedPipe(name,NMPWAIT_USE_DEFAULT_WAIT))
                        {
                                hf = CreateFile(name,
                                                GENERIC_READ|GENERIC_WRITE,
                                                0,      //no sharing
                                                NULL,   //no security
                                                OPEN_EXISTING,
                                                0, //no flags
                                                NULL
                                               );
                        } else {
                                //server is overloaded or slow - give up
                        }
                } else {
                        //most likely error_file_not_found
                }
        }
                        
}

int FNPClientConnection::Read(void *buf, int maxbytes) {
        if(hf==INVALID_HANDLE_VALUE) return -1;

        BOOL b;
        DWORD bytesRead=0;
        b = ReadFile(hf, buf, maxbytes, &bytesRead, NULL);
        if(b)
                return (int)bytesRead;
        else {
                CloseHandle(hf); hf=INVALID_HANDLE_VALUE;
                return -1;
        }
}

int FNPClientConnection::Write(const void *buf, int bytes) {
        if(hf==INVALID_HANDLE_VALUE) return -1;

        BOOL b;
        DWORD bytesWritten=0;
        b = WriteFile(hf, buf, bytes, &bytesWritten, NULL);
        if(b)
                return (int)bytesWritten;
        else {
                CloseHandle(hf); hf=INVALID_HANDLE_VALUE;
                return -1;
        }
}

void FNPClientConnection::Close() {
        if(hf!=INVALID_HANDLE_VALUE) {
                CloseHandle(hf);
                hf = INVALID_HANDLE_VALUE;
        }
}

int FNPClientConnection::Fail() const {
        return hf==INVALID_HANDLE_VALUE;
}

