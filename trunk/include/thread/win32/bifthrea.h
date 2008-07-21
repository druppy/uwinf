/* BIF/Thread fan-out header file
 * Win32 interface
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
//undef the yield macro
#undef Yield

struct FThreadActivationRecord;

class BIFCLASS FThread {
        FThread(const FThread&);                //dont copy
        FThread& operator=(const FThread&);     //dont copy
public:
        FThread(unsigned stackNeeded=0);
        virtual ~FThread();

        virtual int OkToGo();           // Did constructor initialize alright?
        virtual void Go() = 0;          // the real thing
        int Start();                    // start the thread

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

//Win32 specific:
protected:
        enum { CompilerStart, Win32Start } startMethod;
        int stackNeeded;
        DWORD threadID;
        HANDLE threadHandle;
        friend class FThreadManager;
        virtual int PreGo();
        int _PreGo();
        virtual void PostGo();
        void _PostGo();
private:
        int rememberedPriority;
        state _state;
public:
        DWORD GetTID() const;
        HANDLE GetHandle() const;
        virtual int Run(FThreadActivationRecord *tar);    //only the thread wiz-bang stuff should call this func.

};


class FMainThread;
class BIFCLASS FThreadManager {
public:
        //enumeration
        static FThread *GetFirstThread();
        static FThread *GetNextThread(FThread *prev);

        static FThread *GetCurrentThread();

        static void Yield() {}
//Win32 specific:
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
#  include <thread\win32\bift_mt.h>
#endif

#if BIFWS_ == BIFWS_WIN_
#  if defined(BIFINCL_WINDOWTHREAD) && !defined(__BIFT_WT_H_INCLUDED)
#    include <thread\win32\win\bift_wt.h>
#  endif
#
#  if defined(BIFINCL_WINDOWTHREAD) && defined(BIFINCL_MAINTHREAD) && !defined(__BIFT_MWT_H_INCLUDED)
#    include <thread\win32\win\bift_mwt.h>
#  endif
#else
#  if defined(BIFINCL_WINDOWTHREAD) && !defined(__BIFT_WT_H_INCLUDED)
#    include <thread\win32\wms\bift_wt.h>
#  endif
#
#  if defined(BIFINCL_WINDOWTHREAD) && defined(BIFINCL_MAINTHREAD) && !defined(__BIFT_MWT_H_INCLUDED)
#    include <thread\win32\wms\bift_mwt.h>
#  endif
#endif

//dependency
#if defined(BIFINCL_SPINSEMAPHORES)
#  define BIFINCL_SEMAPHORES
#endif

#if defined(BIFINCL_SEMAPHORES) && !defined(__BIFT_SEM_H_INCLUDED)
#  include <thread\win32\bift_sem.h>
#endif

#if defined(BIFINCL_SPINSEMAPHORES) && !defined(__SPINSEM_H_INCLUDED)
#  include <thread\win32\spinsem.h>
#endif

HINSTANCE BIFFUNCTION BIFTGet_hInstance();
HINSTANCE BIFFUNCTION BIFTGet_hPrevInstance();
LPSTR     BIFFUNCTION BIFTGet_lpszCmdLine();
int       BIFFUNCTION BIFTGet_nCmdShow();
