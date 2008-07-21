#ifndef __SPINSEM_H_INCLUDED
#define __SPINSEM_H_INCLUDED

class BIFCLASS FSpinMutexSemaphore {
        LONG toggle;
        LONG waiters;
        FEventSemaphore try_again;
public:
        FSpinMutexSemaphore() : toggle(0), waiters(0), try_again() {}
        void Request();
        void Release();
};

class BIFCLASS FSpinEventSemaphore {
        LONG toggle;
        FEventSemaphore wakeup;
        FSpinMutexSemaphore modify;
public:
        FSpinEventSemaphore() : toggle(0), wakeup(0), modify() { }
        void Post();
        void Reset();
        void Wait();
};

class BIFCLASS FSpinSemaphore {
        LONG count;
        FEventSemaphore notzero;
        FSpinMutexSemaphore wait_mutex,change_mutex;
public:
        FSpinSemaphore()
          : count(0),
            notzero(0),
            wait_mutex(),
            change_mutex()
          {}
        void Signal();
        void Wait();
};

#endif

