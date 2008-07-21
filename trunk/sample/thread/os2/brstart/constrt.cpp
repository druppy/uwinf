#include "constrt.hpp"
#include "conthrd.hpp"
#include "brserver.hpp"
#include "srvpipe.hpp"
#include <string.h>
//for sprintf() only:
#include <stdio.h>

ConnectionStarterThread::ConnectionStarterThread(const char *p)
  : FThread()
{
        strcpy(basePipeName,p);
}

void ConnectionStarterThread::Go() {
        char realPipeName[256];
        sprintf(realPipeName,"\\PIPE\\%s",basePipeName);
        while(!terminate_server) {
                //create an instance of the pipe
                ServerIOPipe *sp=new ServerIOPipe(realPipeName);
                if(sp->Fail())
                        break;  //could not create pipe
                //wait for a client to connect
                if(sp->Connect()!=0)
                        break;  //could not connect
                if(terminate_server)
                        break;  //not nice, the clietn won't even get a nack
                //start a thread to handle the connection
                ConnectionThread *ct=new ConnectionThread(sp);
                ct->Start();
        }
}

