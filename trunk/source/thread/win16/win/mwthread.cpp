/***
Filename: mwthread.cpp
Description:
  Implementation of FMainWindowThread
Host:
  BC31 DOS+WIN, BC40 WIN, WC10 WIN
History:
  ISJ 94-07-08 Creation
  ISJ 95-02-17 Made the stack allocation work in DLLs (DS problem)
  ISJ 95-03-11 Made it DLLable
  ISJ 95-03-14 Fixed a serious bug (was GlobalFree'ing the DS)
  ISJ 96-06-02 Removed DLL support
**/
#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#define BIFINCL_WINDOWTHREAD
#include <bif.h>
#include <dbglog.h>

#include "../dsstuff.h"

static FMainWindowThread *mwthread_mwt=0;

FMainWindowThread *FMainWindowThread::GetMWT() {
        return mwthread_mwt;
}

FMainWindowThread::FMainWindowThread(int argc, char *argv[])
  : FThread(), FMainThread(argc,argv), FWindowThread()
{
        TRACE_METHOD1("FMainWindowThread::FMainWindowThread");

        //Somewhere in the Windows kernel it is assumed that SS==DS, when
        //WndProc's are called. WndProc's are called in the context of the main
        //window thread, so we have to to assure that the main window thread
        //has SS=DS. This is done by allocating the stack from the
        //local heap of the application

        SetDSToSS();
        
        WORD offset=(WORD)LocalAlloc(LMEM_FIXED,stackNeeded);
        if(offset) {
                stackOk=1;
                stack = (void far*)(void near*)offset;
        } else {
                stackOk=0;
        }
        RestoreDS();
        
        mwthread_mwt = this;
}

FMainWindowThread::~FMainWindowThread() {
        TRACE_METHOD1("FMainWindowThread::~FMainWindowThread");

        if(stackOk) {
                SetDSToSS();
                
                LocalFree((HLOCAL)(void near*)stack);

                RestoreDS();
                stack = 0;
        }

        mwthread_mwt = 0;
}

