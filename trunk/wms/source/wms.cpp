#define WMSINCL_CPI
#define WMSINCL_SYSVALUE
#define WMSINCL_TIMESTAMP
#include <wms.hpp>

#include "sysq/hwmouse.hpp"
#include "sysq/hwkeybrd.hpp"
#include "sysq/sysqueue.hpp"
#include "sysq/sqe_dp.hpp"
#include "misc/paint_dp.hpp"
#include "misc/wmstimer.hpp"


HWMouse                         hwmouse_internal;
HWKeyboard                      hwkeyboard_internal;
SystemEventQueue                systemEventQueue_internal;
SystemQueueEventDispatcher      sqed;
static PaintDispatcherThread    pdt;
WmsTimerThread                  timerthread;

Bool InitializeWMS() {
        //
        //initialize WMS
        //
        //start system queue event dispatcher
        sqed.Start();
        //start hardware keyboard
        hwkeyboard_internal.Initialize();
        //start CPI
        _InitializeCPI();
        //Inform the mouse of the screen size
        hwmouse_internal.SetScreenSize((int)WmsQuerySysValue(0,SV_CXSCREEN),(int)WmsQuerySysValue(0,SV_CYSCREEN));
        //start hardware mouse
        hwmouse_internal.Initialize();
        //start wm_paint dispatcher thread
        pdt.Start();
        //start wm_timer dispatcher thread
        timerthread.Start();
        //set some system values
        uint32 dblclk_ticks = WmsConvertMs2Ticks(300);
        WmsSetSysValue(0,SV_DBLCLKTIME,dblclk_ticks);
        WmsSetSysValue(0,SV_CXDBLCLK,1);
        WmsSetSysValue(0,SV_CYDBLCLK,1);
        return True;
}

Bool TerminateWMS() {
        timerthread.PleaseTerminate();
        pdt.PleaseTerminate();
        sqed.PleaseTerminate();
        hwkeyboard_internal.Terminate();
        hwmouse_internal.HidePointer();
        hwmouse_internal.Terminate();
        pdt.Wait();
        _TerminateCPI();
        sqed.Wait();
        timerthread.Wait();

        return True;
}

