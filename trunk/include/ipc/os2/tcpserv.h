#ifndef __TCPSERVER_H
#define __TCPSERVER_H

class FTCPServerConnection : public FConnection {
#if BIFOS_ == BIFOS_OS2_
        int s;
#elif BIFOS_ == BIFOS_WIN32_
        unsigned s;
#endif
public:
#if BIFOS_ == BIFOS_OS2_
        FTCPServerConnection(int sock)
          : FConnection(), s(sock)
          {}
#elif BIFOS_ == BIFOS_WIN32_
        FTCPServerConnection(unsigned sock)
          : FConnection(), s(sock)
          {}
#endif
        ~FTCPServerConnection()
          { Close(); }
        int Read(void *buf, int maxbytes);
        int Write(const void *buf, int bytes);
        void Close();
        int Fail() const;

#if BIFOS_ == BIFOS_OS2_
        int GetSocket() { return s; }
#elif BIFOS_ == BIFOS_WIN32_
        unsigned GetSocket() { return s; }
#endif
};

class FTCPBroker;
class FTCPServer_nontemplate : public FServer {
        friend class FTCPBroker;
        unsigned short port;
protected:
        FTCPServer_nontemplate(const char *parms);
        void FreeConnection(FConnection *con);
        virtual FThread *AllocThread(FTCPServerConnection *c) =0;
        virtual FBroker *AllocBroker();
public:
        void StartShutdown();
        void WaitShutdown();
        void ForceShutdown();
        
};

class FTCPBroker : public FBroker {
        FTCPServer_nontemplate *server;
public:
        FTCPBroker(FTCPServer_nontemplate *s)
          : FBroker(s),
            server(s)
          {}
        void Go();
};



template<class CommThread>
class FTCPServer : public FTCPServer_nontemplate {
public:
        FTCPServer(const char *parms)
          : FTCPServer_nontemplate(parms)
          { }
protected:
        FThread *AllocThread(FTCPServerConnection *c)
          { return new CommThread(this,c); }
        void FreeThread(FThread *t)
          { delete t; }
};


#endif
