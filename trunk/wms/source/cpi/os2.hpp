//
//Low-level interface from WMS-CPI to OS/2's VIO subsystem
//
static VIOMODEINFO old_vmi;     //old video mode information
static VIOCURSORINFO old_vci;   //old cursor shape
static VIOCURSORINFO current_vci;
static USHORT current_cursor_col;
static USHORT current_cursor_row;
static VIOINTENSITY old_vi;

Bool _InitializeCPI() {
        APIRET rc;
        old_vmi.cb=sizeof(old_vmi);
        rc=VioGetMode(&old_vmi,0);
        if(rc) return False;

        VIOMODEINFO vmi;
        vmi.cb=8;
        vmi.fbType=1;
        vmi.color=COLORS_16;
        vmi.col=80;
        vmi.row=50;
        rc=VioSetMode(&vmi,0);
        if(rc) return False;

        screen_width=80;
        screen_height=50;

        USHORT buflength;
        uint16 _far16 *LVBPtr;
        rc=VioGetBuf((PULONG)&LVBPtr, &buflength, 0);
        if(rc) return False;
        screen = LVBPtr;
        BYTE cell[2]={' ',7};
        rc=VioScrollUp(0,0,-1,-1,-1, cell, 0);
        if(rc) return False;

        WmsSetSysValue(0,SV_CXSCREEN,screen_width);
        WmsSetSysValue(0,SV_CYSCREEN,screen_height);

        VioGetCurType(&old_vci,0);
        current_vci=old_vci;
        if(current_vci.attr!=-1)
                _CpiHideCursor();

        //use high intensity background colors instead of blinking (if possible)
        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib,&ppib);
        if(ppib->pib_ultype==0) {
                //fullscreen
                old_vi.cb = sizeof(old_vi);
                old_vi.type = 2;
                rc=VioGetState(&old_vi, 0);
                if(rc==0 && old_vi.fs!=1) {
                        VIOINTENSITY new_vi=old_vi;
                        new_vi.fs = 1;
                        rc=VioSetState(&new_vi, 0);
                }
                if(rc==0) {
                        capabilities |= CPICAP_ATTRIBUTECOLOR1616;
                } else {
                        //could not query or set intensity
                        capabilities |= CPICAP_ATTRIBUTECOLOR168;
                }
        } else {
                //VIO windowed assumed
                //VIO does not support mono or blink so this is easy
                capabilities |= CPICAP_ATTRIBUTECOLOR1616;
        }

        //VIO can display all characters
        capabilities |= CPICAP_CHARACTERS_00_1f;
        capabilities |= CPICAP_CHARACTERS_20_7f;
        capabilities |= CPICAP_CHARACTERS_80_ff;

        //detect wether or not the "international" codepage is used
        USHORT codepage;
        rc=VioGetCp(0, &codepage, 0);
        if(rc!=0 ||             //error
           (codepage!=437 && codepage!=860 && codepage!=861 && codepage!=862 && codepage!=863 && codepage!=865))
                ; //nothing
        else
                capabilities |= CPICAP_FULLBOXDRAW;

        return True;
}

Bool _TerminateCPI() {
        //restore old mode
        VioSetMode(&old_vmi,0);
        //clear screen
        BYTE cell[2]={' ',7};
        VioScrollUp(0,0,-1,-1,-1, cell, 0);
        VioShowBuf(0,(USHORT)old_vmi.buf_length,0);
        //restore cursor type
        VioSetCurType(&old_vci,0);
        //restore blinking
        VioSetState(&old_vi,0);
        return True;
}

static void showBuffer(int startLine, int endLine) {
        hwmouse_internal.HidePointer(0,startLine,screen_width-1,endLine-1);
        VioShowBuf((USHORT) (startLine*screen_width*2),
                   (USHORT) ((endLine-startLine)*screen_width*2),
                   0);
        hwmouse_internal.ShowPointer();
}

Bool _CpiSetCursorPos(WmsHWND hwnd, int x, int y) {
        //calc. physical position
        FPoint p(x,y);
        WmsMapWindowPoints(hwnd,HWND_DESKTOP,&p,1);
        //todo: visibility checking
        if(p.x!=current_cursor_col || p.y!=current_cursor_row) {
                APIRET rc=VioSetCurPos((USHORT)p.y,(USHORT)p.x,0);
                if(rc==0) {
                        current_cursor_col=(USHORT)p.x;
                        current_cursor_row=(USHORT)p.y;
                        return True;
                } else
                        return False;
        } else
                return True;
}

Bool _CpiSetCursorShape(int start, int end) {
        //start and end are expressed as percentages of the current font height
        if(start<0 || start>end || end>100) 
                return False;

        VIOCURSORINFO new_vci=current_vci;
        new_vci.yStart = (USHORT)-start;
        new_vci.cEnd = (USHORT)-end;
        if(new_vci.yStart!=current_vci.yStart || new_vci.cEnd!=current_vci.cEnd) {
                APIRET rc=VioSetCurType(&new_vci,0);
                if(rc==0) {
                        current_vci=new_vci;
                        return True;
                } else
                        return False;
        } else
                return True;
}

Bool _CpiShowCursor() {
        VIOCURSORINFO new_vci = current_vci;
        new_vci.attr=0;
        if(current_vci.attr!=new_vci.attr) {
                if(VioSetCurType(&new_vci,0)==0) {
                        current_vci=new_vci;
                        return True;
                } else
                        return False;
        } else
                return True;
}

Bool _CpiHideCursor() {
        VIOCURSORINFO new_vci = current_vci;
        new_vci.attr=-1;
        if(current_vci.attr!=new_vci.attr) {
                if(VioSetCurType(&new_vci,0)==0) {
                        current_vci=new_vci;
                        return True;
                } else
                        return False;
        } else
                return True;
}

