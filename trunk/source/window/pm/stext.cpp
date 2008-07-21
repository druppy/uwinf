/***
Filename: stext.cpp
Description:
  Implementation of static text
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_STATICTEXT
#include <bif.h>
//for sprintf only:
#include <stdio.h>

Bool FStaticText::Create(FWnd *pParent, int ID, FRect *rect, long style, const char *pszText)
{
        ULONG flStyle=SS_TEXT;
        if((style&0x000f)==st_left)
                flStyle|=DT_LEFT|DT_TOP|DT_WORDBREAK;
        if((style&0x000f)==st_center)
                flStyle|=DT_CENTER|DT_TOP;
        if((style&0x000f)==st_right)
                flStyle|=DT_RIGHT|DT_TOP;
        if((style&0x000f)==st_leftnowrap)
                flStyle|=DT_LEFT|DT_TOP;
        if(!(style&st_noprefix))
                flStyle|=DT_MNEMONIC;
        return FControl::Create(pParent, ID,
                                WC_STATIC, pszText,
                                flStyle,rect
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

