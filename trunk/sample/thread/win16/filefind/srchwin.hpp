#ifndef __TESTSW_HPP
#define __TESTSW_HPP

#ifndef _WINDOWS_H
#  include <windows.h>
#endif

HWND CreateSearchWindow(HINSTANCE hInst, HWND hWndOwner, char drive, const char *fspec);

#endif
