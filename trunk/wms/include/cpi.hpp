#ifndef __CPI_HPP
#define __CPI_HPP

class Region;

Bool CpiQueryCellAttr(WmsHDC hdc, const FPoint &p, int *a);
Bool CpiQueryCellChar(WmsHDC hdc, const FPoint &p, char *c);
Bool CpiQueryCellData(WmsHDC hdc, const FPoint &p, uint16 *d);
void CpiSetCellAttr(WmsHDC hdc, const FPoint &p, int a);
void CpiSetCellChar(WmsHDC hdc, const FPoint &p, char c);
void CpiSetCellData(WmsHDC hdc, const FPoint &p, uint16 d);

enum bkmode_t { bm_transparent, bm_opaque };
void CpiSetBkMode(WmsHDC hdc, bkmode_t m);
enum color {
        clr_black,    clr_blue,         clr_green,      clr_cyan,
        clr_red,      clr_magenta,      clr_brown,      clr_lightgray,
        clr_darkgray, clr_lightblue,    clr_lightgreen, clr_lightcyan,
        clr_lightred, clr_lightmagenta, clr_yellow,     clr_white
};
void CpiSetTextColor(WmsHDC hdc, color c);
void CpiSetTextBackgroundColor(WmsHDC hdc, color c);
FPoint CpiOuttext(WmsHDC hdc, int x, int y, const char *psz);
inline FPoint CpiOuttext(WmsHDC hdc, const FPoint &p, const char *psz) {
        return CpiOuttext(hdc,p.x,p.y,psz);
}
FPoint CpiHotkeyTextout(WmsHDC hdc, int lx, int ty, int rx, int by, const char *psz, char hotkeyChar, uint8 normColor, uint8 hotkeyColor);
inline FPoint CpiHotkeyTextout(WmsHDC hdc, const FRect &r, const char *psz, char hotkeyChar, uint8 normColor, uint8 hotkeyColor) {
        return CpiHotkeyTextout(hdc, r.xLeft,r.yTop,r.xRight,r.yBottom, psz, hotkeyChar, normColor, hotkeyColor);
}


enum {
        DT_LEFT       = 0x0000,
        DT_CALCRECT   = 0x0002,
        DT_TEXTATTRS  = 0x0040,
        DT_CENTER     = 0x0100,
        DT_RIGHT      = 0x0200,
        DT_WORDBREAK  = 0x4000,
        DT_ERASERECT  = 0x8000
};
Bool CpiDrawText(WmsHDC hdc, const char *psz, int textlen, FRect *rect, color fore, color back, uint32 format);

void CpiFillRect(WmsHDC hdc, int xLeft, int yTop, int xRight, int yBottom, int color, char c);
inline void CpiFillRect(WmsHDC hdc, const FRect &r, int color, char c) {
        CpiFillRect(hdc,r.xLeft,r.yTop,r.xRight,r.yBottom,color,c);
}
inline void CpiEraseRect(WmsHDC hdc, const FRect &r, int color) {
        CpiFillRect(hdc,r,color,' ');
}
inline void CpiEraseRect(WmsHDC hdc, int xLeft, int yTop, int xRight, int yBottom, int color) {
        CpiFillRect(hdc,xLeft,yTop,xRight,yBottom,color,' ');
}

Bool CpiSetClip(WmsHDC hdc, const Region &newClip);
Bool CpiQueryRegion(WmsHDC hdc, Region *rgn);



WmsHDC WmsGetScreenDC();
WmsHDC WmsGetWindowDC(WmsHWND hwnd, uint32 fl);
WmsHDC WmsBeginPaint(WmsHWND hwnd, FRect *paintRect=0);
void WmsEndPaint(WmsHDC hdc);
void WmsReleaseDC(WmsHDC hdc);

Bool _CpiSetCursorPos(WmsHWND hwnd, int x, int y);
Bool _CpiSetCursorShape(int start, int end);
Bool _CpiShowCursor();
Bool _CpiHideCursor();

Bool _InitializeCPI();
Bool _TerminateCPI();

#define CPICAP_ATTRIBUTEFORMAT          0x0003
#define CPICAP_ATTRIBUTEMONO            0x0001
#define CPICAP_ATTRIBUTECOLOR168        0x0002
#define CPICAP_ATTRIBUTECOLOR1616       0x0003
#define CPICAP_CHARACTERS               0x001c
#define CPICAP_CHARACTERS_00_1f         0x0004
#define CPICAP_CHARACTERS_20_7f         0x0008
#define CPICAP_CHARACTERS_80_ff         0x0010
#define CPICAP_FULLBOXDRAW              0x0020

uint32 CpiQueryCapabilities();

#endif /*__CPI_HPP*/
