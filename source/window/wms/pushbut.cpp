/***
Filename: pushbut.cpp
Description:
  Implementation of FPushButton
Host:
  Watcom 10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-05-08 Creation. Migrated from PM
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_PUSHBUTTON
#define WMSINCL_MSGS
#define WMSINCL_CPI
#include <bif.h>

#include "presparm.h"
#include <string.h>

Bool FPushButton::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, Bool isDefault, CommandType commandType)
{
        defaultButton = isDefault;
        cmdType = commandType;
        return FLowLevelButton::Create(pParent,ID,
                                       pszText,
                                       0,rect);
}


/***
Visibility: public
Description:
        Simulate a push
***/
void FPushButton::Push() {
        WmsPostMsg(GetHwnd(), BM_CLICK, 0,0);
}

Bool FPushButton::Paint(WmsHDC hdc, FRect *rect, const char *pszText, Bool isEnabled, Bool isDown, Bool hasFocus) {
        uint8 clr_foreground;
        uint8 clr_background;

        //erase background
        WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground,1);
        WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR, PP_DIALOGBACKGROUNDCOLOR, 0, &clr_background,1);
        CpiEraseRect(hdc, *rect, clr_foreground|clr_background);

        FRect r;
        if(!isDown) {
                //draw shadow
                WmsQueryPresParam(GetHwnd(), PP_BUTTONSHADOWFOREGROUNDCOLOR, 0, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONSHADOWBACKGROUNDCOLOR, PP_DIALOGBACKGROUNDCOLOR, 0, &clr_background, 1);
                char shadowChars[3];
                WmsQueryPresParam(GetHwnd(), PP_BUTTONSHADOWCHARS, 0, 0, shadowChars, 3);
                r.Set(rect->GetRight()-1, rect->GetTop(), rect->GetRight(), rect->GetTop()+1);
                CpiFillRect(hdc, r, clr_foreground|clr_background, shadowChars[0]);
                r.Set(rect->GetLeft()+1, rect->GetTop()+1, rect->GetRight()-1, rect->GetTop()+2);
                CpiFillRect(hdc, r, clr_foreground|clr_background, shadowChars[1]);
                r.Set(rect->GetRight()-1, rect->GetTop()+1, rect->GetRight(), rect->GetTop()+2);
                CpiFillRect(hdc, r, clr_foreground|clr_background, shadowChars[2]);

                r.Set(rect->GetLeft(), rect->GetTop(), rect->GetRight()-1, rect->GetTop()+1);
        } else
                r.Set(rect->GetLeft()+1, rect->GetTop(), rect->GetRight(), rect->GetTop()+1);

        //draw default indicator chars
        if(defaultButton) {
                char leftd;
                char rightd;
                WmsQueryPresParam(GetHwnd(), PP_BUTTONDEFAULTLEFTCHAR, 0, 0, &leftd, 1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONDEFAULTRIGHTCHAR, 0, 0, &rightd, 1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONDEFAULTFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONDEFAULTBACKGROUNDCOLOR, PP_BUTTONMIDDLEBACKGROUNDCOLOR, 0, &clr_background, 1);
                CpiFillRect(hdc, r.GetLeft(), r.GetTop(), r.GetLeft()+1, r.GetBottom(), clr_foreground|clr_background, leftd);
                CpiFillRect(hdc, r.GetRight()-1, r.GetTop(), r.GetRight(), r.GetBottom(), clr_foreground|clr_background, rightd);
        } else {
                WmsQueryPresParam(GetHwnd(), PP_BUTTONMIDDLEFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clr_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONMIDDLEBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
                CpiFillRect(hdc, r.GetLeft(), r.GetTop(), r.GetLeft()+1, r.GetBottom(), ' ', clr_foreground|clr_background);
                CpiFillRect(hdc, r.GetRight()-1, r.GetTop(), r.GetRight(), r.GetBottom(), ' ', clr_foreground|clr_background);
        }

        //draw text
        uint8 clrhot_foreground,clrhot_background;
        if(hasFocus) {
                WmsQueryPresParam(GetHwnd(), PP_FOCUSFOREGROUNDCOLOR, 0,0, &clr_foreground,1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONMIDDLEBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
                clrhot_foreground=clr_foreground;
                clrhot_background=clr_background;
        } else if(isEnabled) {
                WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR, 0,0, &clr_foreground,1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONMIDDLEBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONMNEMONICFOREGROUNDCOLOR, PP_DEFAULTFOREGROUNDCOLOR, 0, &clrhot_foreground, 1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONMNEMONICBACKGROUNDCOLOR, PP_BUTTONMIDDLEBACKGROUNDCOLOR, 0, &clrhot_background, 1);
        } else {
                WmsQueryPresParam(GetHwnd(), PP_DISABLEDFOREGROUNDCOLOR, 0,0, &clr_foreground,1);
                WmsQueryPresParam(GetHwnd(), PP_BUTTONMIDDLEBACKGROUNDCOLOR, 0, 0, &clr_background, 1);
                clrhot_foreground=clr_foreground;
                clrhot_background=clr_background;
        }
        r.SetLeft(r.GetLeft()+1);
        r.SetRight(r.GetRight()-1);
        CpiEraseRect(hdc, r, clr_background|clr_foreground);

        CpiHotkeyTextout(hdc, r, pszText, '@', clr_foreground|clr_background, clrhot_foreground|clrhot_background);

        return True;
}

void FPushButton::Click() {
        switch(cmdType) {
                case wm_command:
                        WmsPostMsg(WmsQueryWindowOwner(GetHwnd()), WM_COMMAND,    MPFROMUINT16(GetID()), MPFROM2UINT16(CMDSRC_PUSHBUTTON,False));
                        break;
                case wm_syscommand:
                        WmsPostMsg(WmsQueryWindowOwner(GetHwnd()), WM_SYSCOMMAND, MPFROMUINT16(GetID()), MPFROM2UINT16(CMDSRC_PUSHBUTTON,False));
                        break;
                case wm_help:
                        WmsPostMsg(WmsQueryWindowOwner(GetHwnd()), WM_HELP,       MPFROMUINT16(GetID()), MPFROM2UINT16(CMDSRC_PUSHBUTTON,False));
                        break;
        }
}

void FPushButton::GetDownRect(FRect *r) {
        //shrink the rect so it doesn't include the shadow
        r->SetRight(r->GetRight()-1);
        r->SetBottom(r->GetBottom()-1);
}

Bool FPushButton::bm_querydefault() {
        return defaultButton;
}

Bool FPushButton::bm_setdefault(Bool d) {
        defaultButton = d;
        Invalidate();
        return True;
}

Bool FPushButton::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FLowLevelButton::ProcessResourceSetup(props,name,value))
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"default")==0) {
                        defaultButton=True;
                } else if(strcmp(name[p],"command")==0) {
                        cmdType = wm_command;
                } else if(strcmp(name[p],"syscommand")==0) {
                        cmdType = wm_syscommand;
                } else if(strcmp(name[p],"help")==0) {
                        cmdType = wm_help;
                }
        }
        return True;
}
