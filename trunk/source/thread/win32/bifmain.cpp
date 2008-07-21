/***
Filename: bifmain.cpp
Description:
  Implementation of _BIFThreadMain()
Host:
  Watcom 10.0a
History:
  ISJ 95-07-18 Creation (copied from OS/2 sources)
***/
#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>
#include "dbglog.h"

static HINSTANCE saved_hInstance=0;
static HINSTANCE saved_hPrevInstance=0;
static LPSTR     saved_lpszCmdLine=0;
static int       saved_nCmdShow=0;

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
int BIFFUNCTION _BIFThreadMain(int argc, char *argv[], FMainThread *(*pfn)(int,char**)) {
        TRACE_PROC1("_BIFMain");

        //initialize ThreadManager
        if(FThreadManager::Init())
                return -1;

        int r;
        FMainThread *mainThread = pfn(argc,argv);
        if(mainThread) {
                mainThread->Run(0);
                r = mainThread->retcode;
                delete mainThread;
        } else
                r=-1;

        FThreadManager::Done();

        return r;
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
Note:
        This is a copy-and-paste from window/win32/applicat.cpp
        This coode should not be duplicated
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

        //create the rest of the arguments
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

int BIFFUNCTION _BIFThreadMain(HINSTANCE hinst, HINSTANCE hinstprev, LPSTR lpCmdLine, int nCmdShow, FMainThread *(*pfn)(int,char**)) {
        saved_hInstance = hinst;
        saved_hPrevInstance=hinstprev;
        saved_lpszCmdLine=lpCmdLine;
        saved_nCmdShow=nCmdShow;

        int argc;
        char **argv;
        char *argv_buf;
        argv_buf = new char[256+lstrlen(lpCmdLine)+1];
        argv = new char*[1+lstrlen(lpCmdLine)/2+1];
        if(argv!=0 && argv_buf!=0) {
                MakeArgcArgv(hinst,lpCmdLine, &argc,argv,argv_buf);
                int r=_BIFThreadMain(argc,argv,pfn);
                delete[] argv;
                delete[] argv_buf;
                return r;
        } else
                return -1;

}


