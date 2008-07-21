/***
Filename: pushbut.cpp
Description:
  Implementation of FPushButton
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_PUSHBUTTON
#include <bif.h>

Bool FPushButton::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, Bool isDefault)
{
        ULONG flStyle=WS_VISIBLE|BS_PUSHBUTTON;
        if(isDefault)
                flStyle|=BS_DEFAULT;
        return FControl::Create(pParent,ID,
                                WC_BUTTON,pszText,
                                flStyle,rect);
}


/***
Visibility: public
Description:
        Simulate a push
***/
void FPushButton::Push() {
        WinPostMsg(GetHwnd(),BM_CLICK,MPFROMSHORT(TRUE),0);
}

