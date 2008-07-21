/***
Filename: stext.cpp
Description:
  Implementation of static text
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_STATICTEXT
#include <bif.h>

Bool FStaticText::Create(FWnd *pParent, int ID, FRect *rect, long style, const char *pszText)
{
        DWORD dwStyle=0;
        if((style&0x000f)==st_left)
                dwStyle|=SS_LEFT;
        if((style&0x000f)==st_center)
                dwStyle|=SS_CENTER;
        if((style&0x000f)==st_right)
                dwStyle|=SS_RIGHT;
        if((style&0x000f)==st_leftnowrap)
                dwStyle|=SS_LEFTNOWORDWRAP;
        if(style&st_noprefix)
                dwStyle|=SS_NOPREFIX;
        return FControl::Create(pParent, ID,
                                "Static", pszText,
                                dwStyle,rect
                               );
}

Bool FStaticText::SetText(const char *text) {
        return FControlTextHandler::SetText(text);
}

Bool FStaticText::SetText_ul(unsigned long ul) {
        char buf[12];
        wsprintf(buf,"%lu",ul);
        return SetText(buf);
}

Bool FStaticText::SetText_l(long l) {
        char buf[12];
        wsprintf(buf,"%ld",l);
        return SetText(buf);
}

Bool FStaticText::SetText_u(unsigned u) {
        char buf[12];
        wsprintf(buf,"%u",u);
        return SetText(buf);
}

Bool FStaticText::SetText_i(int i) {
        char buf[12];
        wsprintf(buf,"%d",i);
        return SetText(buf);
}

