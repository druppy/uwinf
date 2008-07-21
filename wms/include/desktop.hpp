#ifndef __DESKTOP_HPP
#define __DESKTOP_HPP

#define HWND_DESKTOP (WmsQueryDesktop())

WmsHWND WmsQueryDesktop();
void    WmsSetDesktop(WmsHWND hwnd);

#endif
