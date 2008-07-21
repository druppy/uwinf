#include "clnpipe.hpp"
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

class ClientPipe_rep {
public:
        char *name;     //name of pipe
        int fd;         //file descriptor of pipe

        ClientPipe_rep(const char *n) {
                name=new char[strlen(n)+1];
                strcpy(name,n);
        }
        ~ClientPipe_rep() {
                delete[] name;
        }
};

ClientPipe::ClientPipe(const char *pipename, io_t io)
  : rep(new ClientPipe_rep(pipename)),
    iot(io)
{ }

ClientPipe::~ClientPipe() {
        if(rep && rep->fd>=0) DisConnect();
        delete rep;
}

int ClientPipe::Fail() {
        return rep==0;
}

int ClientPipe::Connect() {
        switch(iot) {
                case io_read: 
                        rep->fd = open(rep->name, O_RDONLY|O_BINARY); //read existing file (=pipe)
                        break;
                case io_write:
                        rep->fd = open(rep->name, O_WRONLY|O_BINARY); //read/write existing file (=pipe)
                        break;
                case io_readwrite:
                        rep->fd = open(rep->name, O_RDWR|O_BINARY); //read/write existing file (=pipe)
                        break;
        }
        if(rep->fd<0)
                return -1;
        else
                return 0;
}


int ClientPipe::DisConnect() {
        if(rep->fd>=0) {
                int r=close(rep->fd);
                rep->fd=-1;
                if(r!=0)
                        return -1;
                else
                        return 0;
        } else
                return -1;
}


ClientIPipe::ClientIPipe(const char *pipename)
  : ClientPipe(pipename,io_read)
{ }

int ClientIPipe::Read(void *buf, int buflen) {
        int bytesRead;
        bytesRead=read(rep->fd,buf,buflen);
        if(bytesRead==-1)
                return -1;
        else
                return bytesRead;
}


ClientOPipe::ClientOPipe(const char *pipename)
  : ClientPipe(pipename,io_write)
{ }

int ClientOPipe::Write(const void *buf, int buflen) {
        int bytesWritten;
        bytesWritten=write(rep->fd,buf,buflen);
        if(bytesWritten==-1 || bytesWritten<buflen)
                return -1;
        else
                return 0;
}


ClientIOPipe::ClientIOPipe(const char *pipename)
  : ClientPipe(pipename,io_readwrite),
    ClientIPipe(pipename),
    ClientOPipe(pipename)
{ }

int ClientIOPipe::Transact(const void *outbuf, int outbuflen,
                           void *inbuf, int inbuflen)
{
        if(Write(outbuf,outbuflen)==-1)
                return -1;
        return Read(inbuf,inbuflen);
}

int ClientIOPipe::Call(const void *outbuf, int outbuflen,
                       void *inbuf, int inbuflen)
{
        if(Connect()==-1) return -1;
        int r=Transact(outbuf,outbuflen,inbuf,inbuflen);
        DisConnect();
        return r;
}

