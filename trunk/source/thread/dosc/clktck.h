#ifndef CLKTCK_H_INCLUDED
#define CLKTCK_H_INCLUDED

#if BIFOS_ == BIFOS_WIN16_
#  define GetCurrentTickCount() GetTickCount()
#  define ClockTicks2ms(ticks) (ticks)
#  define ms2ClockTicks(ms) (ms)
#else
#  include <time.h>
#  define GetCurrentTickCount() clock()
#  define ClockTicks2ms(ticks) (((ticks)*1000L)/CLK_TCK)
#  define ms2ClockTicks(ms) ((((long)(ms))*CLK_TCK)/1000)
#endif

#endif
