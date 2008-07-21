#define WMSINCL_CAPTURE
#define WMSINCL_BW
#define WMSINCL_DESKTOP
#include <wms.hpp>
#include "../sysq/hwmouse.hpp"
#include "../wmsmutex.hpp"

#include <bif.h>

static WmsHWND capture_hwnd=0;

WmsHWND WmsQueryCapture(WmsHWND /*desktop*/) {
        if(WmsIsWindow(capture_hwnd))
                return capture_hwnd;
        else
                return 0;
}

Bool WmsSetCapture(WmsHWND /*desktop*/, WmsHWND hwnd) {
        if(hwnd==0) {
                capture_hwnd=0;
                return True;
        }

        int c,r; hwmouse_internal.QueryPosition(&c,&r);
        FPoint p(c,r);

        if(WmsIsWindow(hwnd) &&
           WmsIsWindowVisible(hwnd) &&
           WmsIsWindowEnabled(hwnd)) {
                if(hwmouse_internal.queryCurrentButtonStatus()==0) {
                        //the mouse must be over the window
                        WmsHWND mousewindow=WmsWindowFromPoint(HWND_DESKTOP,&p,True,HWND_TOP);
                        if(mousewindow!=hwnd && !WmsIsChild(hwnd,mousewindow))
                                return False;
                }
                capture_hwnd=hwnd;
                return True;
        } else
                return False;
}

