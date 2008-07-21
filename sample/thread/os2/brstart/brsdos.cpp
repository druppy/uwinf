#ifdef __OS2__
#define INCL_DOSPROCESS
#include <os2.h>
#define MySleep(seconds) DosSleep(seconds*1000)
#else
#include <dos.h>
#define MySleep(seconds) sleep(seconds)
#endif

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "protocol.hpp"
#include "clnpipe.hpp"
#include <string.h>

void showUsage();
int processParameters(int artc, char **argv);
int execute();

int main(int argc, char **argv) {
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


void showUsage() {
        printf("brstart v1.0\n");
        printf("usage: brstart [option...]\n");
        printf("  -prg<program>    Which program to start\n");
        printf("  -parm<parameter> A parameter to the program\n");
        printf("  -cwd<cwd>        Current working directory\n");
        printf("  -cd<drive>       Current drive\n");
        printf("  -oredir          Redirect output\n");
        printf("  -iredir          Redirect input\n");
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

int processParameters(int argc, char **argv) {
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

        if(redirectInput && redirectOutput) {
                fprintf(stderr,"input AND output redirection not supported in this version of brstart\n");
                return -1;
        }

        return 0;
}


int MyTransact(ClientIOPipe *pipe,
               const void *inbuffer, int inbuflen,
               void *outbuffer, int outbuflen,
               unsigned long expectedAnswer
              );

int execute() {
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
                MySleep(1); //give the server enough time to issue a pipe::connect
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
                MySleep(1); //give the server enough time to issue a pipe::connect
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
                        if(outputRedirectionPipe) {
                                setbuf(stdout,NULL);
                                printf("Porgram output\n");
                                char buf[64];
                                int bytes;
                                while((bytes=outputRedirectionPipe->Read(buf,64))>0) {
                                        fwrite(buf,1,bytes,stdout);
                                }
                                outputRedirectionPipe->DisConnect();
                                delete outputRedirectionPipe;
                                outputRedirectionPipe=0;
                        }
                        if(inputRedirectionPipe) {
                                /*setbuf(stdin,NULL);*/
                                setmode(fileno(stdin),O_BINARY);
                                printf("Enter program input:\n");
                                char buf[64];
                                int bytes;
                                while((bytes=fread(buf,1,1,stdin))>0) {
                                        if(inputRedirectionPipe->Write(buf,bytes)==-1)
                                                break;
                                }
                                inputRedirectionPipe->DisConnect();
                                delete inputRedirectionPipe;
                                inputRedirectionPipe=0;
                        }
                        if(waitForJobToFinish) {
                                CR_Wait wait;
                                if(MyTransact(&pipe, &wait,sizeof(wait), com_buffer,4096, sat_resultcode)==0) {
                                        rc = (int)((SA_ResultCode*)com_buffer)->code;
                                } else
                                        rc=-1;
                        } else
                                rc=0;
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

