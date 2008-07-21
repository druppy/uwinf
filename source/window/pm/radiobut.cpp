/***
Filename: rbut.cpp
Description:
  Implementation of FRadioButton
Host:
  Watcom 10.0a-11.0, VAC++ 3.0
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 97-06-28 Modified FindFirstInGroup() to avaid internal compiler error in
               VAC++
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_RADIOBUTTON
#include <bif.h>

Bool FRadioButton::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText) {
        return FControl::Create(pParent,ID,
                                WC_BUTTON,pszText,
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
        return Bool(WinSendMsg(GetHwnd(),BM_QUERYCHECK,0,0)!=0);
}

/***
Visibility: public
Description:
        Check (selected) the readiobutton
***/
Bool FRadioButton::Check() {
        WinSendMsg(GetHwnd(),BM_SETCHECK,MPFROMSHORT(1),0);
        return True;
}

/***
Visibility: public
Description:
        Uncheck (deselected) the readiobutton
***/
Bool FRadioButton::UnCheck() {
        WinSendMsg(GetHwnd(),BM_SETCHECK,MPFROMSHORT(0),0);
        return True;
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
        return SHORT1FROMMR(WinSendMsg(GetHwnd(),BM_QUERYCHECKINDEX,0,0));
}


/***
Visibility: local to this module
Description:
        Return the first window in the group.
***/
static HWND FindFirstInGroup(HWND hwnd) {
        if(!hwnd) hwnd=NULL; //silly reference to avoid VAC interal compiler error
        for(;;) {
                ULONG ulStyle=WinQueryWindowULong(hwnd,QWL_STYLE);
                if(ulStyle&WS_GROUP)
                        return hwnd;
                HWND hwndPrev=hwnd;
                hwnd=WinQueryWindow(hwnd,QW_PREV);
                if(hwnd==NULL)
                        return hwndPrev;
        }
}

/***
Visibility: local to this module
Description
        Figure out wether or not a window is a radiobutton
***/
static Bool IsRadio(HWND hwnd) {
        //check style
        ULONG style=WinQueryWindowULong(hwnd,QWL_STYLE);
        ULONG primaryStyle=style&BS_PRIMARYSTYLES;
        if(primaryStyle!=BS_RADIOBUTTON &&
           primaryStyle!=BS_AUTORADIOBUTTON)
                return False;
        //check class is in the form #nnnn5
        char className[20];
        WinQueryClassName(hwnd,20,className);
        char *p=className;
        if(*p++!='#') return False;
        while(*p=='0') p++;
        if(*p!='5') return False;

        return True;
        //<mangler> wm_querydlgcode
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
                if(IsRadio(hwndCurrent)) {
                        if(index==i) {
                                WinSendMsg(hwndCurrent,BM_SETCHECK,MPFROMSHORT(1),0);
                                return True;
                        } else
                                index++;
                }
                hwndCurrent=WinQueryWindow(hwndCurrent,QW_NEXT);
                if(hwndCurrent==NULL ||                                        //no more controls
                   WinQueryWindowULong(hwndCurrent,QWL_STYLE)&WS_GROUP)        //new group
                {
                        return False;        //radiobutton not found
                }
        }
}


