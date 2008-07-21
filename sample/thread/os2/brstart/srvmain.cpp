#define INCL_DOSQUEUES
#define BIFINCL_MAINTHREAD
#include "constrt.hpp"
#include "smthrd.hpp"
#include "brserver.hpp"
#include "tdelthrd.hpp"
#include "dispjobs.hpp"
#include "protocol.hpp"

#include <stdio.h>
#include <conio.h>
#include <string.h>

FMutexSemaphore stdio_mutex;
volatile int terminate_server=0;
char resultCodeQueueName[256];
ThreadDeleteThread *threadDeleter;

static void GenerateQueueName() {
        //function for generating system-wide unique queue name for picking 
        //up result codes from child sessions
        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib,&ppib);
        sprintf(resultCodeQueueName,"\\QUEUES\\brserver\\result\\%08ld",(unsigned long)(ppib->pib_ulpid));
}

class ServerMainThread : public FMainThread {
public:
        ServerMainThread(int argc, char **argv)
          : FMainThread(argc,argv)
          {}
        int Main(int argc, char **argv);
};

DEFBIFTHREADMAIN(ServerMainThread);

int ServerMainThread::Main(int argc, char **argv) {
        printf("brserver v1.0\n");
        if(argc>2 ||
           (argc==2 &&
            (strcmp(argv[1],"/?")==0 ||
             strcmp(argv[1],"/h")==0 ||
             strcmp(argv[1],"/help")==0 ||
             strcmp(argv[1],"-help")==0 ||
             strcmp(argv[1],"-usage")==0)))
        {
                printf("usage: brserver [-pipePipename]\n");
                return 0;
        }
        char serverPipeName[128];
        if(argc==2) {
                if(strncmp(argv[1],"-pipe",5)!=0) {
                        fprintf(stderr,"Unknown option: %s\n",argv[1]);
                        return 1;
                }
                strcpy(serverPipeName,argv[1]+5);
        } else
                strcpy(serverPipeName,DEFAULT_SERVER_PIPE_NAME);
        

        printf("Press Q to quit, j for joblist\n");

        GenerateQueueName();
        HQUEUE hqueueResult;
        APIRET rc=DosCreateQueue(&hqueueResult,QUE_FIFO,resultCodeQueueName);
        if(rc!=0) {
                fprintf(stderr,"Could not create queue %s (rc=%ld)\n",resultCodeQueueName,(unsigned long)rc);
                return 1;
        }
        SMThread smthread(hqueueResult);
        smthread.Start();

        threadDeleter = new ThreadDeleteThread;
        threadDeleter->Start();

        ConnectionStarterThread constart(serverPipeName);
        constart.Start();

        while(!terminate_server) {
                int c = getch();
                switch(c) {
                        case 'j':
                        case 'J': 
                                DisplayJobList();
                                break;
                        case 'q':
                        case 'Q':
                        case 'x':
                        case 'X':
                                terminate_server=1;
                                break;
                }
        }

        stdio_mutex.Request();
        printf("Server is shutting down...\n"); fflush(stdout);
        stdio_mutex.Release();

        //start closing the server
        terminate_server=1;

        threadDeleter->PleaseTerminate();
        threadDeleter->Wait();
        //delete threadDeleter;

        smthread.PleaseTerminate();
        smthread.Wait();
        DosCloseQueue(hqueueResult);

        stdio_mutex.Request();
        printf("Server has been shut down.\n"); fflush(stdout);
        stdio_mutex.Release();

        return 0;
}

