#if defined(__OS2__) || defined(OS2)
#  define INCL_DOSMISC
#endif
#define WMSINCL_TIMESTAMP
#include <wms.hpp>

#if defined(__DOS__) || defined(__MSDOS__) || defined(MSDOS)
//Use bios ticks - accurate to 18.2th of a second
//circumvent the bios midnight reset.
//This gives a value that does not overflow for 7 years

#include "../dosrmode.hpp"

static unsigned long last_tick=0;
static unsigned long tick_add=0;

TimeStamp WmsQueryTimeStamp(void) {
        unsigned long current_tick = *(Ptr_0040_Type(volatile unsigned long))Ptr_0040(0x006c);
        if(current_tick<last_tick) {
                //bios has reset tick count due to midnight
                tick_add += 1573040UL; // 0x1800B0 ticks per 24 hours
                last_tick = current_tick;
        }
        return current_tick  + tick_add;
}

uint32 WmsConvertTicks2Ms(uint32 ticks) {
        //return ticks/18.20648148*1000;
        return (ticks*10000L+91)/182;
}

uint32 WmsConvertMs2Ticks(uint32 ms) {
        //return (ms/1000L)*18.20648148;
        return (ms*182L+5000)/10000;
}

#elif defined(__OS2__) || defined(OS2)
#include <os2.h>
TimeStamp WmsQueryTimeStamp(void) {
        ULONG t;
        DosQuerySysInfo(QSV_MS_COUNT,QSV_MS_COUNT,&t,sizeof(t));
        return t;
}

uint32 WmsConvertTicks2Ms(uint32 ticks) {
        return ticks;
}

uint32 WmsConvertMs2Ticks(uint32 ms) {
        return ms;
}

#elif defined(_Windows) || || defined(__WINDOWS__) || defined(_WINDOWS) || defined(__NT__)
//win32 assumed
TimeStamp WmsQueryTimeStamp(void) {
        return (TimeStamp)GetTickCount();
}

uint32 WmsConvertTicks2Ms(uint32 ticks) {
        return ticks;
}

uint32 WmsConvertMs2Ticks(uint32 ms) {
        return ms;
}

#else
#error unknow OS
#endif

