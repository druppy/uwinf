/***
Filename: msgbox.cpp
Description:
  Implementation of MessageBoxes
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_MESSAGEBOX
#include <bif.h>

mbid MessageBox(FWnd *pOwner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon)
{
        HWND hwndOwner;
        if(pOwner) {
                hwndOwner=pOwner->GetHwnd();
        } else
                hwndOwner=NULL;

        return bw_MessageBox(HWND_DESKTOP,hwndOwner,pszText,pszTitle,buttons,icon);
}

mbid bw_MessageBox(HWND hwndParent, HWND hwndOwner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon) {
        ULONG flStyle=MB_MOVEABLE;
        switch(buttons) {
                case mb_abortretryignore: flStyle|= MB_ABORTRETRYIGNORE; break;
                case mb_ok:               flStyle|= MB_OK; break;
                case mb_okcancel:         flStyle|= MB_OKCANCEL; break;
                case mb_retrycancel:      flStyle|= MB_RETRYCANCEL; break;
                case mb_yesno:            flStyle|= MB_YESNO; break;
                case mb_yesnocancel:      flStyle|= MB_YESNOCANCEL;
        }
        switch(icon) {
                case mb_noicon:       break;
                case mb_question:     flStyle|= MB_ICONQUESTION; break;
                case mb_warning:      flStyle|= MB_ICONEXCLAMATION;
                case mb_information:  flStyle|= MB_INFORMATION; break;
                case mb_critical:     flStyle|= MB_ICONHAND; break;
                case mb_error:        flStyle|= MB_ERROR; break;
        }
        return bw_MessageBox(hwndParent, hwndOwner, pszText, pszTitle, 999, flStyle);
}

mbid bw_MessageBox(HWND hwndParent, HWND hwndOwner, const char *pszText, const char *pszTitle, ULONG idWindow, ULONG flStyle) {
        int answer;
        answer= WinMessageBox(hwndParent,hwndOwner,pszText,pszTitle,idWindow,flStyle);
        switch(answer) {
                case MBID_ABORT:  return mbid_abort;
                case MBID_CANCEL: return mbid_cancel;
                case MBID_IGNORE: return mbid_ignore;
                case MBID_NO:     return mbid_no;
                case MBID_OK:     return mbid_ok;
                case MBID_RETRY:  return mbid_retry;
                case MBID_YES:    return mbid_yes;
                default:
                        return mbid_error;
        }
}

