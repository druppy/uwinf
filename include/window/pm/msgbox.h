#ifndef __MSGBOX_H_INCLUDED
#define __MSGBOX_H_INCLUDED

enum mb_buttons {
        mb_abortretryignore,
        mb_ok,
        mb_okcancel,
        mb_retrycancel,
        mb_yesno,
        mb_yesnocancel
};

enum mb_icon {
        mb_noicon,
        mb_question,
        mb_warning,
        mb_information,
        mb_critical,
        mb_error
};

enum mbid {
        mbid_ok,
        mbid_cancel,
        mbid_abort,
        mbid_retry,
        mbid_ignore,
        mbid_yes,
        mbid_no,
        //mbid_help
        mbid_error
};

mbid BIFFUNCTION MessageBox(FWnd *pOwner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon=mb_noicon);

mbid BIFFUNCTION bw_MessageBox(HWND hwndParent, HWND hwndOwner, const char *pszText, const char *pszTitle, mb_buttons buttons, mb_icon icon=mb_noicon);

mbid BIFFUNCTION bw_MessageBox(HWND hwndParent, HWND hwndOwner, const char *pszText, const char *pszTitle, ULONG idWindow, ULONG fuStyle);

#endif
