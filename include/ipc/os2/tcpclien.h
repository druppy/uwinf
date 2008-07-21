#ifndef __TCPCLIENT_H
#define __TCPCLIENT_H

class FTCPClientConnection {
#if BIFOS_ == BIFOS_OS2_
        int s;
#elif BIFOS_ == BIFOS_WIN32_
        unsigned s;
#endif
public:
        FTCPClientConnection(const char *name);
        virtual ~FTCPClientConnection() { Close(); }
        virtual int  Read(void *buf, int maxbytes);
        virtual int  Write(const void *buf, int bytes);
        virtual void Close();
        virtual int Fail() const;

#if BIFOS_ == BIFOS_OS2_
        int GetSocket() { return s; }
#elif BIFOS_ == BIFOS_WIN32_
        unsigned GetSocket() { return s; }
#endif
};

#endif
