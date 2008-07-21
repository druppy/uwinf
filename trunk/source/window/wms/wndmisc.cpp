/***
Filename: wndmisc.cpp
Description:
  Implementation of resource objects (except FMenu which resides in fmenu.cpp)
Host:
  Watcom 10.0a-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 95-03-06 Added FMenu::Popup
  ISJ 96-??-?? Copied to WMS
***/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_RESOURCETEMPLATE
#define WMSINCL_HARDWAREMOUSE
#include <bif.h>
#include <dbglog.h>
#include <string.h>

//FIcon------------------------------------------------------------------------
FIcon::FIcon(int resId, FModule *module)
{
        if(!module) module=GetCurrentApp();
        r=module->LoadIcon(resId);
}



//Pointer
FPointer::FPointer(int resId, FModule *module)
{
        if(!module) module=GetCurrentApp();
        r=module->LoadPointer(resId);
}

void FPointer::UsePointer() {
        const PointerTemplate *t = (const PointerTemplate *)r;
        if(!t) return;
        uint16 ptr[2];
        ptr[0] = (uint16)((uint16)(t->attr_and)<<8 | t->char_and);
        ptr[1] = (uint16)((uint16)(t->attr_xor)<<8 | t->char_xor);
        WmsSetPointerShape(ptr);
}

//SystemPointer
#define PID_ARROW       0xf001
#define PID_TEXT        0xf002
#define PID_SIZENESW    0xf003
#define PID_SIZENS      0xf004
#define PID_SIZENWSE    0xf005
#define PID_SIZEWE      0xf006
#define PID_WAIT        0xf007
POINTER(PID_ARROW)
  POINTERSHAPE(0,255,0x7f,0x7f)
ENDPOINTER
POINTER(PID_TEXT)
  POINTERSHAPE(0,255,0x7f,0x7f)
ENDPOINTER
POINTER(PID_SIZENESW)
  POINTERSHAPE('/',0,0,255)
ENDPOINTER
POINTER(PID_SIZENS)
  POINTERSHAPE(0x12,0,0,255)
ENDPOINTER
POINTER(PID_SIZENWSE)
  POINTERSHAPE('\\',0,0,255)
ENDPOINTER
POINTER(PID_SIZEWE)
  POINTERSHAPE(0x1d,0,0,255)
ENDPOINTER
POINTER(PID_WAIT)
  POINTERSHAPE(236,0,0x70,0)
ENDPOINTER

static FPointer *sys_arrowPointer=0;
static FPointer *sys_textPointer=0;
static FPointer *sys_neswPointer=0;
static FPointer *sys_nsPointer=0;
static FPointer *sys_nwsePointer=0;
static FPointer *sys_wePointer=0;
static FPointer *sys_waitPointer=0;

FPointer * const BIFFUNCTION GetSystemArrowPointer() {
        if(!sys_arrowPointer) sys_arrowPointer=new FPointer(PID_ARROW);
        return sys_arrowPointer;
}
FPointer * const BIFFUNCTION GetSystemTextPointer() {
        if(!sys_textPointer) sys_textPointer=new FPointer(PID_TEXT);
        return sys_textPointer;
}
FPointer * const BIFFUNCTION GetSystemNESWPointer() {
        if(!sys_neswPointer) sys_neswPointer=new FPointer(PID_SIZENESW);
        return sys_neswPointer;
}
FPointer * const BIFFUNCTION GetSystemNSPointer() {
        if(!sys_nsPointer) sys_nsPointer=new FPointer(PID_SIZENS);
        return sys_nsPointer;
}
FPointer * const BIFFUNCTION GetSystemNWSEPointer() {
        if(!sys_nwsePointer) sys_nwsePointer=new FPointer(PID_SIZENWSE);
        return sys_nwsePointer;
}
FPointer * const BIFFUNCTION GetSystemWEPointer() {
        if(!sys_wePointer) sys_wePointer=new FPointer(PID_SIZEWE);
        return sys_wePointer;
}
FPointer * const BIFFUNCTION GetSystemWaitPointer() {
        if(!sys_waitPointer) sys_waitPointer=new FPointer(PID_WAIT);
        return sys_waitPointer;
}


//FResourceString--------------------------------------------------------------
FResourceString::FResourceString(int resId, FModule *module) {
        if(!module) module=GetCurrentApp();
        
        char tmp[256];
        if(module->LoadString(resId,tmp,256) == 0)
                s=0;
        else {
                s=new char[strlen(tmp)+1];
                if(s) strcpy(s,tmp);
        }
}

FResourceString::~FResourceString() { 
        delete[] s; 
}


//FAcceleratorTable------------------------------------------------------------
FAcceleratorTable::FAcceleratorTable(int resID, FModule *module) {
        if(!module) module=GetCurrentApp();

        r=module->LoadAccelTable(resID);
}

