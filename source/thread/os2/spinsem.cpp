#define INCL_NOPMAPI
#define BIFINCL_THREAD
#define BIFINCL_SPINSEMAPHORES
#include <bif.h>

// A few low-level operations which are somewhat compiler-dependent
#ifdef __WATCOMC__
//Watcom C/C++ has a wonderful feature: "pragma aux"
extern "C" void LockedIncrement(unsigned *p);
#pragma aux LockedIncrement = \
".386p                      " \
"lock inc dword ptr [eax]   " \
parm [eax] modify [];

extern "C" void LockedDecrement(unsigned *p);
#pragma aux LockedDecrement = \
".386p                       "\
"lock dec dword ptr [eax]    "\
parm [eax] modify [];

extern "C" unsigned LockedExchange(unsigned *p, unsigned v);
#pragma aux LockedExchange = \
".386p                      "\
"lock xchg ecx,[eax]        "\
parm [eax] [ecx] modify [] value [ecx];
#elif defined(__BORLANDC__)
#error This code has not been tested
//But it should work
#define LockedIncrement(p)      \
  _EAX = (unsigned long)(p);    \
  asm lock inc dword ptr [eax];
#define LockedDecrement(p)      \
  _EAX = (unsigned long)(p);    \
  asm lock inc dword ptr [eax];
#define LockedExchange(p,v)     \
  _ECX = (unsigned long)(p);    \
  _EAX = (unsigned long)(v);    \
  asm lock xchg eax,[ecx]
#elif defined(__IBMCPP__)
//With VAC++ we use an external assembler file as described below
void APIENTRY LockedIncrement(unsigned *p);
void APIENTRY LockedDecrement(unsigned *p);
unsigned APIENTRY LockedExchange(unsigned *p, unsigned v);
#else
#error Unknown compiler
//You can either created your own interface like the two above or
//create this assembler file:
//---CUT---
/*
.386p
fm_TEXT segment para public use32 'CODE'
                assume cs:_TEXT
                assume ds:nothing
LockedIncrement proc    near
                mov     eax,[esp+4]
                lock inc dword ptr [eax]
                ret
LockedIncrement endp

LockedDecrement proc    near
                mov eax,[esp+4]
                lock dec dword ptr [eax]
                ret
LockedDecrement endp

LockedExchange  proc    near
                mov ecx,[esp+4]
                mov eax,[esp+8]
                lock xchg eax,[ecx]
                ret
LockedExchange  endp
                public LockedIncrement
                public LockedDecrement
                public LockedExchange

fm_TEXT    ends

                end
*/
//--CUT--
//And use these prototypes:
//  void APIENTRY LockedIncrement(unsigned *p);
//  void APIENTRY LockedDecrement(unsigned *p);
//  unsigned APIENTRY LockedExchange(unsigned *p, unsigned v);
#endif


static const spin_retries=100;

///////////////////////////////////////////////////////////////////////////////
//FSpinMutex

void FSpinMutexSemaphore::Request() {
        for(unsigned spin=0; spin<spin_retries; spin++) {
                if(LockedExchange(&toggle,1)==0)
                        return;
        }
        LockedIncrement(&waiters);
        for(;;) {
                if(LockedExchange(&toggle,1)==0) {
                        LockedDecrement(&waiters);
                        return;
                }
                try_again.Wait();
                try_again.Reset();
        }
}

void FSpinMutexSemaphore::Release() {
        toggle=0;
        if(waiters!=0)
                try_again.Post();
}

///////////////////////////////////////////////////////////////////////////////
//SpinEvent

void FSpinEventSemaphore::Post() {
        modify.Request();
        if(LockedExchange(&toggle,1)==0)
                wakeup.Post();
        modify.Release();
}

void FSpinEventSemaphore::Reset() {
        modify.Request();
        if(LockedExchange(&toggle,0)==1)
                wakeup.Reset();
        modify.Release();
}

void FSpinEventSemaphore::Wait() {
        for(unsigned spin=0; spin<spin_retries; spin++) {
                if(toggle)
                        return;
        }
        wakeup.Wait();
}

///////////////////////////////////////////////////////////////////////////////
//SpinSemaphore

void FSpinSemaphore::Signal() {
        change_mutex.Request();
        LockedIncrement(&count);
        if(count==1)
                notzero.Post();
        change_mutex.Release();
}

void FSpinSemaphore::Wait() {
        wait_mutex.Request();
        for(unsigned spin=0; spin<spin_retries; spin++) {
                if(count>0) goto OkeyDokey;
        }
        notzero.Wait();
OkeyDokey:
        change_mutex.Request();
        LockedDecrement(&count);
        if(count==0)
          notzero.Reset();
        change_mutex.Release();
        wait_mutex.Release();
}
