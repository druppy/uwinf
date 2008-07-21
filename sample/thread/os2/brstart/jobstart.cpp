#define INCL_DOSSESMGR
#define INCL_DOSMISC
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include "job.hpp"
#include "jobstart.hpp"
#include "brserver.hpp"
#include "srvpipe.hpp"
#include "joblist.hpp"
#include "brlimits.hpp"
#include <string.h>
//stdio for sprintf() only
#include <stdio.h>

//Use a mutex to protext the process-global things such as current directory,
//current drive etc.
FMutexSemaphore sessionstart_mutex;

int StartJob(Job *job, char *errmsg) {
        sessionstart_mutex.Request();

        STARTDATA startdata;
        startdata.Length = 32; //to and including sessiontype
        //We want to pick up the result code, so the session must be a related
        //child session
        startdata.Related = SSF_RELATED_CHILD;
        //Start the session in the foreground (if we are in the background the
        //shell will automatically override this)
        startdata.FgBg = SSF_FGBG_FORE;
        //no tracing (we are not a debugger)
        startdata.TraceOpt = SSF_TRACEOPT_NONE;
        if(job->niceName[0])
                startdata.PgmTitle = job->niceName;
        else
                startdata.PgmTitle = (PSZ)0; //let the shell establish the session title
        startdata.PgmName = job->command;

        char parm[MAXPARAMETERLENGTH+1];
        char *d=parm;
        char *s=job->parameters;
        while(s[0] || s[1]) {
                if(*s)
                        *d++=*s++;
                else {
                        *d++=' ';
                        s++;
                }
        }
        d[0]='\0';
        d[1]='\0';

        startdata.PgmInputs = parm[0] ? parm : (PSZ)0;

        startdata.TermQ = resultCodeQueueName;
        startdata.Environment = 0;
        startdata.InheritOpt = SSF_INHERTOPT_PARENT;
        startdata.SessionType = SSF_TYPE_DEFAULT;

        //change directory and drive
        char oldDir[26][256];
        ULONG oldDisk;
        {
                for(int i=0; i<26; i++) {
                        if(job->currentDir[i][0]) {
                                ULONG cbBuf=256;
                                DosQueryCurrentDir(i+1, (PBYTE)oldDir[i], &cbBuf);
                                DosSetCurrentDir(job->currentDir[i]);
                        }
                }
                if(job->currentDrive[0]) {
                        ULONG dontCare;
                        ULONG drive=job->currentDrive[0];
                        if(drive>='A' && drive<='Z')
                                drive = drive-'A'+1;
                        else
                                drive = drive-'a'+1;
                        DosQueryCurrentDisk(&oldDisk,&dontCare);
                        DosSetDefaultDisk(drive);
                }
        }
        stdio_mutex.Request();
        HFILE oldInput=(HFILE)-1;
        if(job->inputPipeName[0]) {
                DosDupHandle(0,&oldInput);
                HFILE newInput=(HFILE)0;
                DosDupHandle(job->inputPipe->QueryHandle(),&newInput);
        }
        HFILE oldOutput=(HFILE)-1;
        HFILE oldErrorOutput=(HFILE)-1;
        if(job->outputPipeName[0]) {
                DosDupHandle(1,&oldOutput);
                HFILE newOutput=(HFILE)1;
                DosDupHandle(job->outputPipe->QueryHandle(),&newOutput);
                DosDupHandle(2,&oldErrorOutput);
                HFILE newErrorOutput=(HFILE)2;
                DosDupHandle(job->outputPipe->QueryHandle(),&newErrorOutput);
        }

        APIRET rc = DosStartSession(&startdata, &job->idSession, &job->pid);

        if(job->outputPipeName[0]) {
                HFILE newOutput=(HFILE)1;
                DosDupHandle(oldOutput,&newOutput);
                DosClose(oldOutput);
                HFILE newErrorOutput=(HFILE)2;
                DosDupHandle(oldErrorOutput,&newErrorOutput);
                DosClose(oldErrorOutput);
        }
        if(job->inputPipeName[0]) {
                HFILE newInput=(HFILE)0;
                DosDupHandle(oldInput,&newInput);
                DosClose(oldInput);
        }
        stdio_mutex.Release();
        //return to old directory and drive
        {
                for(int i=0; i<26; i++) {
                        if(job->currentDir[i][0]) {
                                ULONG cbBuf=256;
                                DosSetCurrentDir(oldDir[i]);
                        }
                }
                if(job->currentDrive[0])
                        DosSetDefaultDisk(oldDisk);
        }

        int r;
        //...now examine rc
        if(rc!=0 &&
           rc!=ERROR_SMG_START_IN_BACKGROUND
          ) 
        {
                //error occurred
                ULONG msglen;
                char msg[128];
                DosGetMessage((PCHAR*)0,
                              0,
                              (PCHAR)msg,
                              128,
                              (ULONG)rc,
                              (PSZ)"oso001.msg",
                              &msglen
                             );
                sprintf(errmsg,"rc=%ld, %s",(unsigned long)rc, msg);
                r=-1;
        } else {
                job->started = 1;
                jobList.AddJob(job);
                r=0;
        }
        sessionstart_mutex.Release();
        return r;
}
