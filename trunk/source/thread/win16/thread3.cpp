/***
Filename: thread3.cpp
Description:
  Implementation of stack allocation, freeing and switching, and yielding
Host:
  WC 10.5 WIN
History:
  ISJ 96-04-16 Split from thread.cpp
  ISJ 96-06-02 Removed DLL support
***/
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>
#include <dbglog.h>

#include "dsstuff.h"

// FP_OFF, FP_SEG
#include <dos.h>
#ifdef __WATCOMC__
// Watcom has disable() and enable() in this headerfile instead of dos.h
#  include <i86.h>
#endif


/***
Visibility: protected
Description:
        longjump to the thread manager multitasking loop
        Should only be called by the current thread itself, otherwise shit
        happens!
***/
void FThread::Yield() {
        TRACE_METHOD1("FThread::Yield()");
        if(Catch(threadJump)==0) {
                FThreadManager::switchTo();
        }
}



/***
Visibility: local to this module (even if it doesn't seem so)
Return:
        The current stackpointer.
***/
#if defined(__WATCOMC__)
//Watcom does not support the register pseudovariables
//use the pragma aux feature instead
unsigned Get_SP(void);
#pragma aux Get_SP = \
        "mov ax,sp"  \
        value [ax]
#elif defined(__BORLANDC__)
//Use the _SP pseudo-variable
static inline unsigned Get_SP(void) {
        return _SP;
}
#elif defined(__SC__)
#define Get_SP() ((unsigned)_SP)
#else
#error unknown compiler
#endif


/***
Visibility: local to this module
Parameters:
        stackSize       (minimum) size of stack
        clear           flag wether or not to clear the stack to zeros
Return:
        pointer to stack or NULL on error
Description:
        Allocate a memory block of size stackSize. The memory block must be
        useable as a stack.
Note:
        LocalAlloc stuff needs casting so that Symantec C++ insert DS into the
        pointer.
***/
void *allocateThreadStack(unsigned stackSize) {
        TRACE_PROC0("allocateStack");
        if(stackSize<=0) {
                FATAL("allocateStack: stacksize is 0 !");
        }
#ifdef STACK_IN_DS
        WORD offset = (WORD)LocalAlloc(LMEM_FIXED,stackSize);
        return offset ? (void near*)offset : 0;
#else
        return (void far*)GlobalLock(GlobalAlloc(GMEM_MOVEABLE,stackSize));
#endif
}

/***
Visibility: local to this module
Parameters:
        stack   pointer to previously allocated stack
Description:
        free the stack memory block previously allocated with allocateStack()
***/
void freeThreadStack(void *stack) {
        TRACE_PROC0("freeStack");
        ASSERT(stack!=0);
#ifdef STACK_IN_DS
        LocalFree((HLOCAL)stack);
#else
        HGLOBAL hglb = (HGLOBAL)HIWORD(stack);
        GlobalUnlock(hglb);
        GlobalFree(hglb);
#endif
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
        return sp&0xfffc;
}



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
        static int new_sp;
        static int new_ss;
        static char *stack_bottom;
        static char *stack_top;

        cr = FThreadManager::GetCurrentThread();
        stack_bottom = (char*)(cr->stack);
        stack_top = stack_bottom + cr->stackNeeded-1;

        new_sp = AlignSP(FP_OFF(stack_top));
        new_ss = FP_SEG(cr->stack);

        //switch stack
#ifdef __WATCOMC__
        //Watcom doesn't have inline assembler
        extern void switchStack(unsigned new_ss, unsigned new_sp);
#pragma aux switchStack = \
        "mov ss,ax"       \
        "mov sp,bx"       \
        "mov bp,bx"       \
        parm [ax] [bx]

        _disable();
        switchStack(new_ss,new_sp);
        _enable();
#else
        disable();      //disable interrupts
          _AX=new_ss;
          asm mov SS,AX
          _SP=new_sp;
        enable();       //enable interrupts
        _BP=_SP;        //really not necessary, but debuggers like it
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

        Go();

        terminateThread();
        Yield();

        //should never happen:
        FATAL("FThread::Run: the thread is dead!, Yield() should not return!");
}

