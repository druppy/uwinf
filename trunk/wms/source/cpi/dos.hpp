//
//Interface for WMS-CPI to DOS 16bit-real and 32bit-extended
//

//perform compiler and DOS extender check
#if (BIFOS_ == BIFOS_DOS32_)
   //32-bit extender
#  ifdef __WATCOMC__
     //Ok, assume Tenberry's DOS4GW
#  else
#    error Unknown compiler / DOS extender
#  endif
#else
   //16-bit real mode
#  ifdef __WATCOMC__
     //Ok
#  elif defined(__BORLANDC__)
     //Ok
#  else
#    error Unknown compiler
#  endif
#endif


static int oldMode;
static int oldLines;
static int oldCursor_start;
static int oldCursor_end;
static int cursor_start=-1;
static int cursor_end=-1;
static int cursor_visible=1;
static int current_cursor_col=-1;
static int current_cursor_row = -1;

#ifdef __BORLANDC__
#  include <dos.h>
#  include <conio.h>
#elif defined(__WATCOMC__)
#  include <i86.h>
//#  include <graph.h>
#endif

#include "../dosrmode.hpp"

Bool _InitializeCPI() {
        REGS regs;
#ifdef __BORLANDC__
        //Query current video mode so it can be restored on exit
        text_info ti;
        gettextinfo(&ti);
        oldMode=ti.currmode;
        //switch to 50-lines mode
        textmode(C4350);
        //query screen size
        gettextinfo(&ti);
        screen_width=  ti.screenwidth;
        screen_height= ti.screenheight;
#elif defined(__WATCOMC__)
        //Query current video mode info so it can be restored on exit
        regs.w.ax = 0x0f00;
        int80x86(0x10,&regs,&regs);
        oldMode = regs.h.al;
        regs.h.ah = 0x03; ////get cursor position and size
        regs.h.bh = 0x00; //page
        int80x86(0x10,&regs,&regs);
        oldCursor_start = regs.h.ch;
        oldCursor_end = regs.h.cl;
        //switch to 80x50 mode
        regs.h.ah = 0x00; //set video mode
        regs.h.al = 0x03; //80x25
        int80x86(0x10,&regs,&regs);
        //select 8x8 font
        regs.h.ah = 0x11; //character generator
        regs.h.al = 0x12; //8x8 double-dot
        regs.h.bl = 0;    //map
        int80x86(0x10,&regs,&regs);
        //query screen size
        //todo
        screen_width=80;
        screen_height=50;

/*        videoconfig vc;
        _getvideoconfig(&vc);
        oldMode = vc.mode;
        oldLines= vc.numtextrows;
        _setvideomoderows(_TEXTC80,50);
        _getvideoconfig(&vc);
        //query screen size
        screen_width=vc.numtextcols;
        screen_height=vc.numtextrows;*/
#endif
        //allocate screen double-buffer
        screen= new uint16[screen_width*screen_height];
        if(!screen) return False;
        for(int i=0; i<screen_width*screen_height; i++)
                screen[i]=(0<<4|7)<<8|' ';

        WmsSetSysValue(0,SV_CXSCREEN,screen_width);
        WmsSetSysValue(0,SV_CYSCREEN,screen_height);

        //Hide cursor
        _CpiHideCursor();

        //use high intensity background colors instead of blinking
        regs.w.ax = 0x1003; //toggle intensity/blinking
        regs.h.bl = 0x00; //background intensity enabled
        regs.h.bh = 0x00; //avoid problems on some adapters
        int80x86(0x10,&regs,&regs);
        capabilities |= CPICAP_ATTRIBUTECOLOR1616;

        //DOS can display all characters
        capabilities |= CPICAP_CHARACTERS_00_1f;
        capabilities |= CPICAP_CHARACTERS_20_7f;
        capabilities |= CPICAP_CHARACTERS_80_ff;

        //detect wether or not we can use all box-drawing characters
        regs.w.ax = 0x6601;
        int80x86(0x21,&regs,&regs);
        if(regs.w.cflag)
                capabilities |= CPICAP_FULLBOXDRAW; //codepage support not loaded
        else if(regs.w.bx==437 ||
                regs.w.bx==860 ||
                regs.w.bx==861 ||
                regs.w.bx==862 ||
                regs.w.bx==863 ||
                regs.w.bx==865)
                capabilities |= CPICAP_FULLBOXDRAW;

        return True;
}


Bool _TerminateCPI() {
        REGS regs;
        //restore old video mode
#ifdef __BORLANDC__
        textmode((unsigned char)oldMode);
        textattr(BLACK<<4|LIGHTGRAY);
        clrscr();
#elif defined(__WATCOMC__)
        regs.h.ah = 0x00;
        regs.h.al = (uint8)oldMode;
        int80x86(0x10,&regs,&regs);
        //restore cursor shape
        regs.h.ah = 0x01; //set textmode cursor shape
        regs.h.ch = (unsigned char)oldCursor_start; //start and options
        regs.h.cl = (unsigned char)oldCursor_end;
        int80x86(0x10,&regs,&regs);
#endif
        //restore blinking
        regs.w.ax = 0x1003; //toggle intensity/blinking
        regs.h.bl = 0x01; //blink enabled
        regs.h.bh = 0x00; //avoid problems on some adapters
        int80x86(0x10,&regs,&regs);

        delete[] screen;
        return True;
}


static void showBuffer(int startLine, int endLine) {
        //Copy the double-buffer lines to the physical screen
        int offset=startLine*screen_width;
        int length=(endLine-startLine)*screen_width;
        uint16 *s=screen+offset;
        Ptr_b800_Type(uint16) d = (Ptr_b800_Type(uint16))Ptr_b800(offset*sizeof(uint16));
        hwmouse_internal.HidePointer(0,startLine,screen_width-1,endLine-1);
        memcpy(d,s,length*sizeof(uint16));
        hwmouse_internal.ShowPointer();
}


Bool _CpiSetCursorPos(WmsHWND hwnd, int x, int y) {
        //calc. physical position
        FPoint p(x,y);
        WmsMapWindowPoints(hwnd,HWND_DESKTOP,&p,1);
        //todo: visibility checking
        if(p.x!=current_cursor_col || p.y!=current_cursor_row) {
                REGS regs;
                regs.h.ah = 0x02; //set cursor position
                regs.h.bh = 0x00; //page
                regs.h.dh = (unsigned char)p.y;  //row
                regs.h.dl = (unsigned char)p.x;  //column
                int80x86(0x10,&regs,&regs);
                current_cursor_col=p.x;
                current_cursor_row=p.y;
                return True;
        } else
                return True;
}

Bool _CpiSetCursorShape(int start, int end) {
        //start and end are expressed as percentages of the current font height
        if(start<0 || start>end || end>100) 
                return False;
        int cs,ce;
        cs = (start*8)/100;
        ce = (end*8)/100;
        if(cs==cursor_start && ce==cursor_end)
                return True;
        REGS regs;
        regs.h.ah = 0x01; //set textmode cursor shape
        regs.h.ch = (unsigned char)cs; //start and options
        if(!cursor_visible) regs.h.ch |= 0xf;
        regs.h.cl = (unsigned char)ce;
        int80x86(0x10,&regs,&regs);

        cursor_start = cs;
        cursor_end = ce;

        return True;
}

Bool _CpiShowCursor() {
        if(cursor_visible)
                return True;
        REGS regs;
        regs.h.ah = 0x01;
        regs.h.ch = (unsigned char)cursor_start;
        regs.h.cl = (unsigned char)cursor_end;
        int80x86(0x10,&regs,&regs);

        cursor_visible = 1;

        return True;
}

Bool _CpiHideCursor() {
        if(!cursor_visible)
                return True;
        REGS regs;
        regs.h.ah = 0x01;
        regs.h.ch = 0x0f;
        regs.h.cl = 0;
        int80x86(0x10,&regs,&regs);

        cursor_visible = 0;

        return True;
}

