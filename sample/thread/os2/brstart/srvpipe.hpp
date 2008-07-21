#ifndef __SRVPIPE_HPP
#define __SRVPIPE_HPP

class ServerPipe_rep;
class ServerPipe {
public:
        enum type { type_byte, type_message };
protected:
        ServerPipe_rep *rep;
        enum io_t { io_inbound, io_outbound, io_duplex };
        ServerPipe(const char *pipename, int maxInstances, type t, io_t io);
public:
        virtual ~ServerPipe();
        int Fail();
        int Close();

        int Connect(long timeOut=-1);
        int DisConnect();

        int QueryHandle();
};


class ServerIPipe : public virtual ServerPipe {
public:
        ServerIPipe(const char *pipename, int maxInstances=-1, type t=type_message);

        int Read(void *buf, int buflen);
};

class ServerOPipe : public virtual ServerPipe {
public:
        ServerOPipe(const char *pipename, int maxInstances=-1, type t=type_message);

        int Write(const void *buf, int buflen);
};

class ServerIOPipe : public virtual ServerPipe,
                     public virtual ServerIPipe,
                     public virtual ServerOPipe
{
public:
        ServerIOPipe(const char *pipename, int maxInstances=-1, type t=type_message);
};

#endif

