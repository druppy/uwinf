/***
Filename: sle.cpp
Description:
  Implementation of FSLE (Single-Line-Edit)
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_SLE
#include <bif.h>

Bool FSLE::Create(FWnd *pParent, int ID, FRect *rect, long style, const char *pszText)
{
        ULONG flStyle=WS_VISIBLE;
        if(style&sle_password)
                flStyle|=ES_UNREADABLE;
        if(style&sle_autohscroll)
                flStyle|=ES_AUTOSCROLL;
        if(style&sle_readonly)
                flStyle|=ES_READONLY;
        if(style&sle_border)
                flStyle|=ES_MARGIN;

        return FControl::Create(pParent,ID,
                                WC_ENTRYFIELD,pszText,
                                flStyle,rect);
}

//clipboard operations
Bool FSLE::Cut() {
        return (Bool)WinSendMsg(GetHwnd(),EM_CUT,0,0);
}

Bool FSLE::Copy() {
        return (Bool)WinSendMsg(GetHwnd(),EM_COPY,0,0);
}
Bool FSLE::Clear() {
        return (Bool)WinSendMsg(GetHwnd(),EM_CLEAR,0,0);
}

Bool FSLE::Paste() {
        return (Bool)WinSendMsg(GetHwnd(),EM_PASTE,0,0);
}

//text operations
Bool FSLE::LimitText(int limit) {
        return (Bool)WinSendMsg(GetHwnd(), EM_SETTEXTLIMIT, MPFROMSHORT(limit),0);
}


//readonly
Bool FSLE::IsReadonly() {
        return (Bool)WinSendMsg(GetHwnd(),EM_QUERYREADONLY,0,0);
}

Bool FSLE::SetReadonly(Bool f) {
        WinSendMsg(GetHwnd(),EM_SETREADONLY,MPFROMSHORT(f),0);
        return True;
}


//selection
Bool FSLE::GetSelection(int *start, int *end) {
        MRESULT rc=WinSendMsg(GetHwnd(), EM_QUERYSEL, 0,0);
        *start=SHORT1FROMMR(rc);
        *end=SHORT2FROMMR(rc);
        return True;
}

Bool FSLE::SetSelection(int start, int end) {
        return (Bool)WinSendMsg(GetHwnd(),EM_SETSEL,MPFROM2SHORT(start,end),0);
}

