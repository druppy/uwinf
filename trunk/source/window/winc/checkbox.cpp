/***
Filename: chkbox.cpp
Description:
  Implementation of FCheckBox
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_CHECKBOX
#include <bif.h>

Bool FCheckBox::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, int states) {
        DWORD dwStyle=0;
        if(states==2)
                dwStyle|=BS_AUTOCHECKBOX;
        else
                dwStyle|=BS_AUTO3STATE;
        return FControl::Create(pParent,ID,
                                "Button",pszText,
                                dwStyle,rect);
}


int FCheckBox::GetCheck() {
        return (int)SendMessage(GetHwnd(), BM_GETCHECK,0,0);
}

Bool FCheckBox::SetCheck(int state) {
        SendMessage(GetHwnd(), BM_SETCHECK,state,0);
        return True;
}

