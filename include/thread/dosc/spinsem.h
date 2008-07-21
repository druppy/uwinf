#ifndef __SPINSEM_H_INCLUDED
#define __SPINSEM_H_INCLUDED

class BIFCLASS FSpinMutexSemaphore {
        FMutexSemaphore real_sem;
public:
        FSpinMutexSemaphore() : real_sem() {}
        void Request() { real_sem.Request(); }
        void Release() { real_sem.Release(); }
};

class BIFCLASS FSpinEventSemaphore {
        FEventSemaphore real_sem;
public:
        FSpinEventSemaphore() : real_sem() {}
        void Post()  { real_sem.Post(); }
        void Reset() { real_sem.Reset(); }
        void Wait()  { real_sem.Wait(); }
};

class BIFCLASS FSpinSemaphore {
        FSemaphore real_sem;
public:
        FSpinSemaphore() : real_sem(0) {}
        void Signal() { real_sem.Signal(); }
        void Wait()   { real_sem.Wait(); }
};

#endif

