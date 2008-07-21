/***
Filename: pushbut.cpp
Description:
  Implementation of FPushButton
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_PUSHBUTTON
#include <bif.h>

Bool FPushButton::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, Bool isDefault)
{
        DWORD dwStyle=0;
        if(isDefault)
                dwStyle|=BS_DEFPUSHBUTTON;
        else
                dwStyle|=BS_PUSHBUTTON;
        return FControl::Create(pParent,ID,
                                "Button",pszText,
                                dwStyle,rect);
}


/***
Visibility: public
Description:
        Simulate a push
***/
void FPushButton::Push() {
#if BIFOS_==BIFOS_WIN16_
        PostEvent(FEvent(GetParent()->GetHwnd(),WM_COMMAND,GetID(),MAKELPARAM(GetHwnd(),BN_CLICKED)));
#else
        PostEvent(FEvent(GetParent()->GetHwnd(),WM_COMMAND,MAKEWPARAM(GetID(),BN_CLICKED),(WPARAM)GetHwnd()));
#endif
}

