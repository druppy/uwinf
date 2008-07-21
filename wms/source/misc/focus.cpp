#define WMSINCL_FOCUS
#define WMSINCL_BW
#define WMSINCL_MSGQUEUE
#define WMSINCL_MSGS
#include <wms.hpp>

#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#include <bif.h>

static FMutexSemaphore focusMutex;
static WmsHWND hwnd_focus=0;

WmsHWND WmsQueryFocus(WmsHWND /*desktop*/) {
        return hwnd_focus;
}

Bool WmsChangeFocus(WmsHWND /*desktop*/, WmsHWND hwndNewFocus, uint16 fl) {
        if(hwndNewFocus==hwnd_focus)
                return True;
        if(hwndNewFocus!=0 && !(WmsIsWindowVisible(hwndNewFocus) && WmsIsWindowEnabled(hwndNewFocus)))
                return False;

        if(focusMutex.Request(0))
                return False;
        WmsHWND hwndOldFocus=hwnd_focus;

        if(hwndOldFocus!=0)
                WmsSendMsgTimeout(hwndOldFocus,WM_FOCUSCHANGING,MPFROMHWND(hwndNewFocus),MPFROM2UINT16(False,fl),1000);
        hwnd_focus = hwndNewFocus;
        if(hwndNewFocus!=0)
                WmsSendMsgTimeout(hwndNewFocus,WM_FOCUSCHANGING,MPFROMHWND(hwndOldFocus),MPFROM2UINT16(True,fl),1000);

        focusMutex.Release();
        return True;
}

