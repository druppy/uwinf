#ifndef __SMTHRD_HPP
#define __SMTHRD_HPP

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#define INCL_DOSQUEUES
#include <bif.h>

/* Session monitoring thread
 * the thread purpose is to detect when a job has finished
 */

class Job;
class SMThread : public FThread {
        HQUEUE hqueue;
        HEV hevWakeup;
        int please_terminate;
public:
        SMThread(HQUEUE h);
        ~SMThread();

        void Go();

        void PleaseTerminate();
};

#endif
