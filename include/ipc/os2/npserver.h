#ifndef __NPSERVER_H
#define __NPSERVER_H

class FNPServerConnection : public FConnection {
#if BIFOS_ == BIFOS_OS2_
        LHANDLE hpipe;
#elif BIFOS_ == BIFOS_WIN32_
        HANDLE hpipe;
#endif
public:
#if BIFOS_ == BIFOS_OS2_
        FNPServerConnection(LHANDLE hp)
          : FConnection(), hpipe(hp)
          {}
#elif BIFOS_ == BIFOS_WIN32_
        FNPServerConnection(HANDLE hp)
          : FConnection(), hpipe(hp)
          {}
#endif
        ~FNPServerConnection()
          { Close(); }
        int  Read(void *buf, int maxbytes);
        int  Write(const void *buf, int bytes);
        void Close();
        int Fail() const;

#if BIFOS_ == BIFOS_OS2_
        LHANDLE GetHPIPE() { return hpipe; }
#elif BIFOS_ == BIFOS_WIN32_
        HANDLE GetHPIPE() { return hpipe; }
#endif
};

class FNPBroker;
class FNPServer_nontemplate : public FServer {
        friend class FNPBroker;
        char pipename[256];
protected:
        FNPServer_nontemplate(const char *parms);
        void FreeConnection(FConnection *con);
        virtual FThread *AllocThread(FNPServerConnection *c) =0;
        virtual FBroker *AllocBroker();
public:
        void StartShutdown();
        void WaitShutdown();
        void ForceShutdown();
        
};

class FNPBroker : public FBroker {
        FNPServer_nontemplate *server;
public:
        FNPBroker(FNPServer_nontemplate *s)
          : FBroker(s),
            server(s)
          {}
        void Go();
};



template<class CommThread>
class FNPServer : public FNPServer_nontemplate {
public:
        FNPServer(const char *parms)
          : FNPServer_nontemplate(parms)
          { }
protected:
        FThread *AllocThread(FNPServerConnection *c)
          { return new CommThread(this,c); }
        void FreeThread(FThread *t)
          { delete t; }
};


#endif
