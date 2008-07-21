#ifndef __WINDOWS_H
#  include <windows.h>
#endif

//messages from thread to search window
#define WM_FILESFOUND		(WM_USER+1)
#define WM_SEARCHFINISHED	(WM_USER+2)

//message from search window to search start window
#define WM_SEARCHCLOSED		(WM_USER+2)	/* wparam=hwnd */

