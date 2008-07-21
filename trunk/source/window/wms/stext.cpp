/***
Filename: stext.cpp
Description:
  Implementation of static text
Host:
  Watcom 10.0a-10.5
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-05-01 Migrated from PM to WMS
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_STATICTEXT
#define WMSINCL_CPI
#include <bif.h>
#include "presparm.h"
//for sprintf only:
#include <stdio.h>
#include <string.h>
#include <ctype.h>

Bool FStaticText::Create(FWnd *pParent, int resID) {
        return FControl::Create(pParent,resID);
}

Bool FStaticText::Create(FWnd *pParent, int ID, FRect *rect, long style, const char *pszText)
{
        uint32 flStyle=0;
        if((style&0x000f)==st_left)
                flStyle|=DT_LEFT|DT_WORDBREAK;
        if((style&0x000f)==st_center)
                flStyle|=DT_CENTER;
        if((style&0x000f)==st_right)
                flStyle|=DT_RIGHT;
        if((style&0x000f)==st_leftnowrap)
                flStyle|=DT_LEFT;
        //todo:
        //if(!(style&st_noprefix))
        //        flStyle|=DT_MNEMONIC;
        return FControl::Create(pParent, ID,
                                pszText,
                                flStyle, rect
                               );
}

Bool FStaticText::SetText(const char *text) {
        return FControlTextHandler::SetText(text);
}

Bool FStaticText::SetText_ul(unsigned long ul) {
        char buf[12];
        sprintf(buf,"%lu",ul);
        return SetText(buf);
}

Bool FStaticText::SetText_l(long l) {
        char buf[12];
        sprintf(buf,"%ld",l);
        return SetText(buf);
}

Bool FStaticText::SetText_u(unsigned u) {
        char buf[12];
        sprintf(buf,"%u",u);
        return SetText(buf);
}

Bool FStaticText::SetText_i(int i) {
        char buf[12];
        sprintf(buf,"%d",i);
        return SetText(buf);
}

Bool FStaticText::Paint() {
        WmsHDC hdc=WmsBeginPaint(GetHwnd());
        if(hdc) {
                char text[80];
                char *tp;
                int tl=GetTextLen();
                if(tl<80) {
                        GetText(text,80);       //use stack buffer
                        tp=text;
                } else {
                        tp=new char[tl+1];      //use heap buffer
                        if(!tp) {
                                WmsEndPaint(hdc);
                                return False;
                        }
                        GetText(tp,tl+1);
                }

                FRect r;
                WmsQueryWindowRect(GetHwnd(),&r);
                uint8 fg_clr,bg_clr;
                WmsQueryPresParam(GetHwnd(),PP_DEFAULTFOREGROUNDCOLOR,0,0,&fg_clr,1);
                WmsQueryPresParam(GetHwnd(),PP_DEFAULTBACKGROUNDCOLOR,PP_DIALOGBACKGROUNDCOLOR,0,&bg_clr,1);
                long dt=(long)WmsQueryWindowStyle(GetHwnd());
                dt = (dt&0xffff)|DT_ERASERECT;
                CpiDrawText(hdc,
                            tp,
                            tl,
                            &r,
                            (color)fg_clr,
                            (color)(bg_clr>>4),
                            dt
                           );

                if(tp!=text) delete[] tp;
                WmsEndPaint(hdc);
        }
        return True;
}

Bool FStaticText::MatchMnemonic(char c) {
        char text[256];
        GetText(text,256);
        char *p=strchr(text,'@');
        if(p)
                return (Bool)(tolower(p[1])==tolower(c));
        else
                return False;
}

Bool FStaticText::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"text")==0 && value[p]) {
                        SetText(value[p]);
                } else if(strcmp(name[p],"left")==0) {
                        SetStyle(~(DT_LEFT|DT_RIGHT|DT_CENTER|DT_WORDBREAK),DT_LEFT|DT_WORDBREAK);
                } else if(strcmp(name[p],"right")==0) {
                        SetStyle(~(DT_LEFT|DT_RIGHT|DT_CENTER|DT_WORDBREAK),DT_RIGHT);
                } else if(strcmp(name[p],"center")==0) {
                        SetStyle(~(DT_LEFT|DT_RIGHT|DT_CENTER|DT_WORDBREAK),DT_CENTER);
                } else if(strcmp(name[p],"leftnowrap")==0) {
                        SetStyle(~(DT_LEFT|DT_RIGHT|DT_CENTER|DT_WORDBREAK),DT_LEFT);
//                } else if(strcmp(name[p],"noprefix")==0) {
//                        SetStyle(~(DT_MNENOMIC),0);
                }
        }
        return True;
}


