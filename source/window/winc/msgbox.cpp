/***
Filename: msgbox.cpp
Description:
  Implementation of MessageBoxes
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
  ISJ 95-02-28 Changed call to application's messagebox function
***/
#define BIFINCL_WINDOW
#define BIFINCL_MESSAGEBOX
#define BIFINCL_APPLICATION
#include <bif.h>

mbid BIFFUNCTION MessageBox(FWnd *pOwner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon)
{
        HWND hwndOwner;
        if(pOwner) {
                hwndOwner=pOwner->GetHwnd();
        } else
                hwndOwner=NULL;

        return MessageBox(hwndOwner,pszText,pszTitle,buttons,icon);
}

mbid BIFFUNCTION MessageBox(HWND owner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon) {
        UINT fuStyle=0;
        switch(buttons) {
                case mb_abortretryignore: fuStyle|= MB_ABORTRETRYIGNORE; break;
                case mb_ok:               fuStyle|= MB_OK; break;
                case mb_okcancel:         fuStyle|= MB_OKCANCEL; break;
                case mb_retrycancel:      fuStyle|= MB_RETRYCANCEL; break;
                case mb_yesno:            fuStyle|= MB_YESNO; break;
                case mb_yesnocancel:      fuStyle|= MB_YESNOCANCEL;
        }
        switch(icon) {
                case mb_noicon:       break;
                case mb_question:     fuStyle|= MB_ICONQUESTION; break;
                case mb_warning:      fuStyle|= MB_ICONEXCLAMATION;
                case mb_information:  fuStyle|= MB_ICONINFORMATION; break;
                case mb_critical:     fuStyle|= MB_ICONHAND; break;
                case mb_error:        fuStyle|= MB_ICONHAND; break;
        }
        return bw_MessageBox(owner, pszText, pszTitle, fuStyle);
}

mbid BIFFUNCTION bw_MessageBox(HWND hwnd, const char *pszText, const char *pszTitle, UINT fuStyle) {
        int answer;
        answer= GetCurrentApp()->DoMessageBox(hwnd,pszText,pszTitle,fuStyle);
        switch(answer) {
                case IDABORT:  return mbid_abort;
                case IDCANCEL: return mbid_cancel;
                case IDIGNORE: return mbid_ignore;
                case IDNO:     return mbid_no;
                case IDOK:     return mbid_ok;
                case IDRETRY:  return mbid_retry;
                case IDYES:    return mbid_yes;
                default:
                        return mbid_error;
        }
}

