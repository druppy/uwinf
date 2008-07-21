#ifndef __FOCUS_HPP
#define __FOCUS_HPP

WmsHWND WmsQueryFocus(WmsHWND desktop);
#define WmsSetFocus(desktop,hwndNewFocus) WmsChangeFocus(desktop,hwndNewFocus,0)
Bool WmsChangeFocus(WmsHWND desktop, WmsHWND hwndNewFocus, uint16 fl);

/*flags for WM_FOCUSCHANGING*/
#define FC_NOSETFOCUS           0x0001
#define FC_NOLOSEFOCUS          0x0002
#define FC_NOACTIVATE           0x0004
#define FC_NODEACTIVATE         0x0008
#define FC_NOBRINGTOTOP         0x0010

#endif
