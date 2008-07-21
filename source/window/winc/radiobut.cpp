/***
Filename: rbut.cpp
Description:
  Implementation of FRadioButton
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_RADIOBUTTON
#include <bif.h>

Bool FRadioButton::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText) {
        return FControl::Create(pParent,ID,
                                "Button",pszText,
                                BS_AUTORADIOBUTTON,rect);
}

/***
Visibility: public
Description:
        Return wether or not the readiobutton is checked (selected)
Return
        True  checked
        False not checked
***/
Bool FRadioButton::IsChecked() {
        return Bool(SendMessage(GetHwnd(),BM_GETCHECK,0,0)!=0);
}

/***
Visibility: public
Description:
        Check (selected) the readiobutton
***/
Bool FRadioButton::Check() {
        SendMessage(GetHwnd(),BM_SETCHECK,1,0);
        return True;
}

/***
Visibility: public
Description:
        Uncheck (deselected) the readiobutton
***/
Bool FRadioButton::UnCheck() {
        SendMessage(GetHwnd(),BM_SETCHECK,0,0);
        return True;
}

/***
Visibility: local to this module
Description:
        Return the first window in the group.
***/
static HWND FindFirstInGroup(HWND hwnd) {
        for(;;) {
                if(GetWindowLong(hwnd,GWL_STYLE)&WS_GROUP)
                        return hwnd;
                HWND hwndPrev=hwnd;
                hwnd=GetWindow(hwnd,GW_HWNDPREV);
                if(hwnd==NULL)
                        return hwndPrev;
        }
}

/***
Visibility: public
Description:
        Return the checkindex of the group in which the radiobutton is
Return:
        >=0  check index
        -1   no selected radiobutton
***/
int FRadioButton::GetCheckIndex() {
        int index=0;
        HWND hwndCurrent=FindFirstInGroup(GetHwnd());
        for(;;) {
                LRESULT controlType=SendMessage(hwndCurrent,WM_GETDLGCODE,0,0);
                if(controlType&(DLGC_BUTTON|DLGC_RADIOBUTTON)) {
                        //it's a radiobutton
                        LRESULT checked=SendMessage(hwndCurrent,BM_GETCHECK,0,0);
                        if(checked) {
                                return index;
                        } else
                                index++;
                }
                hwndCurrent=GetWindow(hwndCurrent,GW_HWNDNEXT);
                if(hwndCurrent==NULL ||                                 //no more controls
                   GetWindowLong(hwndCurrent,GWL_STYLE)&WS_GROUP)       //new group
                {
                        return -1;      //no radiobutton checked
                }
        }
}

/***
Visibility: public
Description:
        Check the i'th radiobutton in the group
***/
Bool FRadioButton::CheckIndex(int i) {
        int index=0;
        HWND hwndCurrent=FindFirstInGroup(GetHwnd());
        for(;;) {
                LRESULT controlType=SendMessage(hwndCurrent,WM_GETDLGCODE,0,0);
                if(controlType&(DLGC_BUTTON|DLGC_RADIOBUTTON)) {
                        //it's a radiobutton
                        if(index==i) {
                                SendMessage(hwndCurrent,BM_SETCHECK,1,0);
                                return True;
                        } else
                                index++;
                }
                hwndCurrent=GetWindow(hwndCurrent,GW_HWNDNEXT);
                if(hwndCurrent==NULL ||                                 //no more controls
                   GetWindowLong(hwndCurrent,GWL_STYLE)&WS_GROUP)       //new group
                {
                        return False;   //radiobutton not found
                }
        }
}


