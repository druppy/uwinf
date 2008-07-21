#ifdef __OS2__
#define INCL_MOU
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#endif

#define WMSINCL_TIMESTAMP
#include <wms.hpp>
#include "hwmouse.hpp"
#include "sysqueue.hpp"
#include "hwkeybrd.hpp"

#include <stdio.h>

#if (BIFOS_ == BIFOS_DOS16_) || (BIFOS_ == BIFOS_DOS32_)
///////////////////////////////////////////////////////////////////////////////
#include "../dosrmode.hpp"
#include <dos.h>
static int mouse_installed=0;
class MouseReaderThread;
static MouseReaderThread *readerThread=0;
static int screen_width=0;                      //physical screen width
static int screen_height=0;                     //-"-             height
#define BUFFERED_EVENTS 10
struct mouse_event {
        uint16 ax,bx,cx,dx,si,di;
};

///////////////////////////////////////
//Compiler and extender dependent stuff
#if BIFOS_ == BIFOS_DOS16_
//16-bit real-mode
//declare the buffer to be __far so the mouse callback dosn't need DS
static mouse_event __far buffer[BUFFERED_EVENTS];
static volatile unsigned __far buffer_head=0,buffer_tail=0;

#ifdef __WATCOMC__
//turn off stackchecking
#pragma off(check_stack);
//If using /d2 or higher Watcom may squash registers
void mousecallback_prolog();
#pragma aux mousecallback_prolog = "pusha";
void mousecallback_epilog();
#pragma aux mousecallback_epilog = "popa";

static __far MouseCallback(unsigned ax, unsigned bx, unsigned cx, unsigned dx, unsigned si, unsigned di) {
#pragma aux MouseCallback parm [ax] [bx] [cx] [dx] [si] [di];
        mousecallback_prolog();
        unsigned bh=buffer_head;
        buffer[bh].ax = ax;
        buffer[bh].bx = bx;
        buffer[bh].cx = cx;
        buffer[bh].dx = dx;
        buffer[bh].si = si;
        buffer[bh].di = di;
        if((buffer_head+1)%BUFFERED_EVENTS != buffer_tail)
                buffer_head = (buffer_head+1)%BUFFERED_EVENTS;
        mousecallback_epilog();
}
#else
#error Insert your compiler-specific blurb here
#endif
#else
//32-bit protected mode
static mouse_event buffer[BUFFERED_EVENTS];
static volatile unsigned buffer_head=0,buffer_tail=0;

#ifdef __WATCOMC__
//turn off stackchecking
#pragma off(check_stack);

static __loadds __far MouseCallback(unsigned /*eax*/, unsigned ebx, unsigned ecx, unsigned edx, unsigned esi, unsigned edi) {
#pragma aux MouseCallback parm [eax] [ebx] [ecx] [edx] [esi] [edi];
        unsigned bh=buffer_head;
        //buffer[bh].ax = (uint16)eax;
        buffer[bh].bx = (uint16)ebx;
        buffer[bh].cx = (uint16)ecx;
        buffer[bh].dx = (uint16)edx;
        buffer[bh].si = (uint16)esi;
        buffer[bh].di = (uint16)edi;
        if((buffer_head+1)%BUFFERED_EVENTS != buffer_tail)
                buffer_head = (buffer_head+1)%BUFFERED_EVENTS;
}
#else
#error Insert your compiler-specific blurb here
#endif

#endif

/////////////////////////////////////////////////
//Compiler and extender dependent stuff ends here

static restrictMouseToScreen() {
        if(mouse_installed && screen_width!=0 && screen_height!=0) {
                REGS regs;
                regs.w.ax = 0x0007;
                regs.w.cx = 0;
                regs.w.dx = (uint16)((screen_width-1) * 8);
                int80x86(0x33,&regs,&regs);
                regs.w.ax = 0x0008;
                regs.w.cx = 0;
                regs.w.dx = (uint16)((screen_height-1) * 8);
                int80x86(0x33,&regs,&regs);
        }
}

class MouseReaderThread : public FThread {
public:
        void Go();
        int please_terminate;

        MouseReaderThread() : FThread(), please_terminate(0) {}
};

void MouseReaderThread::Go() {
        while(!please_terminate) {
                if(buffer_head!=buffer_tail) {
                        SystemEvent se;
                        se.type = SystemEvent::mouse;
                        se.u.sme.column = (sint16)(buffer[buffer_tail].cx / 8);
                        se.u.sme.row = (sint16)(buffer[buffer_tail].dx / 8);
                        se.u.sme.buttonState = 0;
                        if(buffer[buffer_tail].bx&0x01) se.u.sme.buttonState |= 1;
                        if(buffer[buffer_tail].bx&0x02) se.u.sme.buttonState |= 2;
                        if(buffer[buffer_tail].bx&0x04) se.u.sme.buttonState |= 4;
                        se.keyboardShiftState = hwkeyboard_internal.QueryCurrentShiftState();
                        se.timestamp  = WmsQueryTimeStamp();
                        systemEventQueue_internal.put(&se,-1);
                        buffer_tail = (buffer_tail+1)%BUFFERED_EVENTS;
                } else {
                        Sleep(10);
                }
        }
}


HWMouse::HWMouse() {
}

HWMouse::~HWMouse() {
}

int HWMouse::Initialize() {
        mouse_installed=0;
        if(!_dos_getvect(0x33)) return -1;
        REGS regs;
        regs.w.ax = 0x0000;
        int80x86(0x33,&regs,&regs);
        if(regs.w.ax!=0xffffU) return -1;

        readerThread = new MouseReaderThread;
        if(!readerThread) {
                return -2;
        }
        if(readerThread->Start()!=0) {
                delete readerThread;
                readerThread=0;
                return -3;
        }
        //install mouse callback
        SREGS sregs;
        regs.w.ax = 0x000c;
        regs.w.cx = 0x7f;
        segread(&sregs);
        sregs.es = FP_SEG(MouseCallback);
#if BIFOS_==BIFOS_DOS16_
        regs.x.dx = FP_OFF(MouseCallback);
#else
        regs.x.edx = FP_OFF(MouseCallback);
#endif
        int80x86x(0x33,&regs,&regs,&sregs);
        mouse_installed = 1;

        //set the mouse range to the screen size
        restrictMouseToScreen();

        return 0;
}

void HWMouse::Terminate() {
        if(mouse_installed) {
                //uninstall mouse callback
                REGS regs;
                SREGS sregs;
                regs.w.ax = 0x000c;
                regs.w.cx = 0x00;
                segread(&sregs);
                sregs.es = 0;
                regs.w.dx = 0;
                int80x86x(0x33,&regs,&regs,&sregs);
                mouse_installed = 0;

                readerThread->please_terminate = 1;
                readerThread->Wait();
                delete readerThread;
                readerThread = 0;
        }
}

void HWMouse::SetPointerShape(void *p) {
        if(!mouse_installed) return;
        //todo: only text mode is supported
        REGS regs;
        regs.w.ax = 0x000a; //define text cursor
        regs.w.bx = 0x0000; //software cursor
        regs.w.cx = ((uint16*)p)[0]; //and mask
        regs.w.dx = ((uint16*)p)[1]; //xor mask
        int80x86(0x33,&regs,&regs);
}

void HWMouse::QueryPosition(int *column, int *row) {
        if(!mouse_installed) return;
        REGS regs;
        regs.w.ax = 0x0003; //get mouse position and button status
        int80x86(0x33,&regs,&regs);
        *column = (int)regs.w.cx / 8;
        *row = (int)regs.w.dx / 8;
}

void HWMouse::SetPosition(int column, int row) {
        if(!mouse_installed) return;
        REGS regs;
        regs.w.ax = 0x0004; //set mouse position
        regs.w.cx = (uint16)(column * 8);
        regs.w.dx = (uint16)(row * 8);
        int80x86(0x33,&regs,&regs);
}

unsigned HWMouse::queryCurrentButtonStatus() {
        if(!mouse_installed) return 0;
        REGS regs;
        regs.w.ax = 0x0003; //get mouse position and button status
        int80x86(0x33,&regs,&regs);
        unsigned r=0;
        if(regs.w.bx&0x1) r|= 1;
        if(regs.w.bx&0x2) r|= 2;
        if(regs.w.bx&0x4) r|= 4;
        return r;
}

void HWMouse::ShowPointer() {
        if(!mouse_installed) return;
        REGS regs;
        regs.w.ax = 0x0001;
        int80x86(0x33,&regs,&regs);
}

void HWMouse::HidePointer(int lx, int ty, int rx, int by) {
        if(!mouse_installed) return;
        REGS regs;
        regs.w.ax= 0x0010;
        regs.w.cx = (uint16)(lx * 8);
        regs.w.dx = (uint16)(ty * 8);
        regs.w.si = (uint16)(rx * 8);
        regs.w.di = (uint16)(by * 8);
        int80x86(0x33,&regs,&regs);
}

void HWMouse::HidePointer() {
        if(!mouse_installed) return;
        //todo: stack hide areas?
        REGS regs;
        regs.w.ax = 0x0002;
        int80x86(0x33,&regs,&regs);
}

void HWMouse::SetScreenSize(int cx, int cy) {
        screen_width = cx;
        screen_height = cy;

        //set the mouse range to the screen size
        restrictMouseToScreen();
}

#elif (BIFOS_ == BIFOS_OS2_)
///////////////////////////////////////////////////////////////////////////////

static HMOU hmou=0;
static MOUEVENTINFO mei;

static void    *savedPointerShapeBuf;
static PTRSHAPE savedPointerShape;
class MouseReaderThread;
static MouseReaderThread *readerThread=0;
static int screen_width;                        //physical screen width
static int screen_height;                       //-"-             height
static int pointerShapeSupported=0;             //only true in FS sessions

class MouseReaderThread : public FThread {
public:
        void Go();
        FEventSemaphore terminating;
        volatile int please_terminate;

        MouseReaderThread() : FThread(), terminating(), please_terminate(0) {}
};

void MouseReaderThread::Go() {
        while(!please_terminate) {
                USHORT wait=1;
                APIRET rc=MouReadEventQue(&mei, &wait, hmou);
                if(rc!=0) break;
                if(!please_terminate) {
                        SystemEvent se;
                        se.type = SystemEvent::mouse;
                        se.u.sme.column = mei.col;
                        se.u.sme.row = mei.row;
                        se.u.sme.buttonState = 0;
                        if(mei.fs&0x06) se.u.sme.buttonState |= 1;
                        if(mei.fs&0x18) se.u.sme.buttonState |= 2;
                        if(mei.fs&0x60) se.u.sme.buttonState |= 4;
                        se.keyboardShiftState = hwkeyboard_internal.QueryCurrentShiftState();
                        se.timestamp  = mei.time;
                        systemEventQueue_internal.put(&se,-1);
                }
        }
        terminating.Post();
}

static void savePointer() {
        if(!pointerShapeSupported) return;
        savedPointerShape.cb=0;
        char dummy;
        APIRET rc=MouGetPtrShape((PBYTE)&dummy, &savedPointerShape, hmou);
        if(rc!=0 && savedPointerShape.cb!=0) {
                savedPointerShapeBuf = new char[savedPointerShape.cb];
                if(savedPointerShapeBuf) {
                        rc=MouGetPtrShape((PBYTE)savedPointerShapeBuf, &savedPointerShape, hmou);
                        if(rc!=0) {
                                delete[] savedPointerShapeBuf;
                                savedPointerShapeBuf=0;
                        }
                }
        }
}
static void restorePointer() {
        if(!pointerShapeSupported) return;
        if(savedPointerShapeBuf) {
                MouSetPtrShape((PBYTE)savedPointerShapeBuf, &savedPointerShape, hmou);
                delete savedPointerShapeBuf;
                savedPointerShapeBuf=0;
        }
}

HWMouse::HWMouse() {
}

HWMouse::~HWMouse() {
}


int HWMouse::Initialize() {
        APIRET rc=MouOpen(NULL, &hmou);
        if(rc) {
                hmou=0;
                return -1;
        } else {
                PTIB ptib;
                PPIB ppib;
                DosGetInfoBlocks(&ptib,&ppib);
                if(ppib->pib_ultype==0) //fullscreen
                        pointerShapeSupported=1;
                else
                        pointerShapeSupported=0;

                savePointer();
                readerThread = new MouseReaderThread;
                if(!readerThread) {
                        MouClose(hmou);
                        return -2;
                }
                if(readerThread->Start()!=0) {
                        delete readerThread;
                        readerThread=0;
                        return -3;
                }
                return 0;
        }
}

void HWMouse::Terminate() {
        if(hmou) {
                readerThread->please_terminate = 1;
                // The Mou subsystem is buggy under OS/2 2.0-2.11, so the
                // mousereader thread is not terminated unless we are running
                // Warp or later
                ULONG u[2];
                DosQuerySysInfo(QSV_VERSION_MAJOR,QSV_VERSION_MINOR,u,sizeof(u[0])*2);
                if(u[0]>20 || (u[0]==20 && u[1]>30)) {
                        if(readerThread->terminating.Wait(500)!=0) {
                                readerThread->Kill();
                        }
                        readerThread->Wait();
                        delete readerThread;
                }
                readerThread = 0;
                restorePointer();
                hmou = 0;
        }
}

void HWMouse::SetPointerShape(void *p) {
        if(!hmou) return;
        if(!pointerShapeSupported) return;
        //todo: only text mode is supported
        PTRSHAPE ptrshape;
        ptrshape.cb = 4;
        ptrshape.col=1;
        ptrshape.row=1;
        ptrshape.colHot=0;
        ptrshape.rowHot=0;
        APIRET rc=MouSetPtrShape((PBYTE)p,&ptrshape,hmou);
}

void HWMouse::QueryPosition(int *column, int *row) {
        if(!hmou) return;
        PTRLOC ptrloc;
        MouGetPtrPos(&ptrloc,hmou);
        *column = ptrloc.col;
        *row = ptrloc.row;
}

void HWMouse::SetPosition(int column, int row) {
        if(!hmou) return;
        PTRLOC ptrloc;
        ptrloc.row = (USHORT)row;
        ptrloc.col = (USHORT)column;
        MouSetPtrPos(&ptrloc,hmou);
}

unsigned HWMouse::queryCurrentButtonStatus() {
        if(!hmou) return 0;
        unsigned r=0;
        if(mei.fs&0x06) r|= 1;
        if(mei.fs&0x18) r|= 2;
        if(mei.fs&0x60) r|= 4;
        return r;
}

void HWMouse::ShowPointer() {
        if(!hmou) return;
        if(!pointerShapeSupported) return;
        APIRET rc=MouDrawPtr(hmou);
}

void HWMouse::HidePointer(int lx, int ty, int rx, int by) {
        if(!hmou) return;
        if(!pointerShapeSupported) return;
        NOPTRRECT noptrrect;
        noptrrect.row = (USHORT)ty;
        noptrrect.col = (USHORT)lx;
        noptrrect.cRow = (USHORT)by;
        noptrrect.cCol = (USHORT)rx;
        APIRET rc=MouRemovePtr(&noptrrect,hmou);
}

void HWMouse::HidePointer() {
        if(!hmou) return;
        //todo: stack hide areas?
        HidePointer(0,0,screen_width-1,screen_height-1);
}

void HWMouse::SetScreenSize(int cx, int cy) {
        screen_width = cx;
        screen_height = cy;
}

#else
#error unknow OS
#endif

