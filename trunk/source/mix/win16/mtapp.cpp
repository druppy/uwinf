#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_THREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_MAINTHREAD
#include <bif.h>

Bool FMTApplication::GetEvent(FEvent &ev) {
        ev.SetResult(0);
        return (Bool)mwt->GetMessage(&ev);
}

int FMTApplication::DoMessageBox(HWND hwnd, const char *pszText, const char *pszTitle, UINT fuStyle) {
        return BTMessageBox(hwnd,pszText,pszTitle,fuStyle);
}

