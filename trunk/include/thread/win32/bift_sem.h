#ifndef __BIFT_SEM_H_INCLUDED
#define __BIFT_SEM_H_INCLUDED

//****** Synchronization **********************************************

class BIFCLASS FMutexSemaphore {
        HANDLE hMutex;

        FMutexSemaphore(const FMutexSemaphore&);              //dont copy
        FMutexSemaphore& operator=(const FMutexSemaphore&);   //dont copy
public:
        FMutexSemaphore();
        FMutexSemaphore(int owned);
        virtual ~FMutexSemaphore();

        int Fail();

        int Request(long timeout=-1);
        int Release();
};


class BIFCLASS FEventSemaphore {
        HANDLE hEvent;
        /* Rep.inv: hEvent is either NULL or a handle returned by win32 */

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
};


class BIFCLASS FSemaphore {
        HANDLE hSem;
        /* Rep.inv: hSem is either NULL or a handle returned by win32 */

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