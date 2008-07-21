/***
Filename: 
Description:
  Implementation of FNPClientConnection
Host:
  Watcom 11.0
History:
  ISJ 97-09-?? - 97-10-18 Creation
***/

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSNMPIPES

#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_CLIENT
#define BIFINCL_NPCLIENT
#include <bif.h>

#include <string.h>

FNPClientConnection::FNPClientConnection(const char *name)
  : hf((HFILE)-1)
{
        char buf[256];
        //prefix \pipe if necesarry
        if(strncmp(name,"\\\\",2)!=0 &&         //not UNC
           strnicmp(name,"\\pipe",5)!=0)        //no \PIPE prefix
        {
           strcpy(buf,"\\pipe\\");
           while(*name=='\\') name++;
           strcat(buf,name);
           name = buf;
        }
        
        APIRET rc;
        ULONG action;
        rc = DosOpen((PSZ)name,
                     &hf,
                     &action,
                     0, //size
                     0, //attribute
                     OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                     OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE,
                     0
                    );
        if(rc) {
                hf=(HFILE)-1;
                if(rc==ERROR_PIPE_BUSY) {
                        rc = DosWaitNPipe(name,0);
                        if(rc==0) {
                                rc = DosOpen((PSZ)name,
                                             &hf,
                                             &action,
                                             0, //size
                                             0, //attribute
                                             OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                                             OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE,
                                             0
                                            );
                                if(rc) hf=(HFILE)-1;
                        }
                }
        } 
}

int FNPClientConnection::Read(void *buf, int maxbytes) {
        if(hf==(HFILE)-1) return -1;
        APIRET rc;
        ULONG bytesRead;
        rc = DosRead(hf,buf,maxbytes,&bytesRead);
        if(rc) {
                DosClose(hf);
                hf = (HFILE)-1;
                return -1;
        } else
                return (int)bytesRead;
}

int FNPClientConnection::Write(const void *buf, int bytes) {
        if(hf==(HFILE)-1) return -1;
        APIRET rc;
        ULONG bytesWritten;
        rc = DosWrite(hf,(PVOID)buf,bytes,&bytesWritten);
        if(rc) {
                DosClose(hf);
                hf = (HFILE)-1;
                return -1;
        } else
                return (int)bytesWritten;
}

void FNPClientConnection::Close() {
        if(hf!=(HFILE)-1) {
                DosClose(hf);
                hf = (HFILE)-1;
        }
}

int FNPClientConnection::Fail() const {
        return hf==(HFILE)-1;
}

