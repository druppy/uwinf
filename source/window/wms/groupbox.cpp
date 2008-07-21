/***
Filename: grpbox.cpp
Description:
  Implementation of FGroupBox
Host:
  Watcom 10.0a-10.5
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-05-03 Creation. Migrated from PM
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_GROUPBOX
#define WMSINCL_CPI
#include <bif.h>
#include "presparm.h"
#include <string.h>
#include <ctype.h>

Bool FGroupBox::Create(FWnd *pParent, int ID, FRect *rect, const char *pszText)
{
        return FControl::Create(pParent, ID,
                                pszText,
                                WS_GROUP|WS_CLIPSIBLINGS, rect
                               );
}


Bool FGroupBox::Paint() {
        WmsHDC hdc=WmsBeginPaint(GetHwnd());
        if(hdc) {
                char groupchars[8];
                WmsQueryPresParam(GetHwnd(), PP_GBBORDERCHARS, PP_THINBORDERCHARS, 0,groupchars,8);

                color fg_clr,bg_clr;
                WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR,0,0,&fg_clr,1);
                WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR,PP_DIALOGBACKGROUNDCOLOR,0,&bg_clr,1);

                FRect r;
                WmsQueryWindowRect(GetHwnd(),&r);

                CpiFillRect(hdc, r.GetLeft(),   r.GetTop(),     r.GetLeft()+1, r.GetTop()+1,    fg_clr|bg_clr, groupchars[0]);
                CpiFillRect(hdc, r.GetLeft()+1, r.GetTop(),     r.GetRight()-1,r.GetTop()+1,    fg_clr|bg_clr, groupchars[1]);
                CpiFillRect(hdc, r.GetRight()-1,r.GetTop(),     r.GetRight(),  r.GetTop()+1,    fg_clr|bg_clr, groupchars[2]);
                CpiFillRect(hdc, r.GetLeft(),   r.GetTop()+1,   r.GetLeft()+1, r.GetBottom()-1, fg_clr|bg_clr, groupchars[3]);
                CpiFillRect(hdc, r.GetRight()-1,r.GetTop()+1,   r.GetRight(),  r.GetBottom()-1, fg_clr|bg_clr, groupchars[4]);
                CpiFillRect(hdc, r.GetLeft(),   r.GetBottom()-1,r.GetLeft()+1, r.GetBottom(),   fg_clr|bg_clr, groupchars[5]);
                CpiFillRect(hdc, r.GetLeft()+1, r.GetBottom()-1,r.GetRight()-1,r.GetBottom(),   fg_clr|bg_clr, groupchars[6]);
                CpiFillRect(hdc, r.GetRight()-1,r.GetBottom()-1,r.GetRight(),  r.GetBottom(),   fg_clr|bg_clr, groupchars[7]);

                char text[80];
                GetText(text,80);
                CpiSetTextColor(hdc,fg_clr);
                CpiSetTextBackgroundColor(hdc,color(bg_clr>>4));
                CpiOuttext(hdc, r.GetLeft()+1,r.GetTop(), text);

                r.Shrink(1,1);
                CpiEraseRect(hdc, r, fg_clr|bg_clr);

                WmsEndPaint(hdc);
                return True;
        } else
                return False;

}

Bool FGroupBox::MatchMnemonic(char c) {
        char text[80];
        GetText(text,80);
        char *p=strchr(text,'@');
        if(p)
                return (Bool)(tolower(p[1])==tolower(c));
        else
                return False;
}

Bool FGroupBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        //no additional properties, but we need WS_CLIPSIBLINGS style
        SetStyle(~WS_CLIPSIBLINGS,WS_CLIPSIBLINGS);
        return True;
}

