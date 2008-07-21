/***
Filename: chkbox.cpp
Description:
  Implementation of FCheckBox
Host:
  Watcom 10.5
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-05-05 Creation (migrated from PM)
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_CHECKBOX
#define WMSINCL_MSGS
#define WMSINCL_CPI
#include <bif.h>

#include "presparm.h"
#include <string.h>
Bool FCheckBox::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, int states) {
        if(states!=2 && states!=3) return False;
        FCheckBox::states = states;
        current_state=0;
        return FLowLevelButton::Create(pParent,ID,
                                       pszText,
                                       0,rect);
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
        return UINT161FROMMR(WmsSendMsg(GetHwnd(), BM_QUERYCHECK,0,0));
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
        WmsSendMsg(GetHwnd(), BM_SETCHECK, MPFROMUINT16(state),0);
        return True;
}


Bool FCheckBox::Paint(WmsHDC hdc, FRect *rect, const char *pszText, Bool isEnabled, Bool isDown, Bool hasFocus) {
        uint8 clr_foreground;
        uint8 clr_background;

        //erase background
        WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground,1);
        WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR, PP_DIALOGBACKGROUNDCOLOR, 0, &clr_background,1);
        CpiEraseRect(hdc, *rect, clr_foreground|clr_background);

        //draw image
        if(isEnabled) {
                if(isDown) {
                        WmsQueryPresParam(GetHwnd(), PP_HILITEFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_HILITEBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clr_background, 1);
                } else {
                        WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR, 0, 0, &clr_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
                }
        } else {
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clr_background, 1);
        }
        char image[3];
        if(current_state==0)
                WmsQueryPresParam(GetHwnd(), PP_CBUNCHECKED, 0, 0, image,3);
        else if(current_state==1)
                WmsQueryPresParam(GetHwnd(), PP_CBCHECKED, 0, 0, image,3);
        else
                WmsQueryPresParam(GetHwnd(), PP_CBUNDETERMINED, 0, 0, image,3);

        CpiDrawText(hdc, image, 3, (FRect*)rect, (color)clr_foreground, color(clr_background>>4), DT_LEFT);

        //draw label
        uint8 clrhot_foreground,clrhot_background;
        if(isEnabled) {
                if(hasFocus) {
                        WmsQueryPresParam(GetHwnd(), PP_FOCUSFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_FOCUSBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clr_background, 1);
                        clrhot_foreground=clr_foreground;
                        clrhot_background=clr_background;
                } else {
                        WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR, 0, 0, &clr_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
                        WmsQueryPresParam(GetHwnd(), PP_MNEMONICFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clrhot_foreground, 1);
                        WmsQueryPresParam(GetHwnd(), PP_MNEMONICBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clrhot_background, 1);
                }
        } else {
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDBACKGROUNDCOLOR, PP_DEFAULTBACKGROUNDCOLOR, 0, &clr_background, 1);
                clrhot_foreground=clr_foreground;
                clrhot_background=clr_background;
        }

        FRect r(*rect);
        r.SetLeft(r.GetLeft() + 4);
        CpiHotkeyTextout(hdc, r, pszText, '@', clr_foreground|clr_background, clrhot_foreground|clrhot_background);

        return True;
}

void FCheckBox::Click() {
        current_state++;
        if(current_state==states)
                current_state=0;
        Invalidate();
}

unsigned FCheckBox::bm_querycheck() {
        return (unsigned)current_state;
}

Bool FCheckBox::bm_setcheck(unsigned newstate) {
        if(newstate>=states) return False;
        current_state = (int)newstate;
        Invalidate();
        return True;
}

Bool FCheckBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FLowLevelButton::ProcessResourceSetup(props,name,value))
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"3state")==0)
                        states = 3;
        }
        return True;
}

