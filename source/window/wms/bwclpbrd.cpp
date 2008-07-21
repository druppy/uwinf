/***
Filename: bwclpbrd.cpp
Description:
  Implementation of a clipboard for WMS
  If the program is a DOS program and it is running in a Windows-DOS box 
  Windows' clipboard is used. (Such a trick is not possible in OS/2's
  DOS-boxes) If the program is a OS/2 program and PM is running then PM's
  clipboard is used. Otherwise an internal clipboard is used
Host:
  Watcom 10.5-10.6
History:
  ISJ 96-05-03 Created
  ISJ 96-08-29 Added support for 32-bit DPMI
***/

#if defined(OS2) || defined(__OS2__)
// OS/2 -----------------------------------------------------------------------
# define INCL_DOSMODULEMGR
# define INCL_WINCLIPBOARD
# define INCL_ORDINALS
# define INCL_DOSMEMMGR
//We need PM's definition of clipboard constants, so we turn off WMS just for
//this module
# undef BIFWINDOWSYSTEM_WMS
#endif
#include <bif.h>
#include "bwclpbrd.h"
#include <stdlib.h>
#include <string.h>

WMSClipBoard WmsClipboard; //the clipboard instance

static char *clipboardbuffer=0;
static unsigned clipboardbuffersize=0;

#if (BIFOS_ == BIFOS_OS2_)
static HMODULE hmodpmwin;
static HAB hab;
static HAB (APIENTRY *winInitialize)(ULONG);
static BOOL (APIENTRY *winTerminate)(HAB);
static BOOL (APIENTRY *winOpenClipbrd)(HAB);
static BOOL (APIENTRY *winCloseClipbrd)(HAB);
static BOOL (APIENTRY *winEmptyClipbrd)(HAB);
static BOOL (APIENTRY *winSetClipbrdData)(HAB,ULONG,ULONG,ULONG);
static ULONG (APIENTRY *winQueryClipbrdData)(HAB,ULONG);
WMSClipBoard::WMSClipBoard() {
        if(useInternalClipboard()) {
                hmodpmwin=0;
                return;
        }

        //try to load pmwin.dll
        char dontCare[80];
        APIRET rc=DosLoadModule((PSZ)dontCare,80,"pmwin.dll",&hmodpmwin);
        if(rc) {
                hmodpmwin=0;
                return;
        }

        //load procedure addresses by ordinal
        rc = DosQueryProcAddr(hmodpmwin, ORD_WIN32INITIALIZE, NULL,       (PFN*)&winInitialize);
        if(rc!=0) goto oops;
        rc = DosQueryProcAddr(hmodpmwin, ORD_WIN32TERMINATE, NULL,        (PFN*)&winTerminate);
        if(rc!=0) goto oops;
        rc = DosQueryProcAddr(hmodpmwin, ORD_WIN32OPENCLIPBRD, NULL,      (PFN*)&winOpenClipbrd);
        if(rc!=0) goto oops;
        rc = DosQueryProcAddr(hmodpmwin, ORD_WIN32CLOSECLIPBRD, NULL,     (PFN*)&winCloseClipbrd);
        if(rc!=0) goto oops;
        rc = DosQueryProcAddr(hmodpmwin, ORD_WIN32EMPTYCLIPBRD, NULL,     (PFN*)&winEmptyClipbrd);
        if(rc!=0) goto oops;
        rc = DosQueryProcAddr(hmodpmwin, ORD_WIN32SETCLIPBRDDATA, NULL,   (PFN*)&winSetClipbrdData);
        if(rc!=0) goto oops;
        rc = DosQueryProcAddr(hmodpmwin, ORD_WIN32QUERYCLIPBRDDATA, NULL, (PFN*)&winQueryClipbrdData);
        if(rc!=0) goto oops;

        //create hab
        hab = (*winInitialize)(0);
        if(!hab)
                goto oops;

        return;

oops:
        DosFreeModule(hmodpmwin);
        hmodpmwin = 0;
}

WMSClipBoard::~WMSClipBoard() {
        if(hmodpmwin) {
                //destroy hab
                (*winTerminate)(hab);
                //unload pmwin
                DosFreeModule(hmodpmwin);
        }
}

int WMSClipBoard::SetText(const char *buf, unsigned bytes) {
        if(hmodpmwin) {
                int r=-1;
                if((*winOpenClipbrd)(hab)) {
                        PVOID pSharedMem;
                        if(DosAllocSharedMem(&pSharedMem, NULL, bytes+1, PAG_WRITE|PAG_COMMIT|OBJ_GIVEABLE|OBJ_GETTABLE)==0) {
                                memcpy(pSharedMem,buf,bytes);
                                ((char*)pSharedMem)[bytes]='\0';
                                (*winEmptyClipbrd)(hab);
                                if((*winSetClipbrdData)(hab,(ULONG)pSharedMem,CF_TEXT,CFI_POINTER))
                                        r=0;
                        }
                        (*winCloseClipbrd)(hab);
                }
                return r;
        } else
                return defSetText(buf,bytes);
}

int WMSClipBoard::GetText(char *buf, unsigned maxbuf, unsigned *bytes) {
        if(hmodpmwin) {
                int r=-1;
                if((*winOpenClipbrd)(hab)) {
                        PSZ pSharedMem = 0;
                        pSharedMem = (PSZ) (*winQueryClipbrdData)(hab,CF_TEXT);
                        if(pSharedMem) {
                                if(DosGetSharedMem(pSharedMem,PAG_READ)==0) {
                                        int b=strlen(pSharedMem);
                                        if(b<=maxbuf) {
                                                memcpy(buf,pSharedMem,b);
                                                *bytes = (unsigned)b;
                                                r = 0;
                                        }
                                }
                        }
                        (*winCloseClipbrd)(hab);
                }
                return r;
        } else
                return defGetText(buf,maxbuf,bytes);
}

#elif (BIFOS_ == BIFOS_DOS16_)
// dos 16-bit -----------------------------------------------------------------
#include <dos.h>
#ifdef __WATCOMC__
#include <i86.h>
#endif
static int winoldapp;
WMSClipBoard::WMSClipBoard() {
        if(useInternalClipboard()) {
                winoldapp=0;
                return;
        }
        //test winoldapp presence
        REGS regs;
        regs.x.ax = 0x1700;
        int86(0x2f, &regs, &regs);
        if(regs.x.ax==0x1700)
                winoldapp=0;    //no clipboard support
        else
                winoldapp=1;

}

WMSClipBoard::~WMSClipBoard() {
        //nothing
}

int WMSClipBoard::SetText(const char *buf, unsigned bytes) {
        if(winoldapp) {
                REGS regs;
                SREGS sregs;
                //open the clipboard
                regs.x.ax = 0x1701;
                int86(0x2f, &regs,&regs);
                if(regs.x.ax==0) {
                        //could not open clipboard
                        return -1;
                }

                //empty clipboard
                regs.x.ax = 0x1702;
                int86(0x2f,&regs,&regs);
                //no error checking

                //put data into clipboard
                regs.x.ax = 0x1703;
                regs.x.dx = 0x07; //OEM text
                sregs.es = FP_SEG(buf);
                regs.x.bx = FP_OFF(buf);
                regs.x.si = 0;
                regs.x.cx = bytes;
                int86x(0x2f,&regs,&regs,&sregs);
                if(regs.x.ax==0) {
                        //could not put data into clipboard (very unlikely)
                        regs.x.ax = 0x1708;     //close clipboard
                        int86(0x2f,&regs,&regs);
                        return -1;
                } else {
                        regs.x.ax = 0x1708;     //close clipboard
                        int86(0x2f,&regs,&regs);
                        return 0;
                }
        } else
                return defSetText(buf,bytes);
}

int WMSClipBoard::GetText(char *buf, unsigned maxbuf, unsigned *bytes) {
        if(winoldapp) {
                REGS regs;
                SREGS sregs;
                //open the clipboard
                regs.x.ax = 0x1701;
                int86(0x2f, &regs,&regs);
                if(regs.x.ax==0) {
                        //could not open clipboard
                        return -1;
                }
                //get size of clipboard data
                regs.x.ax = 0x1704;
                regs.x.dx = 0x07; //OEM text
                int86(0x2f,&regs,&regs);
                if(regs.x.ax==0 && regs.x.dx==0) {
                        //no text in clipboard
                        regs.x.ax = 0x1708;
                        int86(0x2f,&regs,&regs);
                        return -1;
                }
                if(regs.x.dx!=0     || //cannot handle more than 64K in clipboard
                   regs.x.ax>maxbuf    //too much text
                 ) {
                        
                        regs.x.ax = 0x1708;
                        int86(0x2f,&regs,&regs);
                        return -1;
                }

                //get clipboard data
                regs.x.ax = 0x1705;
                regs.x.dx = 0x07; //OEM text
                sregs.es = FP_SEG(buf);
                regs.x.bx = FP_OFF(buf);
                int86x(0x2f,&regs,&regs,&sregs);
                int retreived=regs.x.ax!=0;

                //close clipboard
                regs.x.ax = 0x1708;
                int86(0x2f,&regs,&regs);

                return retreived?0:-1;
        } else
                return defGetText(buf,maxbuf,bytes);
}

#elif (BIFOS_ == BIFOS_DOS32_)
//We use the internal clipboard only
//Otherwise we would have to interface the DPMI allocate real mode memory etc.
WMSClipBoard::WMSClipBoard() {
}

WMSClipBoard::~WMSClipBoard() {
        //nothing
}

int WMSClipBoard::SetText(const char *buf, unsigned bytes) {
        return defSetText(buf,bytes);
}

int WMSClipBoard::GetText(char *buf, unsigned maxbuf, unsigned *bytes) {
        return defGetText(buf,maxbuf,bytes);
}

#else
#error not yet
#endif


int WMSClipBoard::defSetText(const char *buf, unsigned bytes) {
        if(bytes==0) return -1;
        if(bytes>clipboardbuffersize) {
                //reallocate to expand clipboardbuffer
                char *p = (char*)realloc(clipboardbuffer,bytes);
                if(!p) {
                        //squash old buffer and try to alloc
                        if(clipboardbuffer) {
                                free(clipboardbuffer);
                                clipboardbuffer=0;
                                clipboardbuffersize=0;
                        }
                        p = (char*)malloc(bytes);
                }
                if(!p) return -1;
                clipboardbuffer=p;
                clipboardbuffersize=bytes;
        } else if(bytes<clipboardbuffersize) {
                //reallocate to reduce memory
                char *p = (char*)realloc(clipboardbuffer,bytes);
                if(p) {
                        clipboardbuffer=p;
                        clipboardbuffersize=bytes;
                } else {
                        //should never happen unless the RTL is brain-dead
                        clipboardbuffersize=bytes;
                }
        }
        memcpy(clipboardbuffer,buf,bytes);
        return 0;
}

int WMSClipBoard::defGetText(char *buf, unsigned maxbuf, unsigned *bytes) {
        if(clipboardbuffer==0)
                return -1;
        if(clipboardbuffersize>maxbuf)
                return -1;
        memcpy(buf,clipboardbuffer,clipboardbuffersize);
        *bytes = clipboardbuffersize;
        return 0;
}

int WMSClipBoard::useInternalClipboard() {
        //If the environent contains WMSCLIPBOARD=internal ...
        char *p=getenv("WMSCLIPBOARD");
        if(p && stricmp(p,"internal")==0)
                return 1;
        else
                return 0;
}
