/***
Filename: chkbox.cpp
Description:
  Implementation of FCheckBox
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_CHECKBOX
#include <bif.h>

Bool FCheckBox::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, int states) {
        ULONG flStyle=WS_VISIBLE;
        if(states==2)
                flStyle|=BS_AUTOCHECKBOX;
        else
                flStyle|=BS_AUTO3STATE;
        return FControl::Create(pParent,ID,
                                WC_BUTTON,pszText,
                                flStyle,rect);
}


/***
Visibility: public
Description:
        Return the state of the checkbox
Return:
        0 unchecked
        1 checked
        2 undetermined
***/
int FCheckBox::GetCheck() {
        return SHORT1FROMMR(WinSendMsg(GetHwnd(), BM_QUERYCHECK,0,0));
}

/***
Visibility: public
Description:
        Setthe state of the checkbox
Parameters:
        state 0 unchecked
              1 checked
              2 undertermined
Return:
        True on success, False otherwise
***/
Bool FCheckBox::SetCheck(int state) {
        WinSendMsg(GetHwnd(), BM_SETCHECK,MPFROMSHORT(state),0);
        return True;
}

