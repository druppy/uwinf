#ifdef BIFINCL_THREADALL
#  define BIFINCL_WINDOWTHREAD
#  define BIFINCL_MAINTHREAD
#  define BIFINCL_SEMAPHORES
#  define BIFINCL_SPINSEMAPHORES
#endif


#ifndef BIFTHREAD_DECLARED
#define BIFTHREAD_DECLARED

//********* Thread ***************************************************

#ifndef __BIFT_WAITTHING_H_INCLUDED
#  include <thread\dosc\bift_waitthing.h>
#endif

class BIFCLASS FThreadManager;
class BIFCLASS FMutexSemaphore;

class BIFCLASS FThread : private FWaitThing {
        FThread(const FThread&);                //dont copy
        FThread& operator=(const FThread&);     //dont copy
public:
        FThread(unsigned stackNeeded=8192);
        virtual ~FThread();

        virtual int OkToGo();   // Did constructor initialize alright?
        virtual void Go() = 0;  // the real thing
        int Start();            // start the thread

        void SetPriority(int p);
        int GetPriority() const;

        int Suspend();
        int Resume();

        enum state { initialized, running, terminated };
        state GetState() const;

        int Wait();     

        int Kill();
protected:
        void Yield();
        void Sleep(long ms);
//win16 stuff:
private:
        void terminateThread();
        int thePriority; int tmpPriority;
        state theState;
        int isStarted;
        int suspendCount;
        int isSleeping; unsigned long timeoutEndTime;
protected://MainWindowThread needs to access these two members
        void *stack;
        unsigned stackNeeded;
private:
        CATCHBUF threadJump;

        //wait thing stuff
        FWaitThing *waitsFor;
        FThread *nextWaiter, *prevWaiter;
        int waitReturnCode;
        friend class FWaitThing;
        FMutexSemaphore *firstOwnedMutex;
        friend class FMutexSemaphore;
        unsigned TID;
        //thread manager stuff
        FThread *prevThread,*nextThread;
        void Run(void);
        static void ThreadEntryPoint();
public: //stupid compiler...
        enum runningSubState { rst_ready, rst_timeout, rst_waiting };
        //win16/dos only:
        unsigned GetTID() const { return TID; }
private:
        runningSubState GetSubState() const;
        friend class FThreadManager;
};


class BIFCLASS FMainThread;
class BIFCLASS FThreadManager {
public:
        //enumeration
        static FThread *GetFirstThread();
        static FThread *GetNextThread(FThread *prev);

        static FThread *GetCurrentThread();

        static void Yield() { GetCurrentThread()->Yield(); }
//win16 specific members:
private:
        static void switchTo(); //switch to thread manager
        static int AddThread(FThread *tp);
        static int RemoveThread(FThread *tp);
        static int ChangeThread(FThread *tp);
        friend class FThread;
        friend class FWaitThing;        //needs to call ChangeThread in GenericWait()

        static void SetExternalWaitThing(FExternalWaitThing *);
        static void UnsetExternalWaitThing();
        friend class FExternalWaitThing;

        static void removeFromChain(FThread *&base, FThread *tp);
        static void insertIntoChain(FThread *&base, FThread *tp);
        static int  findInChain(FThread *base, FThread *tp);

        static void changeTimeoutThreads();
        static void changeWaitingThreads();
        static void waitForAnyThread();
        static void getNextThreadToRun();

        static void ServiceThreads();
        static void SetMainThread(FThread *mt);
        friend BIFFUNCTION _BIFThreadMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow,FMainThread *(*pfn)(int,char**));
};


#endif /*BIFTHREAD_DECLARED*/

#ifdef BIFINCL_SPINSEMAPHORE
#  define BIFINCL_SEMAPHORES
#endif

#ifdef BIFINCL_MAINTHREAD
#  include <thread\dosc\bift_mt.h>
#endif

#ifdef BIFINCL_SEMAPHORES
#  include <thread\dosc\bift_sem.h>
#endif

#ifdef BIFINCL_SPINSEMAPHORES
#  include <thread\dosc\spinsem.h>
#endif


#ifdef BIFINCL_WINDOWTHREAD
#  include <thread\win16\win\bift_wt.h>
#endif


#if defined(BIFINCL_WINDOWTHREAD) && defined(BIFINCL_MAINTHREAD)
#  include <thread\win16\win\bift_mwt.h>
#endif

HINSTANCE BIFFUNCTION BIFTGet_hInstance();
HINSTANCE BIFFUNCTION BIFTGet_hPrevInstance();
LPSTR     BIFFUNCTION BIFTGet_lpszCmdLine();
int       BIFFUNCTION BIFTGet_nCmdShow();
