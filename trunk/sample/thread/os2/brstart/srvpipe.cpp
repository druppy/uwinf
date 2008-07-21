#define INCL_DOSERRORS
#define INCL_DOSNMPIPES
#define INCL_DOSMEMMGR
#define INCL_DOSPROCESS
#include <os2.h>
#include "srvpipe.hpp"
#include <string.h>

#include <stdio.h>

class ServerPipe_rep {
public:
        int failed;
        HPIPE hpipe;
        PVOID buffer;
};

ServerPipe::ServerPipe(const char *pipename, int maxInstances, type t, io_t io)
  : rep(new ServerPipe_rep)
{
        rep->failed=0;
        APIRET rc;
        rc = DosAllocMem(&rep->buffer,
                         4096,
                         PAG_COMMIT|OBJ_TILE|PAG_READ|PAG_WRITE);
        if(rc!=0) {
                rep->failed=1;
                return;
        }

        if(maxInstances==0) maxInstances=1;
        if(maxInstances!=-1)
                maxInstances &= 0xff;
        else
                maxInstances=255;
        ULONG openMode=NP_NOWRITEBEHIND|NP_NOINHERIT;
        if(io==io_inbound)
                openMode |= NP_ACCESS_INBOUND;
        else if(io==io_outbound)
                openMode |= NP_ACCESS_OUTBOUND;
        else
                openMode |= NP_ACCESS_DUPLEX;
        ULONG pipeMode=NP_WAIT|maxInstances;
        if(t==type_message)
                pipeMode |= NP_TYPE_MESSAGE|NP_READMODE_MESSAGE;
        else
                pipeMode |= NP_TYPE_BYTE|NP_READMODE_BYTE;
        rc=DosCreateNPipe((PSZ)pipename,
                          &rep->hpipe,
                          openMode,
                          pipeMode,
                          4096,
                          4096,
                          0
                         );
        if(rc!=0) {
                rep->failed=1;
                DosFreeMem(rep->buffer);
        }
}

ServerPipe::~ServerPipe() {
        if(!rep->failed) {
                DosFreeMem(rep->buffer);
                Close();
        }
        delete rep;
}

int ServerPipe::Fail() {
        return rep->failed;
}

int ServerPipe::Close() {
        if(rep->failed) return -1;
        if(rep->hpipe==(HPIPE)-1) return -1;
        //explicit close the pipe
        APIRET rc=DosClose(rep->hpipe);
        rep->hpipe=(HPIPE)-1;
        return rc==0?0:-1;
}

int ServerPipe::Connect(long timeOut) {
        if(rep->failed) {
                return -1;
        } else {
                if(timeOut==-1) {
                        //wait for a client to open the pipe
                        APIRET rc = DosConnectNPipe(rep->hpipe);
                        return rc==0?0:-1;
                } else if(timeOut<0)
                        return -1;
                else {
                        ULONG oldpstate;
                        //put the pipe into nonblocking mode
                        APIRET rc=DosQueryNPHState(rep->hpipe,&oldpstate);
                        ULONG newpstate=NP_NOWAIT | (oldpstate&(NP_READMODE_BYTE|NP_READMODE_MESSAGE));
                        rc = DosSetNPHState(rep->hpipe,newpstate);

                        while(timeOut>0) {
                                rc = DosConnectNPipe(rep->hpipe);
                                if(rc!=ERROR_INTERRUPT && rc!=ERROR_PIPE_NOT_CONNECTED)
                                        break;
                                DosSleep(50);
                                timeOut -= 50;
                        }
                        //return the pipe to blocking mode
                        DosSetNPHState(rep->hpipe, oldpstate&(NP_WAIT|NP_NOWAIT|NP_READMODE_BYTE|NP_READMODE_MESSAGE));
                        return rc==0?0:-1;
                }
        }
}

int ServerPipe::DisConnect() {
        if(rep->failed) return -1;
        //disconnect the pipe from the client
        APIRET rc;
        rc = DosDisConnectNPipe(rep->hpipe);
        return rc==0?0:-1;
}

int ServerPipe::QueryHandle() {
        if(rep->failed)
                return -1;
        else
                return (int)(rep->hpipe);
}



ServerIPipe::ServerIPipe(const char *pipename, int maxInstances, type t)
  : ServerPipe(pipename,maxInstances,t,io_inbound)
{ }

int ServerIPipe::Read(void *buf, int buflen) {
        if(rep->failed) return -1;
        //read a message
        APIRET rc;
        ULONG bytesRead;
        rc = DosRead(rep->hpipe,
                     rep->buffer,
                     buflen,
                     &bytesRead
                    );
        if(rc!=0) {
                return -1;
        } else {
                memcpy(buf,rep->buffer,buflen);
                return (int)bytesRead;
        }
}


ServerOPipe::ServerOPipe(const char *pipename, int maxInstances, type t)
  : ServerPipe(pipename,maxInstances,t,io_outbound)
{ }

int ServerOPipe::Write(const void *buf, int buflen) {
        if(rep->failed) return -1;
        //write a message
        APIRET rc;
        ULONG bytesWritten;
        memcpy(rep->buffer,buf,buflen);
        rc = DosWrite(rep->hpipe,
                      rep->buffer,
                      buflen,
                      &bytesWritten
                     );
        if(rc!=0) {
                return -1;
        } else {
                DosResetBuffer(rep->hpipe);
                return bytesWritten;
        }
}


ServerIOPipe::ServerIOPipe(const char *pipename, int maxInstances, type t)
  : ServerPipe(pipename,maxInstances,t,io_duplex),
    ServerIPipe(pipename,maxInstances,t),
    ServerOPipe(pipename,maxInstances,t)
{ }

