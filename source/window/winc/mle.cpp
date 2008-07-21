/***
Filename: mle.cpp
Description:
  Implementation of FMLE (Multi-Line-Eddit)
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-??Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_MLE
#include <bif.h>

Bool FMLE::Create(FWnd *pParent, int ID, FRect *rect, long style, const char *pszText) {
        DWORD dwStyle=ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL;
        if(style&mle_hscroll)
                dwStyle|=WS_HSCROLL;
        if(style&mle_vscroll)
                dwStyle|=WS_VSCROLL;
        if(style&mle_readonly)
                dwStyle|=ES_READONLY;
        if(style&mle_border)
                dwStyle|=WS_BORDER;

        return FControl::Create(pParent,ID,
                                "Edit",pszText,
                                dwStyle,rect);
}



//clipboard operations:
Bool FMLE::Cut() {
        return (Bool)SendMessage(GetHwnd(),WM_CUT,0,0);
}

Bool FMLE::Copy() {
        return (Bool)SendMessage(GetHwnd(),WM_COPY,0,0);
}
Bool FMLE::Clear() {
        return (Bool)SendMessage(GetHwnd(),WM_CLEAR,0,0);
}

Bool FMLE::Paste() {
        return (Bool)SendMessage(GetHwnd(),WM_PASTE,0,0);
}

//selection operations:
Bool FMLE::ReplaceSelection(const char *text) {
        SendMessage(GetHwnd(), EM_REPLACESEL, 0, (LPARAM)(LPSTR)text);
        return True;
}


//modify flag
Bool FMLE::HasBeenModified() {
        return (Bool)SendMessage(GetHwnd(),EM_GETMODIFY,0,0);
}

Bool FMLE::SetModify(Bool modified) {
        SendMessage(GetHwnd(),EM_SETMODIFY,modified,0);
        return True;
}


//text
int FMLE::GetTextLen() {
        return (int)SendMessage(GetHwnd(),WM_GETTEXTLENGTH,0,0);
}

Bool FMLE::GetText(char *buffer, int maxbuf) {
        LRESULT l=SendMessage(GetHwnd(), WM_GETTEXT, maxbuf, (LPARAM)(LPSTR)buffer);
        if(l<=0) return False;
        else return True;
}

Bool FMLE::SetText(const char *text) {
        LRESULT r=SendMessage(GetHwnd(), WM_SETTEXT, 0, (LPARAM)(LPSTR)text);
        if(r!=TRUE)
                return False;
        else
                return True;
}

Bool FMLE::LimitText(int limit) {
        SendMessage(GetHwnd(), EM_LIMITTEXT, limit,0);
        return True;
}


//readonly
Bool FMLE::IsReadonly() {
        if(GetStyle()&ES_READONLY)
                return True;
        else
                return False;
}

Bool FMLE::SetReadonly(Bool f) {
        return (Bool)SendMessage(GetHwnd(),EM_SETREADONLY,f,0);
}

