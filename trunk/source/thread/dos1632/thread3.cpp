/***
Filename: thread3.cpp
Description:
  Implementation of stack allocation, freeing and switching, and yielding
Host:
  WC 10.5 DOS16
History:
  ISJ 96-04-16 Split from thread.cpp
***/
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>
#include <dbglog.h>

#include <stdlib.h>

// FP_OFF, FP_SEG
#include <dos.h>


/***
Visibility: protected
Description:
        longjump to the thread manager multitasking loop
        Should only be called by the current thread itself, otherwise shit
        happens!
***/
void FThread::Yield() {
        TRACE_METHOD1("FThread::Yield()");
        if(setjmp(threadJump)==0) {
                FThreadManager::switchTo();
        }
}



/***
Visibility: global (used by thread2.cpp)
Parameters:
        stackSize       (minimum) size of stack
Return:
        pointer to stack or NULL on error
Description:
        Allocate a memory block of size stackSize. The memory block must be
        useable as a stack.
***/
void *allocateThreadStack(unsigned stackSize) {
        TRACE_PROC0("allocateStack");
        if(stackSize<=0) {
                FATAL("allocateStack: stacksize is 0 !");
        }
        void *p;
        p = malloc(stackSize);
        return p;
}

/***
Visibility: global (used by thread2.cpp)
Parameters:
        stack   pointer to previously allocated stack
Description:
        Free the stack memory block previously allocated with
        allocateThreadStack()
***/
void freeThreadStack(void *stack) {
        TRACE_PROC0("freeStack");
        ASSERT(stack!=0);
        free(stack);
}




/***
Visibility: local to this module
Description:
        Return an aligned SP (stack pointer)
***/
static inline unsigned AlignSP(unsigned sp) {
        //SP is aligned on a 4-byte boundary
        //This can speed up stack operations by up to 200%
        //and some functions require the stack pointer to be 2-byte aligned
#if BIFOS_ == BIFOS_DOS32_
        return sp&0xfffffffc;
#else
        return sp&0xfffc;
#endif
}


#if BIFOS_ == BIFOS_DOS32_
#  ifdef __WATCOMC__
     extern void switchStack(unsigned new_esp);
#    pragma aux switchStack = \
     "mov esp,eax"            \
     "mov ebp,esp"            \
     parm [eax];
#  else
#    error Compiler not recognized
#  endif
#else
#  ifdef __WATCOMC__
     extern void switchStack(unsigned new_ss, unsigned new_sp);
#    pragma aux switchStack = \
     "cli"                    \
     "mov ss,ax"              \
     "mov sp,bx"              \
     "sti"                    \
     "mov bp,bx"              \
     parm [ax] [bx];
#  elif defined(__BORLANDC__) || defined(__SC__)
#    define switchStack(new_ss,new_sp) \
     disable();                        \
     _AX=new_ss;                       \
     asm mov ss,ax                     \
     _SP=new_sp;                       \
     enable();                         \
     _BP=_SP;
#  else
#    error Compiler not recognized
#  endif
#endif

/***
Visibility: private (FThreadManager is friend)
Description:
        This function is static and cannot ever take any parameters
        This function is called by the threa manager when a thread is
        kickstarted. The function only needs to switch to the thread's
        stack and call Run()
***/
void FThread::ThreadEntryPoint() {
        TRACE_FORMAT0("Entering FThread::ThreadEntryPoint");
        //local variables CANNOT be on the stack in this function since we are
        //switching the stack
        static FThread *cr;
        static char *stack_bottom;
        static char *stack_top;

        cr = FThreadManager::GetCurrentThread();
        stack_bottom = (char*)(cr->stack);
        stack_top = stack_bottom + cr->stackNeeded-1;

        static unsigned new_sp;
        new_sp = AlignSP(FP_OFF(stack_top));
#if BIFOS_ == BIFOS_DOS16_
        static unsigned new_ss;
        new_ss = FP_SEG(stack_bottom);

        //switch stack
        switchStack(new_ss,new_sp);
#else
        switchStack(new_sp);
#endif
        cr->Run();
}


/***
Visibility: private (called by ThreadEntryPoint())
Description:
        Set up the new state of the thread and call Go().
        Then commit suicide :-)
***/
void FThread::Run(void) {
        TRACE_FORMAT0("Entering FThread::Run");
        isStarted=1;

        if(PreGo()==0) {
                Go();
                PostGo();
        }

        terminateThread();
        Yield();

        //should never happen:
        FATAL("FThread::Run: Zombie!");
}
