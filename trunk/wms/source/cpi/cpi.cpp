#if defined(__OS2__) || defined(OS2)
#  define INCL_VIO
#  define INCL_DOSPROCESS
#endif
#define WMSINCL_BW
#define WMSINCL_CPI
#define WMSINCL_SYSVALUE
#define WMSINCL_REGION
#define WMSINCL_DESKTOP
#include <wms.hpp>

#include "hdcmap.hpp"
#include "../wmsmutex.hpp"
#include "../sysq/hwmouse.hpp"

#include <mem.h>
#include <string.h>

static unsigned screen_width;
static unsigned screen_height;
static uint16 *screen;
static WmsHDC WmsHDC_sequencer=(WmsHDC)0;
static HDCMap hdcMap;
static WmsMutex DCMutex;
static uint32 capabilities=0;

uint32 CpiQueryCapabilities() {
        return capabilities;
}

class DCLock {
public:
        DCLock() { DCMutex.Request(); }
        ~DCLock() { DCMutex.Release(); }
};


//We rely on BIF to detect target environment
#if (BIFOS_ == BIFOS_DOS16_) || (BIFOS_ == BIFOS_DOS32_)
#  include "dos.hpp"
#elif (BIFOS_ == BIFOS_OS2_)
#  include "os2.hpp"
#else
#  error unknown OS
#endif




struct DeviceContext {
public:
        DeviceContext(const FPoint &off, const Region &r, int sl,int el);

        WmsHDC hdc;
        FPoint offset;          //offset in screen
        Region rgnAlwaysClip;   //always clip
        Region rgnCurrentClip;  //current clipping (setclip+alwaysclip)

        //text attributes
        color textColor;
        color textBackgroundColor;
        bkmode_t backgroundMode;
        int startLine,endLine;
        int firstModifyLine,lastModifyLine;

        void recordModify(int y) {
                if(firstModifyLine>y) firstModifyLine=y;
                if(lastModifyLine<=y) lastModifyLine=y+1;
        }
};





DeviceContext::DeviceContext(const FPoint &off, const Region &r, int sl,int el)
  : offset(off),
    rgnAlwaysClip(),
    rgnCurrentClip(),
    textColor(clr_lightgray),
    textBackgroundColor(clr_black),
    backgroundMode(bm_opaque),
    startLine(sl),
    endLine(el),
    firstModifyLine(32000), lastModifyLine(-1)
{
        rgnAlwaysClip.Copy(r);
        rgnCurrentClip.Copy(r);
}



Bool CpiQueryCellAttr(WmsHDC hdc, const FPoint &p, int *a) {
        uint16 d;
        if(!CpiQueryCellData(hdc,p,&d))
                return False;
        *a=(int)((d>>8)&0xff);
        return True;
}

Bool CpiQueryCellChar(WmsHDC hdc, const FPoint &p, char *c) {
        uint16 d;
        if(!CpiQueryCellData(hdc,p,&d))
                return False;
        *c=(char)(d&0xff);
        return True;
}

inline void nocheck_CpiQueryCellData(DeviceContext *dc, int x,int y, uint16 *d) {
        *d=screen[(y+dc->offset.y)*screen_width+x+dc->offset.x];
}


Bool CpiQueryCellData(WmsHDC hdc, const FPoint &p, uint16 *d) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return False;
        if(!dc->rgnCurrentClip.Includes(p))
                return False;
        nocheck_CpiQueryCellData(dc,p.x,p.y,d);
        return True;
}


inline void nocheck_CpiSetCellData(DeviceContext *dc, int x,int y, uint16 d) {
        screen[(y+dc->offset.y)*screen_width+x+dc->offset.x]=d;
}

inline void nocheck_CpiSetCellDataRange(DeviceContext *dc, int x1,int x2,int y, uint16 d) {
        uint16 *p=screen+(y+dc->offset.y)*screen_width+x1+dc->offset.x;
        for(unsigned i=x1; i<x2; i++)
                *p++ = d;
}

void CpiSetCellAttr(WmsHDC hdc, const FPoint &p, int a) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        uint16 d;
        if(!dc->rgnCurrentClip.Includes(p))
                return;
        nocheck_CpiQueryCellData(dc,p.x,p.y,&d);
        d = (uint16)((d&0xff) | (a<<8));
        nocheck_CpiSetCellData(dc,p.x,p.y,d);
        dc->recordModify(p.y);
}

void CpiSetCellChar(WmsHDC hdc, const FPoint &p, char c) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        uint16 d;
        if(!dc->rgnCurrentClip.Includes(p))
                return;
        nocheck_CpiQueryCellData(dc,p.x,p.y,&d);
        d = (uint16)((d&~0xff)|c);
        nocheck_CpiSetCellData(dc,p.x,p.y,d);
        dc->recordModify(p.y);
}

void CpiSetCellData(WmsHDC hdc, const FPoint &p, uint16 d) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return;
        if(!dc->rgnCurrentClip.Includes(p))
                return;
        nocheck_CpiSetCellData(dc,p.x,p.y,d);
        dc->recordModify(p.y);
}




void CpiSetBkMode(WmsHDC hdc, bkmode_t m) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return;
        dc->backgroundMode=m;
}
void CpiSetTextColor(WmsHDC hdc, color c) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return;
        dc->textColor=c;
}
void CpiSetTextBackgroundColor(WmsHDC hdc, color c) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return;
        dc->textBackgroundColor=c;
}




FPoint CpiOuttext(WmsHDC hdc, int x, int y, const char *psz) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return FPoint(x,y);
        FPoint p(x,y);
        FRect bounds=dc->rgnCurrentClip.QueryBoundsRect();
        int color=dc->textBackgroundColor<<4|dc->textColor;
        while(*psz) {
                if(bounds.Contains(p)) {
                        if(dc->rgnCurrentClip.Includes(p)) {
                                if(dc->backgroundMode==bm_opaque) {
                                        nocheck_CpiSetCellData(dc,p.x,p.y,(uint16)(color<<8|*psz));
                                } else {
                                        uint16 d;
                                        nocheck_CpiQueryCellData(dc,p.x,p.y,&d);
                                        d = (uint16) (d&0xf000|(dc->textColor<<8)|*psz);
                                        nocheck_CpiSetCellData(dc,p.x,p.y,d);
                                }
                        }
                } else {
                        if(p.x>=bounds.xRight) {
                                //finished
                                while(*psz++) p.x++;
                                break;
                        }
                }
                psz++;
                p.x++;
        }
        dc->recordModify(y);
        return p;
}

FPoint CpiHotkeyTextout(WmsHDC hdc, int lx, int ty, int rx, int by, const char *psz, char hotkeyChar, uint8 normColor, uint8 hotkeyColor) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return FPoint(lx,ty);
        FPoint p(lx,ty);
        uint16 clr=normColor;
        while(*psz) {
                if(*psz==hotkeyChar) {
                        clr=hotkeyColor;
                } else {
                        if(dc->rgnCurrentClip.Includes(p) &&
                           (p.x>=lx && p.x<rx && p.y>=ty && p.y<by))
                        {
                                if(dc->backgroundMode==bm_opaque) {
                                        nocheck_CpiSetCellData(dc,p.x,p.y,(uint16)(clr<<8|*psz));
                                } else {
                                        uint16 d;
                                        nocheck_CpiQueryCellData(dc,p.x,p.y,&d);
                                        d = (uint16) (d&0xf000|((clr&0x0f)<<8)|*psz);
                                        nocheck_CpiSetCellData(dc,p.x,p.y,d);
                                }
                        }
                        p.x++;
                        clr=normColor;
                }
                psz++;
        }
        dc->recordModify(ty);
        return p;
}


void CpiFillRect(WmsHDC hdc, int xLeft, int yTop, int xRight, int yBottom, int color, char c) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return;

        uint16 d= (uint16)(color<<8|c);


        Region rgnFast;
        if(!rgnFast.Copy(dc->rgnCurrentClip))
                goto slow;
        rgnFast.Clip(FRect(xLeft,yTop,xRight,yBottom));
        rgnFast.Optimize();
        if(rgnFast.QueryRectCount()>20)
                goto slow;
        {
                //fast method
                FRect r[20];
                int rs=rgnFast.QueryRects(r,20);
                for(int i=0; i<rs; i++)
                        for(int y=r[i].yTop; y<r[i].yBottom; y++)
//                                for(int x=r[i].xLeft; x<r[i].xRight; x++)
//                                        nocheck_CpiSetCellData(dc,x,y,d);
                                nocheck_CpiSetCellDataRange(dc,r[i].xLeft,r[i].xRight,y,d);
        }
        dc->recordModify(yTop);
        dc->recordModify(yBottom-1);
        return;

slow:
        //slow method
        FRect bounds=dc->rgnCurrentClip.QueryBoundsRect();
        if(xLeft<bounds.xLeft) xLeft=bounds.xLeft;
        if(xRight>bounds.xRight) xRight=bounds.xRight;
        if(yTop<bounds.yTop) yTop=bounds.yTop;
        if(yBottom>bounds.yBottom) yBottom=bounds.yBottom;

        for(int y=yTop; y<yBottom; y++)
                for(int x=xLeft; x<xRight; x++)
                        if(dc->rgnCurrentClip.Includes(FPoint(x,y)))
                                nocheck_CpiSetCellData(dc,x,y,d);
        dc->recordModify(yTop);
        dc->recordModify(yBottom-1);
}



Bool CpiSetClip(WmsHDC hdc, const Region &newClip) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);

        if(!dc) return False;
        Region ncc;
        if(!ncc.Copy(dc->rgnAlwaysClip)) return False;
        if(!ncc.Intersect(newClip)) return False;
        ncc.Optimize();
        if(!dc->rgnCurrentClip.Copy(ncc)) return False;
        return True;
}

Bool CpiQueryRegion(WmsHDC hdc, Region *rgn) {
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);

        if(!dc) return False;
        if(!rgn->Copy(dc->rgnCurrentClip))
                return False;
        return True;
}

Bool CpiDrawText(WmsHDC hdc,
                 const char *psz, int textlen,
                 FRect *rect,
                 color fore, color back,
                 uint32 format)
{
        DCLock dl;
        DeviceContext *dc=hdcMap.Find(hdc);
        if(!dc) return False;
        if(!psz) return False;

        if(textlen==-1) textlen=strlen(psz);
        if(textlen<0) return False;
        if(format&~(DT_LEFT|DT_CALCRECT|DT_TEXTATTRS|DT_CENTER|DT_RIGHT|DT_WORDBREAK|DT_ERASERECT))
                return False;   //illegal bit
        if(rect->IsEmpty()) return False;

        uint16 attr;
        if(format&DT_TEXTATTRS)
                attr=(uint16)(dc->textBackgroundColor<<4|dc->textColor);
        else
                attr=(uint16)(back<<4|fore);

        if(format&DT_ERASERECT)
                CpiFillRect(hdc,*rect,attr,' ');

        int rectwidth=rect->GetWidth();

        int index=0;
        int y=rect->yTop;
        while(index<textlen) {
                //find end of line
                int nextIndex,eol;
                if(psz[index]=='\n') {
                        eol=index;
                        nextIndex=eol+1;
                } else {
                        eol=index+1;
                        for(;;) {
                                if((eol==textlen) ||
                                   (psz[eol]==' ' && format&DT_WORDBREAK && eol-index>rectwidth) ||
                                   (psz[eol]=='\n')) {
                                        nextIndex=eol+1;
                                        break;
                                }
                                nextIndex=eol;
                                eol++;
                        }
                }

                if(!(format&DT_CALCRECT)) {
                        //compute start (x,y)
                        int x;
                        if(format&DT_RIGHT) {
                                x = rect->xLeft + rectwidth-eol-index;
                        } else if(format&DT_CENTER) {
                                x = rect->xLeft + (rectwidth-eol-index)/2;
                        } else {
                                x = rect->xLeft + 0;
                        }

                        //output line
                        if(format&DT_TEXTATTRS && dc->backgroundMode==bm_transparent) {
                                while(index<eol) {
                                        uint16 d;
                                        if(CpiQueryCellData(hdc,FPoint(x,y),&d)) {
                                                d&=0xf000;
                                                d|=(uint16)(dc->textColor<<8);
                                                d|=psz[index];
                                                CpiSetCellData(hdc,FPoint(x,y),d);
                                        }
                                        index++;
                                        x++;
                                }
                        } else {
                                while(index<eol) {
                                        CpiSetCellData(hdc,FPoint(x,y),(uint16)(attr<<8|psz[index]));
                                        index++;
                                        x++;
                                }
                        }
                }

                index=nextIndex;
                y++;
        }

        dc->recordModify(rect->yTop);
        dc->recordModify(y-1);

        if(format&DT_CALCRECT)
                rect->yBottom=y;
        return True;
}






static WmsHDC WmsCreateDC(const FPoint &pt, const Region &rgn, int sl, int el) {
        DeviceContext *dc=new DeviceContext(pt,rgn,sl,el);
        if(!dc) return 0;
        DCLock dl;
        WmsHDC_sequencer = (WmsHDC)((unsigned)WmsHDC_sequencer+1);
        dc->hdc = WmsHDC_sequencer;
        hdcMap.Insert(dc->hdc,dc);
        return dc->hdc;
}

WmsHDC WmsGetScreenDC() {
        Region r;
        r.Include(FRect(0,0,screen_width,screen_height));
        return WmsCreateDC(FPoint(0,0),r,0,screen_height);
}

WmsHDC WmsGetWindowDC(WmsHWND hwnd, uint32 fl) {
        Region rgn;
        if(!WmsQuerySpecialRegion(hwnd,fl,&rgn)) return 0;

        WmsHWND wndtop=hwnd;
        while(WmsQueryWindowParent(wndtop))
                wndtop=WmsQueryWindowParent(wndtop);

        FPoint p(0,0);
        WmsMapWindowPoints(hwnd,wndtop,&p,1);
        FRect rct=rgn.QueryBoundsRect();
        WmsMapWindowRects(hwnd,wndtop,&rct,1);
        return WmsCreateDC(p,rgn,rct.yTop,rct.yBottom);
}

WmsHDC WmsBeginPaint(WmsHWND hwnd, FRect *paintRect) {
        WmsHDC hdc=WmsGetWindowDC(hwnd,0);
        if(!hdc) return 0;

        Region paintRgn;
        if(!WmsQueryWindowUpdateRegion(hwnd,&paintRgn)) return 0;

        CpiSetClip(hdc,paintRgn);

        Region rgn;
        CpiQueryRegion(hdc,&rgn);
        //filter empty repaints
        if(rgn.IsEmpty()) {
                WmsReleaseDC(hdc);
                return 0;
        }
        if(paintRect)
                *paintRect = rgn.QueryBoundsRect();

        WmsValidateRect(hwnd,0,False);
        return hdc;
}

void WmsEndPaint(WmsHDC hdc) {
        WmsReleaseDC(hdc);
}

void WmsReleaseDC(WmsHDC hdc) {
        DeviceContext *dc;
        {
                DCLock dl;
                dc=hdcMap.Find(hdc);
                if(!dc) return;
                hdcMap.Remove(hdc);
        }

        int startLine=dc->startLine;
        int endLine=dc->endLine;
        if(startLine < dc->offset.y+dc->firstModifyLine)
                startLine = dc->offset.y+dc->firstModifyLine;
        if(startLine<0) startLine=0;
        if(endLine > dc->offset.y+dc->lastModifyLine)
                endLine = dc->offset.y+dc->lastModifyLine;
        if(endLine>screen_height) endLine=screen_height;
        delete dc;
        if(startLine<endLine)
                showBuffer(startLine,endLine);
}


