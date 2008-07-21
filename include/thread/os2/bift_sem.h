#ifndef __BIFT_SEM_H_INCLUDED
#define __BIFT_SEM_H_INCLUDED

//****** Synchronization **********************************************

class BIFCLASS FMutexSemaphore {
        HMTX hmtx;
        /* Rep.inv: hmtx is either NULL or a hmtx returned by OS/2 */

        FMutexSemaphore(const FMutexSemaphore&);              //dont copy
        FMutexSemaphore& operator=(const FMutexSemaphore&);   //dont copy
public:
        FMutexSemaphore();
        FMutexSemaphore(int owned);
        virtual ~FMutexSemaphore();

        int Fail();

        int Request(long timeout=-1);
        int Release();

        HMTX QueryHMTX() { return hmtx; }                     //OS/2 only
};


class BIFCLASS FEventSemaphore {
        HEV hev;
        /* Rep.inv: hev is either NULL or a hev returned by OS/2 */

        FEventSemaphore(const FEventSemaphore&);             //dont copy
        FEventSemaphore& operator=(const FEventSemaphore&);  //dont copy
public:
        FEventSemaphore();
        FEventSemaphore(int posted);
        virtual ~FEventSemaphore();

        int Fail();

        int Post();
        int Wait(long timeout=-1);
        int Reset();

        HEV QueryHEV() { return hev; }                       //OS/2 only
};


class BIFCLASS FSemaphore {
        // OS/2 does not provide a general semaphore as proposed by Dijkstra
        // so we have to make it ourselves.
        HMTX wait_mutex;        // Only one thread can wait
        HEV not_zero_event;     // posted if count<>0
        HMTX change_mutex;      // serialize access to count and not_zero_event
        int count;              // the count of the semaphore

        int failed;                //did construction fail?

        FSemaphore(const FSemaphore&);             //dont copy
        FSemaphore& operator=(const FSemaphore&);  //dont copy
public:
        FSemaphore(long initialCount);
        virtual ~FSemaphore();

        int Fail();

        int Signal();
        int Wait(long timeout=-1);
};

#endif
