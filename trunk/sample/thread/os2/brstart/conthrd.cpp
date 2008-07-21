#include "job.hpp"
#include "conthrd.hpp"
#include "protocol.hpp"
#include "connecti.hpp"
#include "conlist.hpp"
#include "joblist.hpp"
#include "srvpipe.hpp"
#include "brserver.hpp"
#include "tdelthrd.hpp"
#include "jobstart.hpp"

#include <string.h>
#include <stdio.h>

static void generateUniquePipeName(char *pipename) {
        static FMutexSemaphore sequence_mutex;
        static unsigned sequence=0;
        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib,&ppib);
        sequence_mutex.Request();
          sprintf(pipename,"%08ld\\%08ld\\%08d",
                  (unsigned long)ppib->pib_ulpid,
                  (unsigned long)ptib->tib_ptib2->tib2_ultid,
                  sequence++
                 );
        sequence_mutex.Release();
}

ConnectionThread::ConnectionThread(ServerIOPipe *p)
  : FThread(64*1024), pipe(p)
{}

ConnectionThread::~ConnectionThread() {
        delete pipe;
}

void ConnectionThread::Go() {
        //register the connection
        Connection con;
        con.pipe=pipe;
        con.thread=this;
        connectionList.AddConnection(&con);

        Job *job = new Job;
        connection_closed=0;
        void *com_buffer = new char[4096];
        while(!connection_closed) {
                int r=pipe->Read(com_buffer,4096);
                if(r<=0) break;
                handleMessage(com_buffer,job);
        }
        delete[] (char*)com_buffer;

        connectionList.RemoveConnection(&con);
        pipe->DisConnect();

        if(job->started) {
                job->event_terminated.Wait();
                jobList.RemoveJob(job);
        }
        delete job;

        threadDeleter->MarkThreadForDelete(this);
}


void ConnectionThread::handleMessage(void *com_buffer, Job *job) {
        int r=0;
        switch(((CR_common*)com_buffer)->requestType) {
                case crt_close: {
                        SA_Ack answer;
                        r=pipe->Write(&answer,sizeof(answer));
                        connection_closed=1;
                        break;
                }

                case crt_queryjoblist: {
                        SA_JobList answer;
                        jobList.StartTraversal();
                        Job *job=jobList.GetNext();
                        while(job) {
                                answer.AddJob(job->niceName,job->command,job->terminated);
                                job=jobList.GetNext();
                        }
                        jobList.EndTraversal();
                        r=pipe->Write(&answer,sizeof(answer));
                        break;
                }
                case crt_setcommand: {
                        if(job->started) {
                                SA_Nack answer("Job already started");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                strcpy(job->command, ((CR_SetCommand*)com_buffer)->command);
                                SA_Ack answer;
                                r=pipe->Write(&answer,sizeof(answer));
                        }
                        break;
                }
                case crt_setparameters: {
                        if(job->started) {
                                SA_Nack answer("Job already started");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                memcpy(job->parameters, ((CR_SetParameters*)com_buffer)->parameters, MAXPARAMETERLENGTH);
                                SA_Ack answer;
                                r=pipe->Write(&answer,sizeof(answer));
                        }
                        break;
                }
                case crt_setcwd: {
                        if(job->started) {
                                SA_Nack answer("Job already started");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                CR_SetCwd *cwd=(CR_SetCwd*)com_buffer;
                                int drive=cwd->cwd[0];
                                if(drive>='A' && drive<='Z')
                                        drive-='A';
                                else if(drive>='a' && drive<='z')
                                        drive-='z';
                                else
                                        drive=-1;
                                if(drive!=-1) {
                                        strcpy(job->currentDir[drive],cwd->cwd);
                                        SA_Ack answer;
                                        r=pipe->Write(&answer,sizeof(answer));
                                } else {
                                        SA_Nack answer("Invalid drive letter");
                                        r=pipe->Write(&answer,sizeof(answer));
                                }
                        }
                        break;
                }
                case crt_setcd: {
                        if(job->started) {
                                SA_Nack answer("Job already started");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                strcpy(job->currentDrive, ((CR_SetCd*)com_buffer)->drive);
                                SA_Ack answer;
                                r=pipe->Write(&answer,sizeof(answer));
                        }
                        break;
                }
                case crt_setinputredir: {
                        if(job->inputPipeName[0]) {
                                SA_Nack answer("input redirection already specified");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                char buf[MAXPIPENAMELENGTH];
                                generateUniquePipeName(buf);
                                sprintf(job->inputPipeName,"\\PIPE\\%s",buf);
                                job->inputPipe = new ServerIPipe(job->inputPipeName,1,ServerIPipe::type_byte);
                                if(job->inputPipe->Fail()) {
                                        SA_Nack answer("input redirection failed");
                                        r=pipe->Write(&answer,sizeof(answer));
                                        job->inputPipeName[0]='\0';
                                        delete job->inputPipe;
                                } else {
                                        SA_PipeName pipename(job->inputPipeName);
                                        pipe->Write(&pipename,sizeof(pipename));
                                        r=job->inputPipe->Connect(2000);
                                        if(r!=0) {
                                                SA_Nack answer("input redirection connect timeout");
                                                pipe->Write(&answer,sizeof(answer));
                                                job->inputPipeName[0]='\0';
                                                delete job->inputPipe;
                                        } else {
                                                SA_Ack answer;
                                                r=pipe->Write(&answer,sizeof(answer));
                                        }
                                }
                        }
                        break;
                }
                case crt_setoutputredir: {
                        if(job->outputPipeName[0]) {
                                SA_Nack answer("output redirection already specified");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                char buf[MAXPIPENAMELENGTH];
                                generateUniquePipeName(buf);
                                sprintf(job->outputPipeName,"\\PIPE\\%s",buf);
                                job->outputPipe = new ServerOPipe(job->outputPipeName,1,ServerIPipe::type_byte);
                                if(job->outputPipe->Fail()) {
                                        SA_Nack answer("output redirection failed");
                                        r=pipe->Write(&answer,sizeof(answer));
                                        job->outputPipeName[0]='\0';
                                        delete job->outputPipe;
                                } else {
                                        SA_PipeName pipename(job->outputPipeName);
                                        pipe->Write(&pipename,sizeof(pipename));
                                        r=job->outputPipe->Connect(2000);
                                        if(r!=0) {
                                                SA_Nack answer("output redirection connect timeout");
                                                pipe->Write(&answer,sizeof(answer));
                                                job->outputPipeName[0]='\0';
                                                delete job->outputPipe;
                                        } else {
                                                SA_Ack answer;
                                                r=pipe->Write(&answer,sizeof(answer));
                                        }
                                }
                        }
                        break;
                }
                case crt_setnicename: {
                        if(job->started) {
                                SA_Nack answer("Job already started");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                strcpy(job->niceName, ((CR_SetNiceName*)com_buffer)->niceName);
                                SA_Ack answer;
                                r=pipe->Write(&answer,sizeof(answer));
                        }
                        break;
                }
                case crt_start: {
                        if(job->started) {
                                SA_Nack answer("Job already started");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else if(job->command[0]=='\0') {
                                SA_Nack answer("command not specified");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                char errmsg[MAXERRORMSGLENGTH];
                                if(StartJob(job,errmsg)!=0) {
                                        SA_Nack answer(errmsg);
                                        r=pipe->Write(&answer,sizeof(answer));
                                } else {
                                        job->waitForTermination = ((CR_Start*)com_buffer)->waitForTermination;
                                        SA_Ack answer;
                                        r=pipe->Write(&answer,sizeof(answer));
                                        if(job->outputPipeName[0]) {
                                                job->outputPipeName[0]='\0';
                                                delete job->outputPipe;
                                        }
                                        if(job->inputPipeName[0]) {
                                                job->inputPipeName[0]='\0';
                                                delete job->inputPipe;
                                        }
                                }
                        }
                        break;
                }
                case crt_wait: {
                        if(!job->started) {
                                SA_Nack answer("Job not started");
                                r=pipe->Write(&answer,sizeof(answer));
                        } else {
                                job->event_terminated.Wait();
                                SA_ResultCode answer(job->resultCode);
                                r=pipe->Write(&answer,sizeof(answer));
                        }
                        break;
                }
                default: {
                        SA_Nack answer("Unknown request");
                        r=pipe->Write(&answer,sizeof(answer));
                }
        }
        if(r==-1) {
                //write error, better terminate conversation
                connection_closed=1;
        }
}


