/***
Filename:
  thread1.cpp
Description:
  Implementation of FThread class; thread killing
Host:
  Watcom 9.5
History:
  ISJ 1994-06-14 Creation
***/

#define INCL_NOPMAPI
#define INCL_DOSERRORS
#define BIFINCL_THREAD
#define BIFINCL_THREAD
#include <bif.h>
#include "dbglog.h"

/***
Visibility: public
Description:
  Stop the thread. This is different from suspension because killing
  terminates the thread
***/
int FThread::Kill() {
        TRACE_METHOD1("FThread::Kill");
        if(_state!=running)
                return -1;        //no need to kill at thread that is not running

        APIRET rc = DosKillThread(threadID);
        if(rc==ERROR_BUSY) {
                //the thread is executing 16-bit code
                //wait a second and try again
                DosSleep(1000);
                if(_state==running) {        //still running?
                        rc = DosKillThread(threadID);        //try again
                        if(rc==0) {
                                _state=terminated;
                                return 0;
                        } else {
                                // If the second attempt to kill the thread
                                // fails then we give up
                                // "bad lock - kill() isn't recommended anyway"
                                NOTE_TEXT0("FThread::kill(), DosKillThread() failed");
                                NOTE_TEXT1("FThread::kill: cannot kill thread");
                                return -1;
                        }
                } else
                        return 0;
        } else {
                _state = terminated;
                return 0;
        }
}
