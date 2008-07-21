/***
Filename: sle.cpp
Description:
  Implementation of FSLE (Single-Line-Edit)
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_SLE
#include <bif.h>

Bool FSLE::Create(FWnd *pParent, int ID, FRect *rect, long style, const char *pszText)
{
        DWORD dwStyle=0;
        if(style&sle_password)
                dwStyle|=ES_PASSWORD;
        if(style&sle_autohscroll)
                dwStyle|=ES_AUTOHSCROLL;
        if(style&sle_readonly)
                dwStyle|=ES_READONLY;
        if(style&sle_border)
                dwStyle|=WS_BORDER;

        return FControl::Create(pParent,ID,
                                "Edit",pszText,
                                dwStyle,rect);
}

///////////////////////
// Clipboard operations

Bool FSLE::Cut() {
        return (Bool)SendMessage(GetHwnd(),WM_CUT,0,0);
}

Bool FSLE::Copy() {
        return (Bool)SendMessage(GetHwnd(),WM_COPY,0,0);
}
Bool FSLE::Clear() {
        return (Bool)SendMessage(GetHwnd(),WM_CLEAR,0,0);
}

Bool FSLE::Paste() {
        return (Bool)SendMessage(GetHwnd(),WM_PASTE,0,0);
}

//////////////////
// text operations
Bool FSLE::LimitText(int limit) {
        SendMessage(GetHwnd(), EM_LIMITTEXT, limit,0);
        return True;
}


///////////
// readonly
Bool FSLE::IsReadonly() {
        if(GetStyle()&ES_READONLY)
                return True;
        else
                return False;
}

Bool FSLE::SetReadonly(Bool f) {
        return (Bool)SendMessage(GetHwnd(),EM_SETREADONLY,f,0);
}


//selection
Bool FSLE::GetSelection(int *start, int *end) {
        LRESULT l=SendMessage(GetHwnd(), EM_GETSEL, 0,0);
        *start=LOWORD(l);
        *end=HIWORD(l);
        return True;
}

Bool FSLE::SetSelection(int start, int end) {
        return (Bool)SendMessage(GetHwnd(),EM_SETSEL,0,MAKELPARAM(start,end));
}

