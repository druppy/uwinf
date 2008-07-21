/* BIF/Thread fan-out header file
 * OS/2 interface
 */

#ifdef BIFINCL_THREADALL
#  define BIFINCL_WINDOWTHREAD
#  define BIFINCL_MAINTHREAD
#  define BIFINCL_SEMAPHORES
#  define BIFINCL_SPINSEMAPHORES
#endif


#ifndef BIFTHREAD_DECLARED
#define BIFTHREAD_DECLARED

//********* Thread ***************************************************
struct FThreadActivationRecord;

class BIFCLASS FThread {
        FThread(const FThread&);                //dont copy
        FThread& operator=(const FThread&);     //dont copy
public:
        FThread(unsigned stackNeeded=0);
        virtual ~FThread();

        virtual int OkToGo();        // Did constructor initialize alright?
        virtual void Go() = 0;        // the real thing
        int Start();                // start the thread

        void SetPriority(int p);
        int GetPriority() const;

        int Suspend();
        int Resume();

        enum state { initialized, running, terminated };
        state GetState() const;

        int Wait();

        int Kill();
protected:
        void Yield() {}
        void Sleep(long ms);

//OS/2 specific:
protected:
        enum { CompilerStart, OS2Start } startMethod;
        int stackNeeded;
        TID threadID;

        virtual int PreGo();
        int _PreGo();
        virtual void PostGo();
        void _PostGo();
private:
        int rememberedPriority;
        state _state;
public:
        TID GetTID() const { return threadID; }
        int Run(FThreadActivationRecord *tar);        //only the thread wiz-bang stuff should call this func.
};


class FMainThread;
class BIFCLASS FThreadManager {
public:
        //enumeration
        static FThread *GetFirstThread();
        static FThread *GetNextThread(FThread *prev);

        static FThread *GetCurrentThread();

        static void Yield() {}
//OS/2 specific:
private:

        static int Insert(FThread *tp);
        static int Remove(FThread *tp);
        friend class FThread;      

        static int Init();
        static void Done();
        friend int BIFFUNCTION _BIFThreadMain(int,char**,FMainThread *(*pfn)(int,char**));
};


#endif /*BIFTHREAD_DECLARED*/


#if defined(BIFINCL_MAINTHREAD) && !defined(__BIFT_MT_H_INCLUDED)
#  include <thread\os2\bift_mt.h>
#endif

#if BIFWS_ == BIFWS_PM_
#  if defined(BIFINCL_WINDOWTHREAD) && !defined(__BIFT_WT_H_INCLUDED)
#    include <thread\os2\pm\bift_wt.h>
#  endif
#
#  if defined(BIFINCL_WINDOWTHREAD) && defined(BIFINCL_MAINTHREAD) && !defined(__BIFT_MWT_H_INCLUDED)
#    include <thread\os2\pm\bift_mwt.h>
#  endif
#else
#  if defined(BIFINCL_WINDOWTHREAD) && !defined(__BIFT_WT_H_INCLUDED)
#    include <thread\os2\wms\bift_wt.h>
#  endif
#
#  if defined(BIFINCL_WINDOWTHREAD) && defined(BIFINCL_MAINTHREAD) && !defined(__BIFT_MWT_H_INCLUDED)
#    include <thread\os2\wms\bift_mwt.h>
#  endif
#endif

//dependency
#if defined(BIFINCL_SPINSEMAPHORES)
#  define BIFINCL_SEMAPHORES
#endif

#if defined(BIFINCL_SEMAPHORES) && !defined(__BIFT_SEM_H_INCLUDED)
#  include <thread\os2\bift_sem.h>
#endif

#if defined(BIFINCL_SPINSEMAPHORES) && !defined(__SPINSEM_H_INCLUDED)
#  include <thread\os2\spinsem.h>
#endif

