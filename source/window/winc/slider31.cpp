#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdlib.h>
#include <mem.h>

#ifdef __BORLANDC__
//BC simply cannot understand that slider31.h should be found in the directory
//containing this file. The full path must be hardcoded
//Sorry for the inconvinience
#include "..\source\window\winc\slider31.h"
#else
#include "slider31.h"
#endif

#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
//The perfectly reasonable MoveTo() function has been dropped in win32
#define MoveTo(hdc,x,y) MoveToEx(hdc,x,y,NULL)
#endif

//This code is long and complicated ...   :-)

//First we define the structures used by the slider:
struct Shaft {
        RECT rect;              //in slider window
        int start;              //pixels, relative to shaft rect (AKA internal home)
        int range;              //possible pixel positions (inclusive)
};


struct Arm {
        int position;           //pixels from home
        RECT rect;              //in slider window
};

struct Tick {
        POINT pos;              //in slider window
        int size;               //pixel height/width (depends on slider orientation)
        char text[80];          //tick text, if any
};

struct TinyButton {
        RECT rect;              //in slider window
        enum {
                up,down,
        } state;
        BOOL disabled;
};

struct Detent {
        BOOL used;              //is the detent used?
        int pixelsFromHome;     
        POINT pos;              //in slider window
};

#define MAXDETENTS      128
struct Slider {
        Shaft shaft;
        Arm arm;
        TinyButton lessButton;
        TinyButton moreButton;
        enum {
                cap_none,               //mouse not captured
                cap_arm,                //mouse captured by shaft
                cap_less,               //mouse captured by less button
                cap_more                //mouse captured by more button
        } capture;
        HFONT hfont;                    //if NULL use system font (yurck!)
        Detent detent[MAXDETENTS];      //128 detents is max.
        int ticks;                      //# ticks
        Tick tick[1];                   //variable length       
};


//formatting-----------------------------------------------------------------
/*
//formatting constants (they really should be resolution-dependent)
#define SHAFTWIDTH 11
#define SHAFTBUTTONSPACING      3
#define SHAFTTICKSPACING        4
#define ARMWIDTH                9
#define ARMHEIGHT               (SHAFTWIDTH+SHAFTTICKSPACING*2)
#define TICKTICKTEXTSPACING     1
*/
struct {
    int ShaftWidth;
    int ShaftButtonSpacing;
    int ShaftTickSpacing;
    int ArmWidth;
    int ArmHeight;
    int TickTickTextSpacing;
} formatMetrics;
static void GetFormatMetrics() {
        LONG l=(LONG)GetDialogBaseUnits();
        if(HIWORD(l)>18) {
                formatMetrics.ShaftWidth = 15;
                formatMetrics.ShaftButtonSpacing = 4;
                formatMetrics.ShaftTickSpacing = 5;
                formatMetrics.ArmWidth = 11;
        } else {
                formatMetrics.ShaftWidth = 11;
                formatMetrics.ShaftButtonSpacing = 3;
                formatMetrics.ShaftTickSpacing = 4;
                formatMetrics.ArmWidth = 9;
        }
        formatMetrics.ArmHeight = formatMetrics.ShaftWidth + formatMetrics.ShaftTickSpacing*2;
        formatMetrics.TickTickTextSpacing = 1;
}

#define SHAFTWIDTH formatMetrics.ShaftWidth
#define SHAFTBUTTONSPACING      formatMetrics.ShaftButtonSpacing
#define SHAFTTICKSPACING        formatMetrics.ShaftTickSpacing
#define ARMWIDTH                formatMetrics.ArmWidth
#define ARMHEIGHT               formatMetrics.ArmHeight
#define TICKTICKTEXTSPACING     formatMetrics.TickTickTextSpacing

static void FormatShaft(WORD style, Slider FAR &sl, const RECT &slrect) {
        //Place the shaft acording to the style and calculate the retangle
        //of it
        RECT &r=sl.shaft.rect;
        if((style&0x0001)==SLS_HORIZONTAL) {
                //horizontal slider
                switch(style&0x0006) {
                        case SLS_CENTER:
                                r.top=(slrect.bottom-slrect.top-SHAFTWIDTH)/2;
                                break;
                        case SLS_BOTTOM:
                                r.top=slrect.bottom-SHAFTWIDTH-SHAFTTICKSPACING;
                                break;
                        case SLS_TOP:
                                r.top=slrect.top+SHAFTTICKSPACING;
                                break;
                }
                r.bottom=r.top+SHAFTWIDTH;

                r.left=slrect.left;
                r.right=slrect.right;

                if((style&SLS_READONLY)==0) {
                        switch(style&0x0030) {
                                case 0:
                                        //no buttons
                                        break;
                                case SLS_BUTTONSLEFT:
                                        r.left+=SHAFTWIDTH*2+SHAFTBUTTONSPACING;
                                        break;
                                case SLS_BUTTONSRIGHT:
                                        r.right-=SHAFTWIDTH*2+SHAFTBUTTONSPACING;
                                        break;
                        }
                }
                sl.shaft.start=2+ARMWIDTH/2;
                sl.shaft.range=r.right-r.left-2-2-ARMWIDTH;
        } else {
                //vertical slider
                switch(style&0x0006) {
                        case SLS_CENTER:
                                r.left=(slrect.right-slrect.left-SHAFTWIDTH)/2;
                                break;
                        case SLS_LEFT:
                                r.left=slrect.left+SHAFTTICKSPACING;
                                break;
                        case SLS_RIGHT:
                                r.left=slrect.right-SHAFTWIDTH-SHAFTTICKSPACING;
                                break;
                }
                r.right=r.left+SHAFTWIDTH;

                r.top=slrect.top;
                r.bottom=slrect.bottom;

                if((style&SLS_READONLY)==0) {
                        switch(style&0x0030) {
                                case 0:
                                        //no buttons
                                        break;
                                case SLS_BUTTONSTOP:
                                        r.top+=SHAFTWIDTH*2+SHAFTBUTTONSPACING;
                                        break;
                                case SLS_BUTTONSBOTTOM:
                                        r.bottom-=SHAFTWIDTH*2+SHAFTBUTTONSPACING;
                                        break;
                        }
                }

                sl.shaft.start=2+ARMWIDTH/2;
                sl.shaft.range=r.bottom-r.top-2-2-ARMWIDTH;
        }
}


static void FormatArm(WORD style, Slider FAR &sl) {
        //calculate the rectangle of the arm

        RECT &rect=sl.arm.rect;
        if(style&SLS_READONLY) {
                //readonly sliders don't have arms
                SetRect(&rect,0,0,0,0);
                return;
        }
        if((style&0x0001)==SLS_HORIZONTAL) {
                int center=sl.shaft.rect.left+sl.shaft.start+sl.arm.position;
                rect.left=center-ARMWIDTH/2;
                rect.right=rect.left+ARMWIDTH;
                rect.top=sl.shaft.rect.top-(ARMHEIGHT-SHAFTWIDTH)/2;
                rect.bottom=rect.top+ARMHEIGHT;
        } else {
                int center=sl.shaft.rect.top+sl.shaft.start+sl.arm.position;
                rect.top=center-ARMWIDTH/2;
                rect.bottom=rect.top+ARMWIDTH;
                rect.left=sl.shaft.rect.left-(ARMHEIGHT-SHAFTWIDTH)/2;
                rect.right=rect.left+ARMHEIGHT;
        }
}


static void FormatTinyButtons(WORD style, Slider FAR &sl) {
        TinyButton &less=sl.lessButton;
        TinyButton &more=sl.moreButton;
        if(style&SLS_READONLY) {
                //readonly sliders don't have buttons
                SetRect(&less.rect,0,0,0,0);
                SetRect(&more.rect,0,0,0,0);
                return;
        }
        if((style&0x0001)==SLS_HORIZONTAL) {
                switch(style&0x0030) {
                        case 0:
                                //no buttons
                                SetRect(&less.rect,0,0,0,0);
                                SetRect(&more.rect,0,0,0,0);
                                break;
                        case SLS_BUTTONSLEFT:
                                //buttons on the left of the shaft
                                more.rect.right=sl.shaft.rect.left-SHAFTBUTTONSPACING;
                                more.rect.left=more.rect.right-SHAFTWIDTH;
                                less.rect.right=more.rect.left;
                                less.rect.left=less.rect.right-SHAFTWIDTH;
                                break;
                        case SLS_BUTTONSRIGHT:
                                //buttons on the right of the shaft
                                less.rect.left=sl.shaft.rect.right+SHAFTBUTTONSPACING;
                                less.rect.right=less.rect.left+SHAFTWIDTH;
                                more.rect.left=less.rect.right;
                                more.rect.right=more.rect.left+SHAFTWIDTH;
                                break;
                }
                less.rect.top=more.rect.top=sl.shaft.rect.top;
                less.rect.bottom=more.rect.bottom=sl.shaft.rect.bottom;
        } else {
                //vertical slider
                switch(style&0x0030) {
                        case 0:
                                //no buttons
                                SetRect(&less.rect,0,0,0,0);
                                SetRect(&more.rect,0,0,0,0);
                                break;
                        case SLS_BUTTONSTOP:
                                //buttons above the shaft
                                more.rect.bottom=sl.shaft.rect.top-SHAFTBUTTONSPACING;
                                more.rect.top=more.rect.bottom-SHAFTWIDTH;
                                less.rect.bottom=more.rect.top;
                                less.rect.top=less.rect.bottom-SHAFTWIDTH;
                                break;
                        case SLS_BUTTONSBOTTOM:
                                //buttons below the shaft
                                less.rect.top=sl.shaft.rect.bottom+SHAFTBUTTONSPACING;
                                less.rect.bottom=less.rect.top+SHAFTWIDTH;
                                more.rect.top=less.rect.bottom;
                                more.rect.bottom=more.rect.top+SHAFTWIDTH;
                                break;
                }
                less.rect.left=more.rect.left=sl.shaft.rect.left;
                less.rect.right=more.rect.right=sl.shaft.rect.right;
        }
}


static void FormatTicks(WORD style, Slider FAR &sl) {
        //calculate the position of the ticks
        if((style&0x0001)==SLS_HORIZONTAL) {
                int y;
                int home=sl.shaft.rect.left+sl.shaft.start;
                if((style&0x0006)!=SLS_TOP)
                        y=sl.shaft.rect.top-SHAFTTICKSPACING-1;
                else
                        y=sl.shaft.rect.bottom+SHAFTTICKSPACING+1;
                for(int i=0; i<sl.ticks; i++) {
                        sl.tick[i].pos.x=home+(sl.shaft.range*i)/(sl.ticks-1);
                        sl.tick[i].pos.y=y;
                }
        } else {
                int x;
                int home=sl.shaft.rect.top+sl.shaft.start;
                if((style&0x0006)!=SLS_RIGHT)
                        x=sl.shaft.rect.right+SHAFTTICKSPACING+1;
                else
                        x=sl.shaft.rect.left-SHAFTTICKSPACING-1;
                for(int i=0; i<sl.ticks; i++) {
                        sl.tick[i].pos.x=x;
                        sl.tick[i].pos.y=home+(sl.shaft.range*i)/(sl.ticks-1);
                }
        }
}

static void FormatDetents(WORD style, Slider FAR &sl) {
        if((style&0x0001)==SLS_HORIZONTAL) {
                int y;
                if((style&0x0006)!=SLS_TOP)
                        y=sl.shaft.rect.top-SHAFTTICKSPACING-1;
                else
                        y=sl.shaft.rect.bottom+SHAFTTICKSPACING+1;
                int home;
                if((style&0x0200)==SLS_HOMELEFT)
                        home=sl.shaft.rect.left+sl.shaft.start;
                else
                        home=sl.shaft.rect.left+sl.shaft.start+sl.shaft.range-1;

                for(int i=0; i<MAXDETENTS; i++) {
                        if(sl.detent[i].used) {
                                sl.detent[i].pos.y=y;
                                if((style&0x0200)==SLS_HOMELEFT)
                                        sl.detent[i].pos.x=home+sl.detent[i].pixelsFromHome;
                                else
                                        sl.detent[i].pos.x=home-sl.detent[i].pixelsFromHome;
                        }
                }
        } else {
                int x;
                if((style&0x0006)!=SLS_RIGHT)
                        x=sl.shaft.rect.right+SHAFTTICKSPACING+1;
                else
                        x=sl.shaft.rect.left-SHAFTTICKSPACING-1;
                int home;
                if((style&0x0200)==SLS_HOMETOP)
                        home=sl.shaft.rect.top+sl.shaft.start;
                else
                        home=sl.shaft.rect.top+sl.shaft.start+sl.shaft.range-1;

                for(int i=0; i<MAXDETENTS; i++) {
                        if(sl.detent[i].used) {
                                sl.detent[i].pos.x=x;
                                if((style&0x0200)==SLS_HOMETOP)
                                        sl.detent[i].pos.y=home+sl.detent[i].pixelsFromHome;
                                else
                                        sl.detent[i].pos.y=home-sl.detent[i].pixelsFromHome;
                        }
                }
        }
}
        
static void FormatSlider(HWND hwnd, Slider FAR &sl) {
        //calculate positions and size of all of the subitems
        WORD style=(WORD)GetWindowLong(hwnd,GWL_STYLE);
        RECT slrect;
        GetClientRect(hwnd,&slrect);

        FormatShaft(style,sl,slrect);
        FormatArm(style,sl);
        FormatTinyButtons(style,sl);
        FormatTicks(style,sl);
        FormatDetents(style,sl);
}





//drawing--------------------------------------------------------------------
#define CLR_BLACK       RGB(0,0,0)
#define CLR_GRAY        RGB(128,128,128)
#define CLR_LTGRAY      RGB(192,192,192)
#define CLR_WHITE       RGB(255,255,255)


static void DrawShaft(WORD style, HDC hdc, Slider FAR &sl, int hasFocus, HPEN blackPen, HPEN grayPen, HPEN whitePen, BOOL globalDisable) {
        const RECT &rect=sl.shaft.rect;
        //draw rectangle (gray or black depending on focus)
        if(hasFocus && !globalDisable)
                SelectObject(hdc,blackPen);
        else
                SelectObject(hdc,grayPen);
        MoveTo(hdc,rect.left,rect.top);
        LineTo(hdc,rect.right-1,rect.top);
        LineTo(hdc,rect.right-1,rect.bottom-1);
        LineTo(hdc,rect.left,rect.bottom-1);
        LineTo(hdc,rect.left,rect.top);
        //draw shading
        SelectObject(hdc,grayPen);
        MoveTo(hdc,rect.left+1,rect.bottom-3);
        LineTo(hdc,rect.left+1,rect.top+1);
        LineTo(hdc,rect.right-1,rect.top+1);
        SelectObject(hdc,whitePen);
        MoveTo(hdc,rect.left+1,rect.bottom-2);
        LineTo(hdc,rect.right-2,rect.bottom-2);
        LineTo(hdc,rect.right-2,rect.top+0);


        //interior
        if(style&SLS_RIBBONSTRIP) {
                //ribbonstrip
                RECT r;
                r.top=rect.top+2;
                r.bottom=rect.bottom-2;

                //dark gray from home to current position
                r.left=rect.left+2;
                r.right=rect.left+sl.shaft.start+sl.arm.position;
                FillRect(hdc,&r,(HBRUSH)GetStockObject(GRAY_BRUSH));
                //light gray from current to position to end
                r.left=r.right+1;
                r.right=rect.right-2;
                FillRect(hdc,&r,(HBRUSH)GetStockObject(LTGRAY_BRUSH));
        } else {
                //no ribbonstrip
                RECT inner;
                inner.left=rect.left+2;
                inner.right=rect.right-2;
                inner.top=rect.top+2;
                inner.bottom=rect.bottom-2;
                FillRect(hdc,&inner,(HBRUSH)GetStockObject(LTGRAY_BRUSH));
        }
}

static void DrawArm(WORD style, HDC hdc, Slider FAR &sl, int hasFocus, HPEN blackPen, HPEN grayPen, HPEN whitePen, BOOL) {
        const RECT &rect=sl.arm.rect;
        //draw the black frame
        SelectObject(hdc,blackPen);
        MoveTo(hdc,rect.left+1,rect.top); LineTo(hdc,rect.right-1,rect.top);
        MoveTo(hdc,rect.left+1,rect.bottom-1); LineTo(hdc,rect.right-1,rect.bottom-1);
        MoveTo(hdc,rect.left,rect.top+1); LineTo(hdc,rect.left,rect.bottom-1);
        MoveTo(hdc,rect.right-1,rect.top+1); LineTo(hdc,rect.right-1,rect.bottom-1);

        //draw shading
        SelectObject(hdc,whitePen);
        MoveTo(hdc,rect.left+1,rect.bottom-3);
        LineTo(hdc,rect.left+1,rect.top+1);
        LineTo(hdc,rect.right-1,rect.top+1);
        SelectObject(hdc,grayPen);
        MoveTo(hdc,rect.left+2,rect.bottom-2);
        LineTo(hdc,rect.right-2,rect.bottom-2);
        LineTo(hdc,rect.right-2,rect.top+1);
        SetPixel(hdc,rect.left+1,rect.bottom-2,CLR_LTGRAY);
        SetPixel(hdc,rect.right-2,rect.top+1,CLR_LTGRAY);
        //clear the interiour
        RECT cr;
        cr.left=rect.left+2; cr.right=rect.right-2;
        cr.top=rect.top+2; cr.bottom=rect.bottom-2;
        FillRect(hdc,&cr,(HBRUSH)GetStockObject(LTGRAY_BRUSH));

        if((style&0x0001)==SLS_HORIZONTAL) {
                //draw vertical groove
                int half=ARMWIDTH/2;
                SelectObject(hdc,grayPen);
                MoveTo(hdc,rect.left+half,rect.top+2);
                LineTo(hdc,rect.left+half,rect.bottom-1);
                SelectObject(hdc,whitePen);
                MoveTo(hdc,rect.right-half,rect.top+1);
                LineTo(hdc,rect.right-half,rect.bottom-1);
                SetPixel(hdc,rect.left+half,rect.top+1,CLR_LTGRAY);
                SetPixel(hdc,rect.right-half,rect.bottom-2,CLR_LTGRAY);
        } else {
                //draw horizontal groove
                int half=ARMWIDTH/2;
                SelectObject(hdc,grayPen);
                MoveTo(hdc,rect.left+2,rect.top+half);
                LineTo(hdc,rect.right-2,rect.top+half);
                SelectObject(hdc,whitePen);
                MoveTo(hdc,rect.left+2,rect.bottom-half);
                LineTo(hdc,rect.right-2,rect.bottom-half);
                SetPixel(hdc,rect.left+1,rect.top+half,CLR_LTGRAY);
                SetPixel(hdc,rect.right-2,rect.bottom-half,CLR_LTGRAY);
        }

        if(hasFocus) {
                //put a black spot in the middle
                int cx=rect.left+(rect.right-rect.left)/2;
                int cy=rect.top+(rect.bottom-rect.top)/2;
                SetPixel(hdc,cx-1,cy-1,CLR_BLACK);
                SetPixel(hdc,cx  ,cy-1,CLR_BLACK);
                SetPixel(hdc,cx+1,cy-1,CLR_BLACK);
                SetPixel(hdc,cx-1,cy  ,CLR_BLACK);
                SetPixel(hdc,cx  ,cy  ,CLR_BLACK);
                SetPixel(hdc,cx+1,cy  ,CLR_BLACK);
                SetPixel(hdc,cx-1,cy+1,CLR_BLACK);
                SetPixel(hdc,cx  ,cy+1,CLR_BLACK);
                SetPixel(hdc,cx+1,cy+1,CLR_BLACK);
        }
}

static void DrawTinyButton(WORD style, HDC hdc, const TinyButton FAR &tb, HPEN blackPen, HPEN grayPen, HPEN whitePen, BOOL less, BOOL globalDisable) {
        const RECT &r=tb.rect;

        //draw outer rectangle
        if(!globalDisable)
                SelectObject(hdc,blackPen);
        else
                SelectObject(hdc,grayPen);
        MoveTo(hdc,r.left,r.top);
        LineTo(hdc,r.right-1,r.top);
        LineTo(hdc,r.right-1,r.bottom-1);
        LineTo(hdc,r.left,r.bottom-1);
        LineTo(hdc,r.left,r.top);

        //draw shading
        if(tb.state==tb.up)
                SelectObject(hdc,whitePen);
        else
                SelectObject(hdc,grayPen);
        MoveTo(hdc,r.left+1,r.bottom-3);
        LineTo(hdc,r.left+1,r.top+1);
        LineTo(hdc,r.right-2,r.top+1);
        if(tb.state==tb.up)
                SelectObject(hdc,grayPen);
        else
                SelectObject(hdc,whitePen);
        MoveTo(hdc,r.left+2,r.bottom-2);
        LineTo(hdc,r.right-2,r.bottom-2);
        LineTo(hdc,r.right-2,r.top+1);
        SetPixel(hdc,r.left+1,r.bottom-2,CLR_LTGRAY);
        SetPixel(hdc,r.right-2,r.top+1,CLR_LTGRAY);

        //clear interior
        RECT cr;
        cr.left=r.left+2;
        cr.right=r.right-2;
        cr.top=r.top+2;
        cr.bottom=r.bottom-2;
        FillRect(hdc,&cr,(HBRUSH)GetStockObject(LTGRAY_BRUSH));

        //draw the arrows
        COLORREF clr;
        if(!(globalDisable || tb.disabled))
                clr=CLR_BLACK;
        else
                clr=CLR_GRAY;
        int width=r.right-r.left;
        int height=r.bottom-r.top;
        int cx=r.left+width/2;
        int cy=r.top+height/2;
        if(tb.state==TinyButton::down) {
                cx++;
                cy++;
        }
        if(less) {
                if((style&0x0001)==SLS_HORIZONTAL) {
                        //left arrow:
                        //   *
                        //  **
                        // ***
                        //  **
                        //   *
                        SetPixel(hdc,cx-1,cy  ,clr);
                        SetPixel(hdc,cx  ,cy-1,clr);
                        SetPixel(hdc,cx  ,cy  ,clr);
                        SetPixel(hdc,cx  ,cy+1,clr);
                        SetPixel(hdc,cx+1,cy-2,clr);
                        SetPixel(hdc,cx+1,cy-1,clr);
                        SetPixel(hdc,cx+1,cy  ,clr);
                        SetPixel(hdc,cx+1,cy+1,clr);
                        SetPixel(hdc,cx+1,cy+2,clr);
                } else {
                        //up arrow
                        //    *
                        //   ***
                        //  *****
                        SetPixel(hdc,cx  ,cy-1,clr);
                        SetPixel(hdc,cx-1,cy  ,clr);
                        SetPixel(hdc,cx  ,cy  ,clr);
                        SetPixel(hdc,cx+1,cy  ,clr);
                        SetPixel(hdc,cx-2,cy+1,clr);
                        SetPixel(hdc,cx-1,cy+1,clr);
                        SetPixel(hdc,cx  ,cy+1,clr);
                        SetPixel(hdc,cx+1,cy+1,clr);
                        SetPixel(hdc,cx+2,cy+1,clr);
                }
        } else {
                if((style&0x0001)==SLS_HORIZONTAL) {
                        //right arrow
                        // *
                        // **
                        // ***
                        // **
                        // *
                        SetPixel(hdc,cx-1,cy-2,clr);
                        SetPixel(hdc,cx-1,cy-1,clr);
                        SetPixel(hdc,cx  ,cy-1,clr);
                        SetPixel(hdc,cx-1,cy  ,clr);
                        SetPixel(hdc,cx  ,cy  ,clr);
                        SetPixel(hdc,cx+1,cy  ,clr);
                        SetPixel(hdc,cx-1,cy+1,clr);
                        SetPixel(hdc,cx  ,cy+1,clr);
                        SetPixel(hdc,cx-1,cy+2,clr);
                } else {
                        //down arrow
                        // *****
                        //  ***
                        //   *
                        SetPixel(hdc,cx-2,cy-1,clr);
                        SetPixel(hdc,cx-1,cy-1,clr);
                        SetPixel(hdc,cx  ,cy-1,clr);
                        SetPixel(hdc,cx+1,cy-1,clr);
                        SetPixel(hdc,cx+2,cy-1,clr);
                        SetPixel(hdc,cx-1,cy  ,clr);
                        SetPixel(hdc,cx  ,cy  ,clr);
                        SetPixel(hdc,cx+1,cy  ,clr);
                        SetPixel(hdc,cx  ,cy+1,clr);
                }
        }
}

static void DrawTicks(WORD style, HDC hdc, Slider FAR &sl, HPEN blackPen, HPEN , HPEN ) {
        SelectObject(hdc,blackPen);
        if((style&0x0001)==SLS_HORIZONTAL) {
                if((style&0x0006)!=SLS_TOP) {
                        Tick *t=sl.tick;
                        for(int i=0; i<sl.ticks; i++, t++) {
                                if(t->size!=0) {
                                        MoveTo(hdc,t->pos.x,t->pos.y);
                                        LineTo(hdc,t->pos.x,t->pos.y-t->size-1);
                                }
                        }
                } else {
                        Tick *t=sl.tick;
                        for(int i=0; i<sl.ticks; i++, t++) {
                                if(t->size!=0) {
                                        MoveTo(hdc,t->pos.x,t->pos.y);
                                        LineTo(hdc,t->pos.x,t->pos.y+t->size+1);
                                }
                        }
                }
        } else {
                if((style&0x0006)!=SLS_RIGHT) {
                        Tick *t=sl.tick;
                        for(int i=0; i<sl.ticks; i++, t++) {
                                if(t->size!=0) {
                                        MoveTo(hdc,t->pos.x,t->pos.y);
                                        LineTo(hdc,t->pos.x+t->size+1,t->pos.y);
                                }
                        }
                } else {
                        Tick *t=sl.tick;
                        for(int i=0; i<sl.ticks; i++, t++) {
                                if(t->size!=0) {
                                        MoveTo(hdc,t->pos.x,t->pos.y);
                                        LineTo(hdc,t->pos.x-t->size-1,t->pos.y);
                                }
                        }
                }
        }
}

static void MeasureText(HDC hdc, LPCSTR t, int *w, int *h) {
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
        SIZE sz;
        GetTextExtentPoint32(hdc,t,lstrlen(t),&sz);
        *w = (int)sz.cx;
        *h = (int)sz.cy;
#else
        DWORD dw=GetTextExtent(hdc,t,lstrlen(t));
        *w = (int)LOWORD(dw);
        *h = (int)HIWORD(dw);
#endif
}

static void DrawTickTexts(WORD style, HDC hdc, Slider FAR &sl, HPEN , HPEN , HPEN ) {
        int maxsize=0;
        for(int i=0; i<sl.ticks; i++)
                if(sl.tick[i].size>maxsize)
                        maxsize=sl.tick[i].size;

        if(sl.hfont)
                SelectObject(hdc,sl.hfont);
        int savemode=SetBkMode(hdc,TRANSPARENT);

        if((style&0x0001)==SLS_HORIZONTAL) {
                if((style&0x0006)!=SLS_TOP) {
                        Tick *t=sl.tick;
                        for(int i=0; i<sl.ticks; i++, t++) {
                                int tw,th; MeasureText(hdc,t->text,&tw,&th);
                                TextOut(hdc,
                                        t->pos.x-tw/2,
                                        t->pos.y-maxsize-TICKTICKTEXTSPACING-th,
                                        t->text,
                                        lstrlen(t->text)
                                       );
                        }
                } else {
                        Tick *t=sl.tick;
                        for(int i=0; i<sl.ticks; i++, t++) {
                                int tw,th; MeasureText(hdc,t->text,&tw,&th);
                                TextOut(hdc,
                                        t->pos.x-tw/2,
                                        t->pos.y+maxsize+TICKTICKTEXTSPACING,
                                        t->text,
                                        lstrlen(t->text)
                                       );
                        }
                }
        } else {
                if((style&0x0006)!=SLS_RIGHT) {
                        Tick *t=sl.tick;
                        for(int i=0; i<sl.ticks; i++, t++) {
                                int tw,th; MeasureText(hdc,t->text,&tw,&th);
                                TextOut(hdc,
                                        t->pos.x+maxsize+TICKTICKTEXTSPACING,
                                        t->pos.y-th/2,
                                        t->text,
                                        lstrlen(t->text)
                                       );
                        }
                } else {
                        Tick *t=sl.tick;
                        for(int i=0; i<sl.ticks; i++, t++) {
                                int tw,th; MeasureText(hdc,t->text,&tw,&th);
                                TextOut(hdc,
                                        t->pos.x-maxsize-TICKTICKTEXTSPACING-tw,
                                        t->pos.y-th/2,
                                        t->text,
                                        lstrlen(t->text)
                                       );
                        }
                }
        }
        SetBkMode(hdc,savemode);
}

static void DrawDetents(WORD style, HDC hdc, Slider FAR &sl, HPEN , HPEN , HPEN ) {
struct pt { int dx,dy; };
static pt down1[]= {{0,-3}, {0,-2}, {-1,-1},{0,-1}, {1,-1}, {0,0}  };
static pt up1[]  = {{0,0},  {-1,1}, {0,1},  {1,1},  {0,2},  {0,3}  };
static pt left1[]= {{1,-1}, {0,0},  {1,0},  {2,0},  {3,0},  {1,1}  };
static pt right1[]={{2,-1}, {0,0},  {1,0},  {2,0},  {3,0},  {2,1}  };

static pt down2[]= {{0,-5}, {0,-4}, {0,-3}, {-2,-2},{-1,-2},{0,-2},{1,-2},{2,-2}, {-1,-1},{0,-1},{1,-1}, {0,0}};
static pt up2[]=   {{0,0}, {-1,1},{0,1},{1,1}, {-2,2},{-1,2},{0,2},{1,2},{2,2}, {0,3}, {0,4}, {0,5}};
static pt left2[]= {{2,-2}, {1,-1},{2,-1}, {0,0}, {1,0},{2,0},{3,0},{4,0},{5,0}, {1,1},{2,1}, {2,2}};
static pt right2[]={{-2,-2}, {-2,-1},{-1,-1}, {-5,0},{-4,0},{-3,0},{-2,0},{-1,0},{-0,0}, {-2,1},{-1,1}, {-2,2}};

        int bigdetents=GetSystemMetrics(SM_CXSCREEN)>=1024;
        pt *detentPoint;
        int detentPoints=bigdetents?12:6;
        if((style&0x0001)==SLS_HORIZONTAL) {
                if((style&0x0006)!=SLS_TOP) {
                        detentPoint = bigdetents?down2:down1;
                } else {
                        detentPoint = bigdetents?up2:up1;
                }
        } else {
                if((style&0x0006)!=SLS_RIGHT) {
                        detentPoint = bigdetents?left2:left1;
                } else {
                        detentPoint = bigdetents?right2:right1;
                }
        }
        
        for(int i=0; i<MAXDETENTS; i++) {
                if(sl.detent[i].used) {
                        POINT p=sl.detent[i].pos;
                        for(int j=0; j<detentPoints; j++)
                                SetPixel(hdc, p.x+detentPoint[j].dx, p.y+detentPoint[j].dy, CLR_BLACK);
                }
        }
}

static void DrawSlider(HWND hwnd, Slider FAR &sl, HDC hdc, const RECT &paintrect, BOOL eraseBkgnd) {
        int sDC=SaveDC(hdc);

        if(eraseBkgnd) {
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
                HBRUSH hbr=(HBRUSH)SendMessage(GetParent(hwnd),WM_CTLCOLORBTN,(WPARAM)hdc,(LPARAM)hwnd);
#else
                HBRUSH hbr=(HBRUSH)SendMessage(GetParent(hwnd),WM_CTLCOLOR,(WPARAM)hdc,MAKELONG(hwnd,CTLCOLOR_BTN));
#endif
                if(hbr)
                        FillRect(hdc,&paintrect,hbr);
                else
                        FillRect(hdc,&paintrect,(HBRUSH)GetStockObject(LTGRAY_BRUSH));
        }

        HPEN blackPen=CreatePen(PS_SOLID,0,CLR_BLACK);
        HPEN grayPen=CreatePen(PS_SOLID,0,CLR_GRAY);
        HPEN whitePen=CreatePen(PS_SOLID,0,CLR_WHITE);

        WORD style=(WORD)GetWindowLong(hwnd,GWL_STYLE);
        BOOL globalDisable=(GetWindowLong(hwnd,GWL_STYLE)&WS_DISABLED)!=0;
        RECT tmp;
        int hasFocus=GetFocus()==hwnd;
        if(IntersectRect(&tmp,&paintrect,&sl.shaft.rect))
                DrawShaft(style,hdc,sl,hasFocus,blackPen,grayPen,whitePen,globalDisable);
        if(IntersectRect(&tmp,&sl.lessButton.rect,&paintrect))
                DrawTinyButton(style,hdc,sl.lessButton,blackPen,grayPen,whitePen,TRUE,globalDisable);
        if(IntersectRect(&tmp,&sl.moreButton.rect,&paintrect))
                DrawTinyButton(style,hdc,sl.moreButton,blackPen,grayPen,whitePen,FALSE,globalDisable);
        if((style&0x0001)==SLS_HORIZONTAL) {
                if((style&0x0006)!=SLS_TOP) {
                        if(sl.tick[0].pos.y>paintrect.top) {
                                DrawTicks(style,hdc,sl,blackPen,grayPen,whitePen);
                                DrawTickTexts(style,hdc,sl,blackPen,grayPen,whitePen);
                                DrawDetents(style,hdc,sl,blackPen,grayPen,whitePen);
                        }
                } else {
                        if(sl.tick[0].pos.y<paintrect.bottom) {
                                DrawTicks(style,hdc,sl,blackPen,grayPen,whitePen);
                                DrawTickTexts(style,hdc,sl,blackPen,grayPen,whitePen);
                                DrawDetents(style,hdc,sl,blackPen,grayPen,whitePen);
                        }
                }
        } else {
                if((style&0x0006)!=SLS_RIGHT) {
                        if(sl.tick[0].pos.x<paintrect.right) {
                                DrawTicks(style,hdc,sl,blackPen,grayPen,whitePen);
                                DrawTickTexts(style,hdc,sl,blackPen,grayPen,whitePen);
                                DrawDetents(style,hdc,sl,blackPen,grayPen,whitePen);
                        }
                } else {
                        if(sl.tick[0].pos.x>paintrect.left) {
                                DrawTicks(style,hdc,sl,blackPen,grayPen,whitePen);
                                DrawTickTexts(style,hdc,sl,blackPen,grayPen,whitePen);
                                DrawDetents(style,hdc,sl,blackPen,grayPen,whitePen);
                        }
                }
        }
        if(IntersectRect(&tmp,&paintrect,&sl.arm.rect))
                DrawArm(style,hdc,sl,hasFocus,blackPen,grayPen,whitePen,globalDisable);

        RestoreDC(hdc,sDC);
        DeleteObject(blackPen);
        DeleteObject(grayPen);
        DeleteObject(whitePen);
}


static void PaintSlider(HWND hwnd, Slider FAR &sl) {
        PAINTSTRUCT ps;
        HDC hdc=BeginPaint(hwnd,&ps);
        if(hdc) {
                DrawSlider(hwnd, sl, ps.hdc, ps.rcPaint, ps.fErase);
                EndPaint(hwnd,&ps);
        }
}

static void NoFlickerArmRedraw(HWND hwnd, Slider FAR &sl, RECT prevArmRect, RECT curArmRect) {
        //Redraw the arm without flicker
        //This is done by drawing it in a memory DC and then bitblt'ing to screen

        //calculate the bounding rectangle
        int minx,maxx;
        if(prevArmRect.left<curArmRect.left)
                minx=prevArmRect.left;
        else
                minx=curArmRect.left;
        if(prevArmRect.right>curArmRect.right)
                maxx=prevArmRect.right;
        else
                maxx=curArmRect.right;
        int miny,maxy;
        if(prevArmRect.top<curArmRect.top)
                miny=prevArmRect.top;
        else
                miny=curArmRect.top;
        if(prevArmRect.bottom>curArmRect.bottom)
                maxy=prevArmRect.bottom;
        else
                maxy=curArmRect.bottom;


        HDC hdcWindow=GetDC(hwnd);
        if(!hdcWindow) return;

        HDC hdcMem=CreateCompatibleDC(hdcWindow);
        if(!hdcMem) {
                ReleaseDC(hwnd,hdcWindow);
                return;
        }

        HBITMAP hbmMem=CreateCompatibleBitmap(hdcWindow,maxx,maxy);
        if(!hbmMem) {
                DeleteDC(hdcMem);
                ReleaseDC(hwnd,hdcWindow);
                return;
        }

        HGDIOBJ hbmOld=SelectObject(hdcMem,hbmMem);

        RECT r;
        r.left=minx;
        r.top=miny;
        r.right=maxx;
        r.bottom=maxy;

        DrawSlider(hwnd, sl, hdcMem, r, TRUE);

        BitBlt(hdcWindow,
               r.left,r.top,
               r.right-r.left,r.bottom-r.top,
               hdcMem,
               r.left,r.top,
               SRCCOPY
              );

        SelectObject(hdcMem,hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd,hdcWindow);

        ValidateRect(hwnd,&r);          //we have just painted it, so leave us alone!
}

static LRESULT NotifyOwner(HWND hwndSlider, int code) {
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
        return SendMessage(GetParent(hwndSlider), WM_COMMAND, MAKELONG(GetWindowLong(hwndSlider,GWL_ID),code), (LPARAM)hwndSlider);

#else
        return SendMessage(GetParent(hwndSlider), WM_COMMAND, GetWindowWord(hwndSlider,GWW_ID), MAKELONG(hwndSlider,code));
#endif
}

static void SetArmPos(HWND hwnd, WORD style, Slider FAR &sl, int newPos) {
        //Set the position of the arm and redraw/invalidate as necessary

        //ensure within range
        if(newPos<0)
                newPos=0;
        if(newPos>sl.shaft.range)
                newPos=sl.shaft.range;
        if(style&SLS_SNAPTOINCREMENT) {
                int ticksize=sl.shaft.range/(sl.ticks-1);
                newPos+=ticksize/2;
                int i;
                for(i=0;
                    i*sl.shaft.range/(sl.ticks-1)<newPos &&
                    (i+1)*sl.shaft.range/(sl.ticks-1)<newPos;
                    i++);
                newPos=i*sl.shaft.range/(sl.ticks-1);
        }
        if(newPos==sl.arm.position)
                return; //no change

        if(style&SLS_RIBBONSTRIP) {
                //invalidate interior of shaft
                RECT r;
                if((style&0x0001)==SLS_HORIZONTAL) {
                        int x1=sl.shaft.rect.left+sl.shaft.start+sl.arm.position;
                        int x2=sl.shaft.rect.left+sl.shaft.start+newPos;
                        if(x1<x2) {
                                r.left=x1;
                                r.right=x2;
                        } else {
                                r.left=x2;
                                r.right=x1;
                        }
                        r.top=sl.shaft.rect.top+2;
                        r.bottom=sl.shaft.rect.bottom-2;
                } else {
                        int y1=sl.shaft.rect.top+sl.shaft.start+sl.arm.position;
                        int y2=sl.shaft.rect.top+sl.shaft.start+newPos;
                        if(y1<y2) {
                                r.top=y1;
                                r.bottom=y2;
                        } else {
                                r.top=y2;
                                r.bottom=y1;
                        }
                        r.left=sl.shaft.rect.left+2;
                        r.right=sl.shaft.rect.right-2;
                }
                InvalidateRect(hwnd,&r,FALSE);
        }

        RECT prevRect=sl.arm.rect;
        //InvalidateRect(hwnd,&sl.arm.rect,TRUE);
        sl.arm.position=newPos;
        FormatArm(style,sl);
        //InvalidateRect(hwnd,&sl.arm.rect,TRUE);

        NoFlickerArmRedraw(hwnd,sl,prevRect,sl.arm.rect);


        //update tiny buttons if necessary
        BOOL b;
        b=newPos==0;
        if(sl.lessButton.disabled!=b) {
                sl.lessButton.disabled=b;
                InvalidateRect(hwnd,&sl.lessButton.rect,FALSE);
        }
        b=newPos==sl.shaft.range;
        if(sl.moreButton.disabled!=b) {
                sl.moreButton.disabled=b;
                InvalidateRect(hwnd,&sl.moreButton.rect,FALSE);
        }

        //post notification
        if(sl.capture!=Slider::cap_arm)
                NotifyOwner(hwnd,SLN_CHANGE);
        else
                NotifyOwner(hwnd,SLN_SLIDERTRACK);
}

static void MuchLess(HWND hwnd, WORD style, Slider FAR &sl) {
        SetArmPos(hwnd,style,sl,sl.arm.position-sl.shaft.range/(sl.ticks-1));
}
static void MuchMore(HWND hwnd, WORD style, Slider FAR &sl) {
        SetArmPos(hwnd,style,sl,sl.arm.position+sl.shaft.range/(sl.ticks-1));
}
static void OneLess(HWND hwnd, WORD style, Slider FAR &sl) {
        if(GetWindowLong(hwnd,GWL_STYLE)&SLS_SNAPTOINCREMENT) 
                SetArmPos(hwnd,style,sl,sl.arm.position-sl.shaft.range/sl.ticks);
        else
                SetArmPos(hwnd,style,sl,sl.arm.position-1);
}
static void OneMore(HWND hwnd, WORD style, Slider FAR &sl) {
        if(GetWindowLong(hwnd,GWL_STYLE)&SLS_SNAPTOINCREMENT) 
                SetArmPos(hwnd,style,sl,sl.arm.position+sl.shaft.range/sl.ticks);
        else
                SetArmPos(hwnd,style,sl,sl.arm.position+1);
}


static BOOL CreateSlider(HWND hwnd) {
        //Allocate and initialize slider data

        //find out how many ticks are wanted
        char wndtext[20];
        GetWindowText(hwnd,wndtext,20);
        int ticks=atoi(wndtext);
        if(ticks<2)
                return FALSE;   //can't do that

        //alloc slider data
        HGLOBAL hglob=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof(Slider)+sizeof(Tick)*(ticks-1));
        if(hglob==NULL)
                return FALSE;
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
        SetWindowLong(hwnd,0,(LONG)hglob);
#else
        SetWindowWord(hwnd,0,(WORD)hglob);
#endif
        Slider far *slp=(Slider far *)GlobalLock(hglob);
        if(slp==NULL)
                return FALSE;
        SetWindowLong(hwnd,sizeof(HGLOBAL),(LONG)slp);

        //init slider data
        slp->capture=Slider::cap_none;
        slp->ticks=ticks;

        //format the slider subitems
        WORD style=(WORD)GetWindowLong(hwnd,GWL_STYLE);
        FormatSlider(hwnd,*slp);
        SetArmPos(hwnd,style,*slp,0);

        return TRUE;
}

static BOOL SetTickCount(HWND hwnd, int count) {
        if(count<2) return FALSE;

#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
        HGLOBAL hglobOld=(HGLOBAL)GetWindowLong(hwnd,0);
#else
        HGLOBAL hglobOld=(HGLOBAL)GetWindowWord(hwnd,0);
#endif
        GlobalUnlock(hglobOld);
        HGLOBAL hglobNew = GlobalReAlloc(hglobOld, sizeof(Slider)+sizeof(Tick)*(count-1), GMEM_ZEROINIT);
        if(!hglobNew) {
                SetWindowLong(hwnd,sizeof(HGLOBAL),(LONG)GlobalLock(hglobOld));
                return FALSE;
        }
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
        SetWindowLong(hwnd,0,(LONG)hglobNew);
#else
        SetWindowWord(hwnd,0,(WORD)hglobNew);
#endif
        Slider far *slp=(Slider far*)GlobalLock(hglobNew);
        SetWindowLong(hwnd,sizeof(HGLOBAL),(LONG)slp);
        slp->ticks = count;
        FormatSlider(hwnd,*slp);
        return TRUE;
}

Slider far* GetSLP(HWND hwnd) {
        return (Slider far*)GetWindowLong(hwnd,sizeof(HGLOBAL));
}

static void Destroy(HWND hwnd) {
        //Free slider data
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
        HGLOBAL hglob=(HGLOBAL)GetWindowLong(hwnd,0);
#else
        HGLOBAL hglob=(HGLOBAL)GetWindowWord(hwnd,0);
#endif
        if(hglob) {
                GlobalUnlock(hglob);
                GlobalFree(hglob);
        }
}

#define TID_SLIDER      4321
#define TIMER_INTERVAL  30
static void SetArmPosition(HWND hwnd, Slider FAR &sl, int newPos);

static void MouseDown(HWND hwnd, Slider FAR &sl, int x, int y) {
        POINT where; where.x=x; where.y=y;
        if(sl.capture!=sl.cap_none)
                return;         //already got capture
        if(GetFocus()!=hwnd)
                SetFocus(hwnd);
        WORD style=(WORD)GetWindowLong(hwnd,GWL_STYLE);
        if(style&SLS_READONLY)
                return; //ignore

        if(PtInRect(&sl.lessButton.rect,where) && !sl.lessButton.disabled) {
                //click in "less" tiny button
                if(sl.lessButton.disabled)
                        return; //reject
                SetCapture(hwnd);
                sl.capture=sl.cap_less;
                sl.lessButton.state=sl.lessButton.down;
                OneLess(hwnd,style,sl);
                InvalidateRect(hwnd,&sl.lessButton.rect,FALSE);
                SetTimer(hwnd,TID_SLIDER,TIMER_INTERVAL,NULL);
                return;
        }
        if(PtInRect(&sl.moreButton.rect,where) && !sl.moreButton.disabled) {
                //click in "more" tiny button
                if(sl.moreButton.disabled)
                        return; //reject
                SetCapture(hwnd);
                sl.capture=sl.cap_more;
                sl.moreButton.state=sl.moreButton.down;
                OneMore(hwnd,style,sl);
                InvalidateRect(hwnd,&sl.moreButton.rect,FALSE);
                SetTimer(hwnd,TID_SLIDER,TIMER_INTERVAL,NULL);
                return;
        }
        if(PtInRect(&sl.arm.rect,where)) {
                //click on arm
                SetCapture(hwnd);
                sl.capture=sl.cap_arm;
                return;
        }
        if(PtInRect(&sl.shaft.rect,where)) {
                //click in shaft
                if((style&0x0001)==SLS_HORIZONTAL) {
                        if(where.x<sl.arm.rect.left)
                                MuchLess(hwnd,style,sl);
                        else if(where.x>=sl.arm.rect.right)
                                MuchMore(hwnd,style,sl);
                } else {
                        if(where.y<sl.arm.rect.top)
                                MuchLess(hwnd,style,sl);
                        else if(where.y>=sl.arm.rect.bottom)
                                MuchMore(hwnd,style,sl);
                }
                return;
        }

        int bigdetents=GetSystemMetrics(SM_CXSCREEN)>=1024;
        int dlx,drx,dty,dby;
        if((style&0x0001)==SLS_HORIZONTAL) {
                if((style&0x0006)!=SLS_TOP) {
                        if(bigdetents)
                                dlx=-2,drx=2,dty=-3,dby=0;
                        else
                                dlx=-1,drx=1,dty=-2,dby=0;
                } else {
                        if(bigdetents)
                                dlx=-2,drx=2,dty=0,dby=3;
                        else
                                dlx=-1,drx=1,dty=0,dby=2;
                }
        } else {
                if((style&0x0006)!=SLS_RIGHT) {
                        if(bigdetents)
                                dlx=0,drx=3,dty=-2,dby=2;
                        else
                                dlx=0,drx=2,dty=-1,dby=1;
                } else {
                        if(bigdetents)
                                dlx=-3,drx=0,dty=-2,dby=2;
                        else
                                dlx=-3,drx=0,dty=-1,dby=1;
                }
        }        
        for(int i=0; i<MAXDETENTS; i++) {
                if(sl.detent[i].used) {
                        if(where.x>=sl.detent[i].pos.x+dlx &&
                           where.x<=sl.detent[i].pos.x+drx &&
                           where.y>=sl.detent[i].pos.y+dty &&
                           where.y<=sl.detent[i].pos.y+dby)
                        {
                                SetArmPosition(hwnd,sl,sl.detent[i].pixelsFromHome);
                                return;
                        }
                }
        }
}

static void MouseMove(HWND hwnd, Slider FAR &sl, int x, int y) {
        switch(sl.capture) {
                case Slider::cap_arm: {
                        WORD style=(WORD)GetWindowLong(hwnd,GWL_STYLE);
                        int newArmPos;
                        if((style&0x0001)==SLS_HORIZONTAL) {
                                newArmPos = x-sl.shaft.rect.left-sl.shaft.start;
                        } else {
                                newArmPos = y-sl.shaft.rect.top-sl.shaft.start;
                        }
                        SetArmPos(hwnd, style, sl, newArmPos);
                        break;
                }
                case Slider::cap_less: {
                        POINT pt; pt.x=x; pt.y=y;
                        if(PtInRect(&sl.lessButton.rect,pt))
                                sl.lessButton.state=TinyButton::down;
                        else
                                sl.lessButton.state=TinyButton::up;
                        InvalidateRect(hwnd,&sl.lessButton.rect,FALSE);
                        break;
                }
                case Slider::cap_more: {
                        POINT pt; pt.x=x; pt.y=y;
                        if(PtInRect(&sl.moreButton.rect,pt))
                                sl.moreButton.state=TinyButton::down;
                        else
                                sl.moreButton.state=TinyButton::up;
                        InvalidateRect(hwnd,&sl.moreButton.rect,FALSE);
                        break;
                }
        }
}

static void MouseUp(HWND hwnd, Slider FAR &sl) {
        switch(sl.capture) {
                case Slider::cap_none:
                        break;
                case Slider::cap_less:
                        sl.lessButton.state=sl.lessButton.up;
                        InvalidateRect(hwnd,&sl.lessButton.rect,FALSE);
                        ReleaseCapture();
                        KillTimer(hwnd,TID_SLIDER);
                        break;
                case Slider::cap_more:
                        sl.moreButton.state=sl.moreButton.up;
                        InvalidateRect(hwnd,&sl.moreButton.rect,FALSE);
                        ReleaseCapture();
                        KillTimer(hwnd,TID_SLIDER);
                        break;
                case Slider::cap_arm:
                        ReleaseCapture();
                        //tell the parent that dragging has ended
                        NotifyOwner(hwnd,SLN_CHANGE);
        }
        sl.capture=Slider::cap_none;
}

static void TimerReceived(HWND hwnd, Slider FAR &sl) {
        WORD style=(WORD)GetWindowLong(hwnd,GWL_STYLE);
        DWORD pp=GetMessagePos();
        POINT p;
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
        p.x = MAKEPOINTS(pp).x;
        p.y = MAKEPOINTS(pp).y;
#else
        p=MAKEPOINT(pp);
#endif
        MapWindowPoints(HWND_DESKTOP,hwnd,&p,1);
        if(sl.capture==sl.cap_less) {
                if(PtInRect(&sl.lessButton.rect,p))
                        OneLess(hwnd,style,sl);
        } else if(sl.capture==sl.cap_more) {
                if(PtInRect(&sl.moreButton.rect,p)) {
                        OneMore(hwnd,style,sl);
                }
        }
}


static int QueryArmPosition(HWND hwnd, Slider FAR &sl) {
        WORD style=(WORD)GetWindowLong(hwnd,GWL_STYLE);
        if((style&0x0001)==SLS_HORIZONTAL) {
                if((style&0x0200)==SLS_HOMELEFT)
                        return sl.arm.position;
                else
                        return sl.shaft.range-sl.arm.position;
        } else {
                if((style&0x0200)==SLS_HOMETOP)
                        return sl.arm.position;
                else
                        return sl.shaft.range-sl.arm.position;
        }
}

static void SetArmPosition(HWND hwnd, Slider FAR &sl, int newPos) {
        WORD style=(WORD)GetWindowLong(hwnd,GWL_STYLE);
        if((style&0x0001)==SLS_HORIZONTAL) {
                if((style&0x0200)==SLS_HOMELEFT)
                        SetArmPos(hwnd,style,sl,newPos);
                else
                        SetArmPos(hwnd,style,sl,sl.shaft.range-newPos);
        } else {
                if((style&0x0200)==SLS_HOMETOP)
                        SetArmPos(hwnd,style,sl,newPos);
                else
                        SetArmPos(hwnd,style,sl,sl.shaft.range-newPos);
        }
}

static int AddDetent(HWND hwnd, Slider FAR &sl, int pos) {
        for(int i=0; i<MAXDETENTS; i++) {
                if(!sl.detent[i].used) {
                        sl.detent[i].used=TRUE;
                        sl.detent[i].pixelsFromHome=pos;
                        FormatDetents((WORD)GetWindowLong(hwnd,GWL_STYLE),sl);
                        InvalidateRect(hwnd,NULL,FALSE);
                        return i+1;
                }
        }
        return 0;
}

static BOOL RemoveDetent(HWND hwnd, Slider FAR &sl, int detentId) {
        if(detentId<=0 || detentId>MAXDETENTS)
                return FALSE;
        if(!sl.detent[detentId-1].used)
                return FALSE;
        sl.detent[detentId-1].used=FALSE;
        InvalidateRect(hwnd,NULL,TRUE);
        return TRUE;
}

static LRESULT QueryDetentPos(HWND , Slider FAR &sl, int detentId) {
        if(detentId<=0 || detentId>MAXDETENTS)
                return 0;
        if(!sl.detent[detentId-1].used)
                return 0;
        return sl.detent[detentId-1].pixelsFromHome;
}

static BOOL SetTickSize(HWND hwnd, Slider FAR &sl, int tickNum, int size) {
        if(size<0) return FALSE;
        if(tickNum==SMA_SETALLTICKS || tickNum==-1) {
                for(int i=0; i<sl.ticks; i++)
                        sl.tick[i].size=size;
        } else {
                if(tickNum>=sl.ticks) return FALSE;
                sl.tick[tickNum].size=size;
        }
        FormatTicks((WORD)GetWindowLong(hwnd,GWL_STYLE),sl);
        InvalidateRect(hwnd,NULL,TRUE);
        return TRUE;
}
static int QueryTickSize(HWND, Slider FAR &sl, int tickNum) {
        if(tickNum<0 || tickNum>=sl.ticks)
                return -1;
        else
                return sl.tick[tickNum].size;
}
static LRESULT QueryTickPos(HWND, Slider FAR &sl, int tickNum) {
        if(tickNum<0 || tickNum>=sl.ticks)
                return 0;
        else
                return MAKELONG(sl.tick[tickNum].pos.x,sl.tick[tickNum].pos.y);
}
static unsigned QueryTickCount(HWND /*hwnd*/, Slider far &sl) {
        return sl.ticks;
}

static unsigned QueryShaftRange(HWND /*hwnd*/, Slider far &sl) {
        return sl.shaft.range;
}

LRESULT CALLBACK
#ifndef WIN32
__export
#endif
SliderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch(msg) {
                case WM_CREATE: {
                        if(CreateSlider(hwnd))
                                return 0;
                        else
                                return -1;
                }
                case WM_DESTROY:
                        Destroy(hwnd);
                        return 0;

                case WM_SIZE:
                        //format the slider subitems
                        FormatSlider(hwnd,*GetSLP(hwnd));
                        SetArmPos(hwnd,(WORD)GetWindowLong(hwnd,GWL_STYLE),*GetSLP(hwnd),GetSLP(hwnd)->arm.position);
                        return 0;

                //focus
                case WM_SETFOCUS: {
                        Slider *slp=GetSLP(hwnd);
                        InvalidateRect(hwnd,&slp->shaft.rect,FALSE);
                        InvalidateRect(hwnd,&slp->arm.rect,FALSE);
                        NotifyOwner(hwnd,SLN_SETFOCUS);
                        return 0;
                }
                case WM_KILLFOCUS: {
                        Slider *slp=GetSLP(hwnd);
                        InvalidateRect(hwnd,&slp->shaft.rect,FALSE);
                        InvalidateRect(hwnd,&slp->arm.rect,FALSE);
                        NotifyOwner(hwnd,SLN_KILLFOCUS);
                        return 0;
                }
                
                //painting
                case WM_PAINT:
                        PaintSlider(hwnd,*GetSLP(hwnd));
                        return 0;
                case WM_ERASEBKGND:
                        //dont erase the background here, do it in wm_paint
                        return 0;

                //dialog manager support
                case WM_GETDLGCODE:
                        return DLGC_BUTTON|DLGC_WANTARROWS;

                //mouse
                case WM_LBUTTONDOWN: {
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
                        MouseDown(hwnd, *GetSLP(hwnd), MAKEPOINTS(lParam).x,MAKEPOINTS(lParam).y);
#else
                        MouseDown(hwnd, *GetSLP(hwnd), MAKEPOINT(lParam).x,MAKEPOINT(lParam).y);
#endif
                        return 0;
                }
                case WM_MOUSEMOVE:
#if defined(WIN32) || defined(__NT__) || defined(__WIN32__) || defined(_WIN32)
                        MouseMove(hwnd,*GetSLP(hwnd),MAKEPOINTS(lParam).x,MAKEPOINTS(lParam).y);
#else
                        MouseMove(hwnd,*GetSLP(hwnd),MAKEPOINT(lParam).x,MAKEPOINT(lParam).y);
#endif
                        return 0;
                case WM_CANCELMODE:
                case WM_LBUTTONUP:
                        MouseUp(hwnd,*GetSLP(hwnd));
                        return 0;

                //font handling
                case WM_SETFONT:
                        GetSLP(hwnd)->hfont=(HFONT)wParam;
                        if(LOWORD(lParam))
                                InvalidateRect(hwnd,NULL,TRUE);
                        return 0;
                case WM_GETFONT:
                        return (LRESULT)GetSLP(hwnd)->hfont;

                //timer for auto-repeat in tiny buttons
                case WM_TIMER: {
                        if(wParam==TID_SLIDER) {
                                TimerReceived(hwnd,*GetSLP(hwnd));
                                return 0;
                        } else
                                return DefWindowProc(hwnd,msg,wParam,lParam);
                }

                //keyboard support
                case WM_KEYDOWN: {
                        if(GetWindowLong(hwnd,GWL_STYLE)&SLS_READONLY)
                                return DefWindowProc(hwnd,msg,wParam,lParam);
                        switch(wParam) {
                                case VK_LEFT:
                                case VK_UP:
                                        OneLess(hwnd,(WORD)GetWindowLong(hwnd,GWL_STYLE),*GetSLP(hwnd));
                                        return 0;
                                case VK_RIGHT:
                                case VK_DOWN:
                                        OneMore(hwnd,(WORD)GetWindowLong(hwnd,GWL_STYLE),*GetSLP(hwnd));
                                        return 0;
                                default:
                                        return DefWindowProc(hwnd,msg,wParam,lParam);
                        }
                }                       


                //slider messages:
                case SLM_ADDDETENT:
                        return AddDetent(hwnd,*GetSLP(hwnd),wParam);
                case SLM_QUERYDETENTPOS: 
                        return QueryDetentPos(hwnd,*GetSLP(hwnd),wParam);
                case SLM_REMOVEDETENT:
                        return RemoveDetent(hwnd,*GetSLP(hwnd),wParam);
                case SLM_QUERYSCALETEXT: {
                        int tickNum=(int)wParam;
                        LPSTR lpszText=(LPSTR)lParam;
                        Slider *slp=GetSLP(hwnd);
                        if(tickNum<0 || tickNum>slp->ticks)
                                return -1;
                        lstrcpy(lpszText,slp->tick[tickNum].text);
                        return lstrlen(lpszText);
                }                                   
                case SLM_QUERYSLIDERINFO:
                        if(wParam==SMA_SLIDERARMPOSITION)
                                return QueryArmPosition(hwnd,*GetSLP(hwnd));
                        else if(wParam==SMA_SHAFTRANGE)
                                return QueryShaftRange(hwnd,*GetSLP(hwnd));
                        else if(wParam==SMA_TICKCOUNT)
                                return QueryTickCount(hwnd,*GetSLP(hwnd));
                        else
                                return -1;
                case SLM_QUERYTICKPOS: {
                        return QueryTickPos(hwnd,*GetSLP(hwnd),(int)wParam);
                }
                case SLM_QUERYTICKSIZE: 
                        return QueryTickSize(hwnd,*GetSLP(hwnd),(int)wParam);
                case SLM_SETSCALETEXT: {
                        int tickNum=(int)wParam;
                        LPCSTR lpszText=(LPSTR)lParam;
                        Slider *slp=GetSLP(hwnd);
                        if(tickNum<0 || tickNum>slp->ticks)
                                return FALSE;
                        lstrcpy(slp->tick[tickNum].text,lpszText);
                        InvalidateRect(hwnd,NULL,TRUE);
                        return TRUE;
                }
                case SLM_SETSLIDERINFO: {
                        if(wParam==SMA_SLIDERARMPOSITION)
                                SetArmPosition(hwnd,*GetSLP(hwnd),LOWORD(lParam));
                        else if(wParam==SMA_TICKCOUNT)
                                return SetTickCount(hwnd,(int)lParam);
                        else
                                return FALSE;
                        return 0;
                }
                case SLM_SETTICKSIZE: {
                        int tickNum=(int)wParam;
                        int size=LOWORD(lParam);
                        return SetTickSize(hwnd,*GetSLP(hwnd),tickNum,size);
                }
                //not a message that we are interested in:
                default:
                        return DefWindowProc(hwnd,msg,wParam,lParam);
        }
}


BOOL RegisterSlider(HINSTANCE hinst) {
        GetFormatMetrics();
        
        WNDCLASS cls;
        memset(&cls,0,sizeof(cls));
        cls.style=CS_HREDRAW|CS_VREDRAW;
        cls.lpfnWndProc=(WNDPROC)SliderWndProc;
        cls.cbClsExtra=0;
        cls.cbWndExtra=sizeof(HGLOBAL)+sizeof(Slider far*);
        cls.hInstance=hinst;
        cls.hCursor=LoadCursor(NULL,IDC_ARROW);
        cls.lpszClassName=SLIDER_CLASS;

        return RegisterClass(&cls)!=0;
}

