#ifndef __CAPTURE_HPP
#define __CAPTURE_HPP

WmsHWND WmsQueryCapture(WmsHWND desktop);
Bool    WmsSetCapture(WmsHWND desktop, WmsHWND hwnd);
#define WmsReleaseCapture(desktop) WmsSetCapture(desktop,0)

/*hit-test codes (return from WM_HITTEST)*/
#define HT_DISCARD      0x0000
#define HT_NORMAL       0x0001
#define HT_TRANSPARENT  0x0002
#define HT_ERROR        0x0003

#endif

