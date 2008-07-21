#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#define WMSINCL_BW
#define WMSINCL_MSGS
#include <wms.hpp>

#include "paint_dp.hpp"
#include "../bw/bw_int.hpp"

void PaintDispatcherThread::Go() {
        WmsHAB hab=WmsInitialize(0);
        for(;;) {
                WmsHWND hwnd = WmsQueryInvalidWindow();
                if(hwnd==0) break;
                WmsPostMsg(hwnd,WM_PAINT,0,0);
        }
        WmsTerminate(hab);
}

void PaintDispatcherThread::PleaseTerminate() {
        WmsStopPaintQueue_internal();
}

