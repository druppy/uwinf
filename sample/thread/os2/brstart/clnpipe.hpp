#ifndef __CLNPIPE_HPP
#define __CLNPIPE_HPP

class ClientPipe_rep;
class ClientPipe {
protected:
        enum io_t { io_read, io_write, io_readwrite };
        ClientPipe_rep *rep;
        ClientPipe(const char *pipename, io_t io);
public:
        virtual ~ClientPipe();
        int Fail();

        int Connect();
        int DisConnect();
private:
        io_t iot;
};

class ClientIPipe : public virtual ClientPipe {
public:
        ClientIPipe(const char *pipename);
        int Read(void *buf, int buflen);
};

class ClientOPipe : public virtual ClientPipe {
public:
        ClientOPipe(const char *pipename);
        int Write(const void *buf, int buflen);
};

class ClientIOPipe : public virtual ClientPipe,
                     public virtual ClientIPipe,
                     public virtual ClientOPipe
{
public:
        ClientIOPipe(const char *pipename);
        int Transact(const void *outbuf, int outbuflen,
                     void *inbuf, int inbuflen);
        int Call(const void *outbuf, int outbuflen,
                 void *inbuf, int inbuflen);

};

#endif
