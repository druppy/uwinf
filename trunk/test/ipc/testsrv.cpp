#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD

#define BIFINCL_IPC
#define BIFINCL_CLIENTSERVER
#define BIFINCL_SERVER
#define BIFINCL_TCPSERVER
#include <bif.h>

class MyCommThread : public FThread {
        FServer *s;
        FConnection *c;
public:
        MyCommThread(FServer *server, FConnection *connection)
          : FThread(),
            s(server),
            c(connection)
          {}
        void Go() {
                c->Close();
                s->DeleteMe(this);
        }        
};

class MyMainThread : public FMainThread {
public:
        MyMainThread(int argc, char **argv)
          : FMainThread(argc,argv)
          {}
        int Main(int,char**);
};

int MyMainThread::Main(int, char **) {
        FTCPServer<MyCommThread> server("5500");
        server.Start();
        Sleep(500); //allow server to start up

        Sleep(10000);

        server.StartShutdown();
        server.WaitShutdown();
        return 0;
}

DEFBIFTHREADMAIN(MyMainThread);

