/***
Filename: wndmisc.cpp
Description:
  Implementation of resource objects
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 95-03-06 Added FMenu::Popup
  ISJ 97-08-16 Added FResourceString copy ctor and assignment
***/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#include <bif.h>
#include <dbglog.h>
#include <string.h>

//All the resource objects except FResourceString uses 'reference counting' so
//we can assign, return, modify and so forth without worrying about
//double-destruction or excessive use of memory
struct FResourceObject_rep {
        int refs;
        Bool shouldDelete;
        LHANDLE handle;
};

FResourceObject::FResourceObject(Bool d) {
        rep=new FResourceObject_rep;
        rep->refs=1;
        rep->shouldDelete=d;
        rep->handle=NULL;
}

FResourceObject::FResourceObject(const FResourceObject& ro) {
        rep=ro.rep;
}


FResourceObject::~FResourceObject() {
        if(rep->refs==0) delete rep;
}

void FResourceObject::operator=(const FResourceObject& ro) {
        if(rep->refs==0) delete rep;
        rep=ro.rep;
}

int FResourceObject::inc() {
        return ++(rep->refs);
}

int FResourceObject::dec() {
        return --(rep->refs);
}

//A little function so we dont have to repeat ourselves too much
static inline FModule *DefModule(FModule *module) {
        return module?module:FModule::GetDefaultResourceModule();
}

//FIcon------------------------------------------------------------------------

FIcon::FIcon(HPOINTER hicon, Bool d)
  : FResourceObject(d)
{
        rep->handle=hicon;
}

FIcon::FIcon(int resId, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadIcon(resId);
}

FIcon::FIcon(const FIcon& i)
  : FResourceObject(i)
{}

FIcon::~FIcon() {
        if(dec()==0 && rep->shouldDelete && rep->handle)
                WinDestroyPointer((HPOINTER)rep->handle);
}


HPOINTER FIcon::GetHandle() {
        return (HPOINTER)rep->handle;
}

FIcon& FIcon::operator=(const FIcon& i) {
        if(dec()==0 && rep->shouldDelete && rep->handle)
                WinDestroyPointer((HPOINTER)rep->handle);
        FResourceObject::operator=(i);
        return *this;
}


//Pointer
FPointer::FPointer(HPOINTER hptr, Bool d)
  : FResourceObject(d)
{
        rep->handle=hptr;
}

FPointer::FPointer(const FPointer& p)
  : FResourceObject(p)
{
}

FPointer::FPointer(int resId, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadPointer(resId);
}

FPointer::~FPointer() {
        if(dec()==0 && rep->shouldDelete && rep->handle)
                WinDestroyPointer((HPOINTER)rep->handle);
}

HPOINTER FPointer::GetHandle() {
        return (HPOINTER)rep->handle;
}

FPointer& FPointer::operator=(const FPointer& p) {
        if(dec()==0 && rep->shouldDelete && rep->handle)
                WinDestroyPointer((HPOINTER)rep->handle);
        FResourceObject::operator=(p);
        return *this;
}

//SystemPointer
class FSystemPointer : public FPointer {
public:
        FSystemPointer(LONG id)
          : FPointer(WinQuerySysPointer(HWND_DESKTOP,id,FALSE),False)
        {}
};

static FSystemPointer sys_arrowPointer(SPTR_ARROW);
static FSystemPointer sys_textPointer(SPTR_TEXT);
static FSystemPointer sys_neswPointer(SPTR_SIZENESW);
static FSystemPointer sys_nsPointer(SPTR_SIZENS);
static FSystemPointer sys_nwsePointer(SPTR_SIZENWSE);
static FSystemPointer sys_wePointer(SPTR_SIZEWE);
static FSystemPointer sys_waitPointer(SPTR_WAIT);

FPointer * const BIFFUNCTION GetSystemArrowPointer() {
        return &sys_arrowPointer;
}
FPointer * const BIFFUNCTION GetSystemTextPointer() {
        return &sys_textPointer;
}
FPointer * const BIFFUNCTION GetSystemNESWPointer() {
        return &sys_neswPointer;
}
FPointer * const BIFFUNCTION GetSystemNSPointer() {
        return &sys_nsPointer;
}
FPointer * const BIFFUNCTION GetSystemNWSEPointer() {
        return &sys_nwsePointer;
}
FPointer * const BIFFUNCTION GetSystemWEPointer() {
        return &sys_wePointer;
}
FPointer * const BIFFUNCTION GetSystemWaitPointer() {
        return &sys_waitPointer;
}


//FMenu------------------------------------------------------------------------

//todo:
//FMenu::FMenu()
//  : FResourceObject(True)
//{
//        rep->handle=CreateMenu();
//}


FMenu::FMenu(HWND hwndOwner, int resId, FModule *module)
  : FResourceObject(True)

{
        rep->handle = DefModule(module)->LoadMenu(hwndOwner,resId);
}

FMenu::FMenu(FWnd *pwndOwner, int resId, FModule *module)
  : FResourceObject(True)

{
        rep->handle = DefModule(module)->LoadMenu(pwndOwner->GetHwnd(),resId);
}

FMenu::FMenu(HWND hmenu, Bool d)
  : FResourceObject(d)
{
        rep->handle=hmenu;
}

FMenu::FMenu(const FMenu& m)
  : FResourceObject(m)
{
}


FMenu::~FMenu()
{
        if(dec()==0 && rep->shouldDelete && rep->handle)
                WinDestroyWindow((HWND)rep->handle);
}

HWND FMenu::GetHandle() {
        return (HWND)rep->handle;
}


Bool FMenu::Popup(FWnd *parent, FWnd *owner, const FPoint *position) {
        //figure out where to popup the menu
        int x,y;
        if(position) {
                //position specified
                x=position->GetX();
                y=position->GetY();
        } else {
                //position not specified - use current mouse position
                FPoint p;
                WinQueryPointerPos(HWND_DESKTOP, &p);
                WinMapWindowPoints(HWND_DESKTOP, parent->GetHwnd(), &p, 1);
                x=p.GetX();
                y=p.GetY();
        }

        //calculate flags
        ULONG fs=0;
        fs|= PU_HCONSTRAIN|PU_VCONSTRAIN;

        if(WinGetKeyState(HWND_DESKTOP,VK_BUTTON2)&0x8000)      //button 2 (right) is preferred
                fs|= PU_MOUSEBUTTON2DOWN;
        else if(WinGetKeyState(HWND_DESKTOP,VK_BUTTON1)&0x8000)
                fs|= PU_MOUSEBUTTON1DOWN;
        else if(WinGetKeyState(HWND_DESKTOP,VK_BUTTON3)&0x8000)
                fs|= PU_MOUSEBUTTON3DOWN;
        else
                fs|= PU_NONE;

        fs|= PU_KEYBOARD;       
        fs|= PU_MOUSEBUTTON1;
        fs|= PU_MOUSEBUTTON2;
        fs|= PU_MOUSEBUTTON3;

        return (Bool)WinPopupMenu(parent->GetHwnd(),
                                  owner->GetHwnd(),
                                  GetHandle(),
                                  x,y,
                                  0,
                                  fs);
}

Bool FMenu::AppendItem(const char *pszItem, int nId, Bool enabled, Bool checked )
{
        MENUITEM mi;
        mi.iPosition=MIT_END;
        mi.afStyle=MIS_TEXT;
        mi.afAttribute=0;
        if(!enabled) mi.afAttribute|=MIA_DISABLED;
        if(checked) mi.afAttribute|=MIA_CHECKED;
        mi.id=(USHORT)nId;
        mi.hwndSubMenu=NULL;
        mi.hItem=0;
        MRESULT mr=WinSendMsg(GetHandle(),MM_INSERTITEM,(MPARAM)&mi,(MPARAM)pszItem);
        if((SHORT)SHORT1FROMMR(mr) == MIT_MEMERROR ||
           (SHORT)SHORT1FROMMR(mr) == MIT_ERROR)
                return False;
        else
                return True;
}

Bool FMenu::AppendItem(const char *pszItem, FMenu *menu, Bool enabled )
{
        static USHORT runningID=9000;
        MENUITEM mi;
        mi.iPosition=MIT_END;
        mi.afStyle=MIS_TEXT|MIS_SUBMENU;
        mi.afAttribute=0;
        if(!enabled) mi.afAttribute|=MIA_DISABLED;
        mi.id=runningID++;      //only way to do it
        mi.hwndSubMenu=menu->GetHandle();
        mi.hItem=0;
        MRESULT mr=WinSendMsg(GetHandle(),MM_INSERTITEM,(MPARAM)&mi,(MPARAM)pszItem);
        if((SHORT)SHORT1FROMMR(mr) == MIT_MEMERROR ||
           (SHORT)SHORT1FROMMR(mr) == MIT_ERROR)
                return False;
        else
                return True;
}

void FMenu::AppendSeparator( void )
{
        MENUITEM mi;
        mi.iPosition=MIT_END;
        mi.afStyle=MIS_SEPARATOR;
        mi.afAttribute=0;
        mi.id=0;
        mi.hwndSubMenu=NULL;
        mi.hItem=0;
        WinSendMsg(GetHandle(),MM_INSERTITEM,(MPARAM)&mi,NULL);
}

Bool FMenu::DeleteItem(int nId)
{
        WinSendMsg(GetHandle(),MM_DELETEITEM,MPFROM2SHORT(nId,TRUE),NULL);
        return True;
}


Bool FMenu::GetItem(int Id, char *buf, int buflen) {
        MRESULT mr;
        mr=WinSendMsg(GetHandle(),MM_QUERYITEMTEXT,MPFROM2SHORT(Id,buflen),MPFROMP(buf));
        if(SHORT1FROMMR(mr)==0)
                return False;
        else
                return True;
}

FMenu FMenu::GetSubmenuFromPos(int nPos) {
        MRESULT mr;

        mr=WinSendMsg(GetHandle(),MM_ITEMIDFROMPOSITION,MPFROMSHORT(nPos),NULL);
        SHORT usitem=SHORT1FROMMR(mr);
        if(usitem==MIT_ERROR) {
                FATAL("FMenu::GetSubmenuFromPos(): no such submenu");
        }

        MENUITEM mi;
        WinSendMsg(GetHandle(),MM_QUERYITEM,MPFROM2SHORT(usitem,FALSE),MPFROMP(&mi));
        if((mi.afStyle&MIS_SUBMENU)==0 ||
           mi.hwndSubMenu==NULL) {
                FATAL("FMenu::GetSubmenuFromPos(): item is not a submenu");
        }
        return FMenu(mi.hwndSubMenu,False);
}


void FMenu::CheckItem(int Id, Bool check) {
        WinCheckMenuItem(GetHandle(), Id,check);
}

void FMenu::EnableItem(int Id, Bool enable) {
        WinEnableMenuItem(GetHandle(), Id, enable);
}

Bool FMenu::IsItemEnabled(int Id) {
        return Bool(WinIsMenuItemEnabled(GetHandle(),Id));
}

Bool FMenu::IsItemChecked(int Id) {
        return Bool(WinIsMenuItemChecked(GetHandle(),Id));
}


//FResourceString--------------------------------------------------------------
FResourceString::FResourceString(int resId, FModule *module) {
        module = DefModule(module);
        
        char tmp[256];
        if(module->LoadString(GetWndMan()->GetHAB(),resId,tmp,256) == 0)
                s=0;
        else {
                s=new char[strlen(tmp)+1];
                if(s) strcpy(s,tmp);
        }
}

FResourceString::~FResourceString() { 
        delete[] s; 
}

FResourceString::FResourceString(const FResourceString &rs) {
        if(rs.s) {
                s = new char[strlen(rs.s)+1];
                if(s)
                        strcpy(s,rs.s);
        } else
                s=0;
}

FResourceString &FResourceString::operator=(const FResourceString &rs) {
        //avoid unnecessarry newing & deleting
        if(rs.s) {
                if(s && strlen(s)>=strlen(rs.s))
                        strcpy(s,rs.s);
                else {
                        delete[] s;
                        s = new char[strlen(rs.s)+1];
                        if(s)
                                strcpy(s,rs.s);
                }
        }
        return *this;
}

