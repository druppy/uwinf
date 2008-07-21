#define INCL_DOSFILEMGR
#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "protocol.hpp"
#include "clnpipe.hpp"
#include <string.h>
#include <conio.h>

class ClientMainThread : public FMainThread {
public:
        ClientMainThread(int argc, char **argv)
          : FMainThread(argc,argv)
          {}
        int Main(int argc, char **argv);
private:
        void showUsage();
        int processParameters(int artc, char **argv);
        int execute();


};

int MyTransact(ClientIOPipe *pipe,
               const void *inbuffer, int inbuflen,
               void *outbuffer, int outbuflen,
               unsigned long expectedAnswer
              );


class InputThread : public FThread {
        ClientOPipe *pipe;
public:
        InputThread(ClientOPipe *p)
          : FThread(), pipe(p)
          {}
        void Go();
};
class OutputThread : public FThread {
        ClientIPipe *pipe;
public:
        OutputThread(ClientIPipe *p)
          : FThread(), pipe(p)
          {}
        void Go();
};



DEFBIFTHREADMAIN(ClientMainThread);

int ClientMainThread::Main(int argc, char **argv) {
        if(argc<2 || stricmp(argv[1],"/?")==0 ||
                     stricmp(argv[1],"/help")==0 ||
                     stricmp(argv[1],"/usage")==0 ||
                     stricmp(argv[1],"-?")==0 ||
                     stricmp(argv[1],"-help")==0 ||
                     stricmp(argv[1],"-usage")==0)
        {
                showUsage();
                return 1;
        } else {
                if(processParameters(argc,argv)==0)
                        return execute();
                else
                        return -1;
        }
}


void ClientMainThread::showUsage() {
        printf("brstart v1.0\n");
        printf("usage: brstart [option...]\n");
        printf("  -prg<program>    Which program to start\n");
        printf("  -parm<parameter> A parameter to the program\n");
        printf("  -cwd<cwd>        Current working directory\n");
        printf("  -cd<drive>       Current drive\n");
        printf("  -oredir          Redirect output\n");
        printf("  -iredir          Redirect input\n");
        printf("  -echo            Allow possible double-echo when -iredir is specified\n");
        printf("  -nn<name>        Set nice name of job\n");
        printf("  -server<server>  Server name. Default is local machine\n");
        printf("  -pipe<pipename>  Server pipe name. Default is \"brserver\"\n");
        printf("  -nowait          Does not wait for job to finish\n");
        printf("  -joblist         Display a job list\n");
}

char servername[32]="";
char pipename[128]="brserver";
int  redirectInput=0;
int  redirectOutput=0;
char parameters[MAXPARAMETERLENGTH]="\0\0";
char program[MAXCOMMANDLENGTH]="";
char currentDisk[3]="";
char currentDirectory[26][MAXCWDLENGTH];
char niceName[MAXNICENAMELENGTH]="";
int waitForJobToFinish=1;
int showJobList=0;
int allowEcho=0;

int ClientMainThread::processParameters(int argc, char **argv) {
        for(int i=0; i<26; i++) currentDirectory[i][0]='\0';

        while(--argc) {
                argv++;
                if(strncmp(*argv,"-prg",4)==0) {
                        strcpy(program,*argv+4);
                } else if(strncmp(*argv,"-parm",5)==0) {
                        char *p=parameters;
                        while(p[0] || p[1]) p++;
                        if(p!=parameters) 
                                strcpy(++p,*argv+5);
                        else
                                strcpy(p,*argv+5);
                        while(*p) p++;
                        p[1]='\0';
                } else if(strncmp(*argv,"-cwd",4)==0) {
                        int drive=argv[0][4];
                        if(drive>='A' && drive<='Z')
                                drive-='A';
                        else if(drive>='a' && drive<='z')
                                drive-='a';
                        else {
                                fprintf(stderr,"Illegal drive specified:%s\n",*argv);
                                return -1;
                        }
                        strcpy(currentDirectory[drive],*argv+4);
                } else if(strncmp(*argv,"-cd",3)==0) {
                        int drive=argv[0][3];
                        if(drive>='A' && drive<='Z')
                                drive-='A';
                        else if(drive>='a' && drive<='z')
                                drive-='a';
                        else {
                                fprintf(stderr,"Illegal drive specified:%s\n",*argv);
                                return -1;
                        }
                        currentDisk[0]=(char)(drive+'A');
                        currentDisk[1]=':';
                        currentDisk[2]='\0';
                } else if(strcmp(*argv,"-oredir")==0) {
                        redirectOutput=1;
                } else if(strcmp(*argv,"-iredir")==0) {
                        redirectInput=1;
                } else if(strcmp(*argv,"-echo")==0) {
                        allowEcho=1;
                } else if(strncmp(*argv,"-nn",3)==0) {
                        strcpy(niceName,*argv+3);
                } else if(strncmp(*argv,"-server",7)==0) {
                        strcpy(servername,*argv+7);
                } else if(strncmp(*argv,"-pipe",5)==0) {
                        strcpy(pipename,*argv+5);
                } else if(strcmp(*argv,"-nowait")==0) {
                        waitForJobToFinish=0;
                } else if(strcmp(*argv,"-joblist")==0) {
                        showJobList=1;
                } else {
                        fprintf(stderr,"Unknown option: %s\n",*argv);
                        return -1;
                }
        }
        if(program[0]=='\0' && !showJobList) {
                fprintf(stderr,"-prg not specified\n");
                return -1;
        }
        if((redirectInput || redirectOutput) && !waitForJobToFinish) {
                fprintf(stderr,"-iredir and -oredir cannot be used with -nowait\n");
                return -1;
        }
        return 0;
}



int ClientMainThread::execute() {
        char fullPipeName[256];
        if(servername[0])
                sprintf(fullPipeName,"\\\\%s\\PIPE\\%s",servername,pipename);
        else
                sprintf(fullPipeName,"\\PIPE\\%s",pipename);

        ClientIOPipe pipe(fullPipeName);
        if(pipe.Fail()) {
                fprintf(stderr,"Could not connect to pipe\n");
                return -1;
        }
        if(pipe.Connect()!=0) {
                fprintf(stderr,"Could not connect to pipe\n");
                return -1;
        }

        char *com_buffer=new char[4096];
        if(showJobList) {
                CR_QueryJobList request;
                if(MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_joblist)!=0)
                        return -1;
                SA_JobList *s= (SA_JobList*)com_buffer;
                printf("%d jobs\n",(int)s->jobs);
                for(int j=0; j<s->jobs; j++) {
                        if(s->j[j].niceName[0])
                                printf("%s:\n  ",s->j[j].niceName);
                        printf("%-68.68s %s\n",s->j[j].command,s->j[j].status?"Finished":"Running");
                }
                CR_Close crequest;
                MyTransact(&pipe, &crequest,sizeof(crequest), com_buffer,4096, sat_ack);
                return 0;
        }

        {
                CR_SetCommand request(program);
                if(MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_ack)!=0)
                        return -1;
        }
        if(niceName[0]) {
                CR_SetNiceName request(niceName);
                if(MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_ack)!=0)
                        return -1;
        }
        if(parameters[0]) {
                CR_SetParameters request(parameters);
                if(MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_ack)!=0)
                        return -1;
        }
        if(currentDisk[0]) {
                CR_SetCd request(currentDisk);
                if(MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_ack)!=0)
                        return -1;
        }
        for(int d=0; d<26; d++) {
                if(currentDirectory[d][0]) {
                        CR_SetCwd request(currentDirectory[d]);
                        if(MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_ack)!=0)
                                return -1;
                }
        }
        ClientIPipe *outputRedirectionPipe=0;
        if(redirectOutput) {
                CR_SetOutputRedir request;
                if(MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_pipename)!=0)
                        return -1;
                char pipename[256];
                if(servername[0])
                        sprintf(pipename,"\\\\%s%s",servername,((SA_PipeName*)com_buffer)->pipename);
                else
                        strcpy(pipename,((SA_PipeName*)com_buffer)->pipename);
                outputRedirectionPipe = new ClientIPipe(pipename);
                Sleep(1000); //give the server enough time to issue a pipe::connect
                if(outputRedirectionPipe->Connect()!=0) {
                        fprintf(stderr,"could not connect to output redirection pipe \"%s\"\n",pipename);
                        return -2;
                }
                if(pipe.Read(com_buffer,4096)==-1) {
                        fprintf(stderr,"pipe read error\n");
                        return -1;
                }
                if(((SA_common*)com_buffer)->answerType==sat_nack) {
                        fprintf(stderr,"Nack received: \"%s\"\n", ((SA_Nack*)com_buffer)->reason);
                        return -1;
                }
        }
        ClientOPipe *inputRedirectionPipe=0;
        if(redirectInput) {
                CR_SetInputRedir request;
                if(MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_pipename)!=0)
                        return -1;
                char pipename[256];
                if(servername[0])
                        sprintf(pipename,"\\\\%s%s",servername,((SA_PipeName*)com_buffer)->pipename);
                else
                        strcpy(pipename,((SA_PipeName*)com_buffer)->pipename);
                inputRedirectionPipe = new ClientOPipe(pipename);
                Sleep(1000); //give the server enough time to issue a pipe::connect
                if(inputRedirectionPipe->Connect()!=0) {
                        fprintf(stderr,"could not connect to input redirection pipe \"%s\"\n",pipename);
                        return -2;
                }
                if(pipe.Read(com_buffer,4096)==-1) {
                        fprintf(stderr,"pipe read error\n");
                        return -1;
                }
                if(((SA_common*)com_buffer)->answerType==sat_nack) {
                        fprintf(stderr,"Nack received: \"%s\"\n", ((SA_Nack*)com_buffer)->reason);
                        return -1;
                }
        }
        int rc;
        {
                CR_Start start(waitForJobToFinish);
                if(MyTransact(&pipe, &start,sizeof(start), com_buffer,4096, sat_ack)==0) {
                        InputThread *it=0;
                        OutputThread *ot=0;
                        if(outputRedirectionPipe) {
                                ot = new OutputThread(outputRedirectionPipe);
                                ot->Start();
                        }
                        if(inputRedirectionPipe) {
                                it = new InputThread(inputRedirectionPipe);
                                it->Start();
                        }
                        if(waitForJobToFinish) {
                                CR_Wait wait;
                                if(MyTransact(&pipe, &wait,sizeof(wait), com_buffer,4096, sat_resultcode)==0) {
                                        rc = ((SA_ResultCode*)com_buffer)->code;
                                } else
                                        rc=-1;
                        } else
                                rc=0;
                        if(ot) { ot->Wait(); delete ot; }
                        if(it) { it->Wait(); delete it; }
                } else
                        rc=-1;
        }
        {
                CR_Close request;
                MyTransact(&pipe, &request,sizeof(request), com_buffer,4096, sat_ack);
        }

        return 0;
}

int MyTransact(ClientIOPipe *pipe,
               const void *inbuffer, int inbuflen,
               void *outbuffer, int outbuflen,
               unsigned long expectedAnswer
              )
{
        if(pipe->Write(inbuffer,inbuflen)==-1) {
                fprintf(stderr,"Could not write to pipe\n");
                return -1;
        }
        int bytesRead=pipe->Read(outbuffer,outbuflen);
        if(bytesRead==-1) {
                fprintf(stderr,"Could not read from pipe\n");
                return -1;
        }
        if(bytesRead==0) {
                fprintf(stderr,"zero-length message received\n");
                return -1;
        }
        SA_common *s=(SA_common*)outbuffer;
        if(s->answerType==sat_nack) {
                fprintf(stderr,"Nack received: \"%s\"\n", ((SA_Nack*)outbuffer)->reason);
                return -1;
        }
        if(s->answerType!=expectedAnswer) {
                fprintf(stderr,"Unexpected answer: %08lx, expected %08lx\n",
                        (unsigned long)s->answerType,
                        (unsigned long)expectedAnswer
                       );
                return -1;
        }
        return 0;
}


void InputThread::Go() {
        //find out if the standard input is a file or a pipe
        ULONG type;
        ULONG attr;
        DosQueryHType((HFILE)0,&type,&attr);
        if((!allowEcho) &&
           (redirectInput && redirectOutput) &&
           (type&0x0080)==0 &&     //standard input is a local thing (not remote)
           (type&0x000f)==1 &&     //standard input is a character device (not file or pipe)
           (attr&0x0001)==1)       //STDIN device
        {
                //Standard input is the local keyboard.
                //Use getch() to prevent echoing and buffering
                //The drawback is that commands entered will first be echoed
                //back from cmd.exe (and others) when [return] is pressed.
                int c;
                while((c=getch())!=26) {        //repeat until ^Z
                        if(pipe->Write(&c,1)==-1)
                                break;
                        if(c==13) {
                                //do CR -> CR+NL translation
                                c=10;
                                pipe->Write(&c,1);
                        }
                }
        } else {
                setmode(fileno(stdin),O_BINARY);
                char buf[1];
                int bytes;
                while((bytes=fread(buf,1,1,stdin))>0) {
                        if(pipe->Write(buf,bytes)==-1)
                                break;
                }
        }
        pipe->DisConnect();
        delete pipe;
}
void OutputThread::Go() {
        setbuf(stdout,NULL);
        char buf[64];
        int bytes;
        while((bytes=pipe->Read(buf,64))>0) {
                fwrite(buf,1,bytes,stdout);
        }
        pipe->DisConnect();
        delete pipe;
}


