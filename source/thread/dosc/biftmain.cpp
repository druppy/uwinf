/***
Filename: bifmain.cpp
Description:
  _BIFMain() function
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-07 Creation
  ISJ 94-12-09 Added __WINDOWS__ stuff to support Watcom
  ISJ 95-02-03 Changed saved hinst...nCmdShow from globals to functions
               Changed MakeArgcArgv to use stack buffer for arguments
  ISJ 95-03-11 Made it DLLable
  ISJ 96-06-02 Removed DLL support
**/

#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>
#include "dbglog.h"

#if BIFOS_ == BIFOS_WIN16_
#include <stdlib.h>
#include "../win16/dsstuff.h"


static HINSTANCE saved_hInstance;
static HINSTANCE saved_hPrevInstance;
static LPSTR     saved_lpszCmdLine;
static int       saved_nCmdShow;

HINSTANCE BIFFUNCTION BIFTGet_hInstance() {
        return saved_hInstance;
}
HINSTANCE BIFFUNCTION BIFTGet_hPrevInstance() {
        return saved_hPrevInstance;
}
LPSTR     BIFFUNCTION BIFTGet_lpszCmdLine() {
        return saved_lpszCmdLine;
}
int       BIFFUNCTION BIFTGet_nCmdShow() {
        return saved_nCmdShow;
}


/***
Visibility: local to this module
Parameters:
        hInst           handle to the application
        lpszCmdLine     command line string
        argc            (output) argument count
        argv            (output) argument vector
        argv_buf        (output) buffer arguments
Description:
        Create argc and argv from the stupid Windows hinstance and lpCmdLine
***/
static void MakeArgcArgv(HINSTANCE hInst, LPSTR lpszCmdLine,
                         int *argc, char **argv, char *argv_buf
                        )
{
        TRACE_PROC0("MakeArgcArgv");

        //create argv[0]
        //ANSI says program name or ""
        //We choose program name
        GetModuleFileName(hInst, argv_buf, 140);
        argv[0]=argv_buf;
        *argc=1;

        LPSTR src=lpszCmdLine;
        char *dst=argv_buf+lstrlen(argv_buf)+1;

        //skip initial blanks in command line
        while(*src==' ' || *src=='\t')
                src++;

        while(*src) {
                //store argument start in vector
                argv[*argc]=dst;
                ++*argc;

                //copy until blank is found
                while(*src && *src!=' ' && *src!='\t') {
                        *dst++=*src++;
                }
                *dst++='\0';    //terminate with '\0'

                //skip trailing blanks
                while(*src==' ' || *src=='\t')
                        src++;
        }
        argv[*argc]=0;   //terminate vector with a NULL pointer
}


/***
Visibility: global
Description:
        This function is the main entry point in BIF/Thread. It will create
        argc+argv, make the main thread and start the multitasking.
***/
int BIFFUNCTION _BIFThreadMain(HINSTANCE hInst,
                               HINSTANCE hPrevInst,
                               LPSTR lpszCmdLine,
                               int nCmdShow,
                               FMainThread *(*pfn)(int,char**))
{
        TRACE_PROC1("_BIFThreadMain");

        saved_hInstance     = hInst;
        saved_hPrevInstance = hPrevInst;
        saved_lpszCmdLine   = lpszCmdLine;
        saved_nCmdShow      = nCmdShow;

        int argc;
        char *argv[1+64+1];
        char argv_buf[140+128];

        MakeArgcArgv(hInst,lpszCmdLine, &argc,argv,argv_buf);

        FMainThread *mt=pfn(argc,argv);
        if(!mt) {
                NOTE_TEXT1("_BIFThreadMain: FMainThread::MakeMainThread() returned NULL");
                return -1;
        }

        if(mt->Start()) {
                NOTE_TEXT1("_BIFThreadMain: Could not start main thread");
                delete mt;
                return -1;
        }

        FThreadManager::SetMainThread(mt);

        FThreadManager::ServiceThreads();

        int r=mt->retcode;
        delete mt;
        return r;
}

#else
// DOS ----------------------------------------------------------------------

/***
See the Windows version
***/

int BIFFUNCTION _BIFThreadMain(int argc, char **argv,FMainThread *(*pfn)(int,char**)) {
        TRACE_PROC1("_BIFMain");
        FMainThread *mt=pfn(argc,argv);
        if(!mt) {
                NOTE_TEXT1("_BIFThreadMain: FMainThread::MakeMainThread() returned NULL");
                return -1;
        }

        if(mt->Start()) {
                NOTE_TEXT1("_BIFThreadMain: Could not start main thread");
                delete mt;
                return -1;
        }

        FThreadManager::SetMainThread(mt);

        FThreadManager::ServiceThreads();

        int r=mt->retcode;
        delete mt;
        return r;
}
#endif
