#ifndef __BIFT_SEM_H_INCLUDED
#define __BIFT_SEM_H_INCLUDED

//****** Synchronization **********************************************
#ifndef __BIFT_WAITTHING_H_INCLUDED
#include <thread\bift_waitthing.h>
#endif


class BIFCLASS FMutexSemaphore : private FWaitThing {
        FMutexSemaphore(const FMutexSemaphore&);              //dont copy
        FMutexSemaphore& operator=(const FMutexSemaphore&);   //dont copy
public:
        FMutexSemaphore();
        FMutexSemaphore(int owned);
        virtual ~FMutexSemaphore();

        int Fail();

        int Request(long timeout=-1);
        int Release();
private:
        int failed;     //an owner thread died
        FThread *owner;
        unsigned requests; //nesting
        FMutexSemaphore *nextOwned;     //link to next mutex owned by 'owner'
        friend class FThread;
};


class BIFCLASS FEventSemaphore : private FWaitThing {
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
private:
        int posted;
};


class BIFCLASS FSemaphore : private FWaitThing {
        FSemaphore(const FSemaphore&);             //dont copy
        FSemaphore& operator=(const FSemaphore&);  //dont copy
public:
        FSemaphore(long initialCount);
        virtual ~FSemaphore();

        int Fail();

        int Signal();
        int Wait(long timeout=-1);
private:
        long count;
};

#endif
