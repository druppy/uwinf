/***
Filename: dosyield.cpp
Description:
  Implementation of sleeping under various multitaskers
Host:
  WC10.5 DOS16
History:
  ISJ 96-04-14 Creation (split from threadma.cpp)
*/

#include <bif.h>
#include "dosyield.h"
#if defined(__WATCOMC__)
#  include <dos.h>
#  include <i86.h>
#elif defined(__BORLANDC__)
#  include <dos.h>
#else
#  error Insert your compiler-dependent header files here
#endif

static void int_2f_1680() {
        //release rest of current timeslice
        //this works under Win 3.x, DOS 5+, DPMI 1.0+ and OS/2 2.x
        REGS regs;
#if defined(__BORLANDC__) && __BORLANDC__ < 0x450
        //pre-BC40 does not have then 'w' part of the union
        regs.x.ax = 0x1680;
#else
        regs.w.ax = 0x1680;
#endif
#if BIFOS_ == BIFOS_DOS16_
        int86(0x2f,&regs,&regs);
#else
        int386(0x2f,&regs,&regs);
#endif
}

#ifdef __WATCOMC__
   extern void DosSleep(long ms);
#  if BIFOS_ == BIFOS_DOS16_
#    pragma aux DosSleep = \
        "hlt"          \
        "db 035h,0cah" \
        parm [dx ax]   \
        modify [];
#  else
#    pragma aux DosSleep = \
        "mov ax,dx"    \
        "shr edx,16"   \
        "hlt"          \
        "db 035h,0cah" \
        parm [edx]   \
        modify [eax];
#  endif
#elif defined(__BORLANDC__) || defined(__SC__)
#define DosSleep(ms)                    \
        static unsigned ms_low;         \
        static unsigned ms_high;        \
        ms_low = (unsigned)(ms);        \
        ms_high= (unsigned)(ms>>16);    \
        _AX = ms_low;                   \
        _DX = ms_high;                  \
        asm hlt;                        \
        __emit__(0x35,0xca);
#else
#error Unknown compiler
#endif

static void hlt_35_ca(long ms) {
        //OS/2
        //go to sleep - not for the faint of heart !
        //load DX:AX with the number of milliseconds to sleep,
        //do a HLT (!), with two carefully fabricated bytes after it.
        DosSleep(ms);
}

/***
Visibility: global
Parameters:
        approxSleepTime   approx. sleep time in milliseconds
Description:
        Either give up rest of timeslice to any multitasker,
        sleep the specified miliseconds, or do nothing
***/
void _DosYield(long approxSleepTime) {
        static int dosVersionComputed=0;
        static unsigned char dosMajor;
        static unsigned char dosMinor;
        if(!dosVersionComputed) {
                dosVersionComputed=1;
                REGS regs;
                regs.h.ah = 0x30;       //get dos version
                regs.h.al = 0x00;       //oem number
                intdos(&regs,&regs);
                dosMajor = regs.h.al;
                dosMinor = regs.h.ah;
        }
        if(dosMajor!=20) {
                //not OS/2
                int_2f_1680();
        } else {
                //OS/2
                hlt_35_ca(approxSleepTime);
        }
}
