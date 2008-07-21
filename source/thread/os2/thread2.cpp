/***
Filename : thread2.cpp
Description:
  Implementation of Thread class under OS/2, priority part only
History:
  ISJ 1994-05-14 Creation
***/

#define INCL_NOPMAPI
#define BIFINCL_THREAD
#include <bif.h>
#include "dbglog.h"


/***
Visibility: public
Parameters:
  p  a priority in the range -15..15
Description:
  Set the priority of the thread.
***/
void FThread::SetPriority(int p) {
        TRACE_METHOD1("FThread::SetPriority");
        if(_state!=running) {
                NOTE_TEXT0("FThread::SetPriority(): thread is not started");
                return;        //the priority can only be changed for running threads
        }
        if(p<-15) {
                WARN_TEXT("FThread::SetPriority(): prty<-15");
                p=-15;
        }
        if(p>15) {
                WARN_TEXT("FThread::SetPriority(): prty>15");
                p=15;
        }

        rememberedPriority = p;        // Remember the priority for use in
                                        // getPriority. This is necessary because OS/2
                                        // does not allow us to retrieve the priority
                                        // of another thread

        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib,&ppib);  //Note: gets info for calling thread
        if(ptib->tib_version!=20 ||
           ptib->tib_ptib2->tib2_version!=20) 
        {
                //The TIB is not 2.0 format - abort
                NOTE_TEXT0("FThread::SetPriority(): TIB version is not 20");
                return;
        }
        ULONG prtyClass;
        if(ptib->tib_ptib2->tib2_ulpri<256+16)
                prtyClass = PRTYC_IDLETIME;
        else if(ptib->tib_ptib2->tib2_ulpri<512+16)
                prtyClass = PRTYC_REGULAR;
        else if(ptib->tib_ptib2->tib2_ulpri<768+16)
                prtyClass = PRTYC_TIMECRITICAL;
        else if(ptib->tib_ptib2->tib2_ulpri<1024+16)
                prtyClass = PRTYC_FOREGROUNDSERVER;
        else {
                NOTE_TEXT0("FThread::SetPriority(): could not guess priority class, aborting");
                return;
        }

        if(p<0) {
                //reduce to lower class
                if(prtyClass==PRTYC_IDLETIME)
                        p=0;    //can't be lower
                else {
                        prtyClass--;
                        p+=32;
                }
        }

        APIRET rc = DosSetPriority(PRTYS_THREAD, prtyClass, p, (ULONG)threadID);
        if(rc!=0) {
                NOTE_TEXT0("FThread::SetPriority(), DosSetPriority() failed");
        }
}



/***
Visibility: public
Return value:
  the priority previously set with setPriority() or 0 if it has never
  been set.
***/
int FThread::GetPriority() const {
        TRACE_METHOD1("FThread::GetPriority");
        if(_state!=running) return 0;

        return rememberedPriority;
}
