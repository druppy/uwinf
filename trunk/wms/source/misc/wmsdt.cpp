#define WMSINCL_DESKTOP
#define WMSINCL_BW
#include <wms.hpp>

static WmsHWND desktop=0;

WmsHWND WmsQueryDesktop() {
        return desktop;
}

void WmsSetDesktop(WmsHWND hwnd) {
        desktop=hwnd;
}

