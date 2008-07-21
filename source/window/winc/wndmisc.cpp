/***
FILE
  WndMisc.Cpp
DESCRIPTION
  Implementation of resource objects
HOST
  BC40, SC61, WC10, VC42
HISTORY
  94-11-?? ISJ Creation
  95-01-20 BL  Changed some of the code, and some comments.
  95-03-06 ISJ Removed default FMenu ctor and added two new for portability
               reasons.
               Implemented FMenu::operator= (it was missing)
               Added self-assignment checks in all operator=
               Made FResourceString dtor non-inline
               Corrected bug in reference counting.
               Added FResourceString copy ctor and assignment
  95-03-11 ISJ Made it DLLable
  95-03-14 ISJ Made BC4 happy (operator new)
  96-??-?? ISJ removed DLL support (too ugly)
***/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#include <bif.h>
#include <dbglog.h>
#include <stddef.h>


// All the resource objects except FResourceString uses 'reference counting'
// so we can assign, return, modify and so forth without worrying about
// double-destruction or excessive use of memory
struct FResourceObject_rep {
        int refs;
        Bool shouldDelete;
        HANDLE handle;
};

FResourceObject::FResourceObject( Bool d )
{
        rep = new FResourceObject_rep;
        rep->refs = 1;
        rep->shouldDelete = d;
        rep->handle = NULL;
}

FResourceObject::FResourceObject(const FResourceObject &ro)
{
        rep = ro.rep;
        inc();
}

FResourceObject::~FResourceObject( void )
{
        if( rep->refs == 0)
                delete rep;
}

void FResourceObject::operator=(const FResourceObject &ro)
{
        if( rep->refs == 0 )
                delete rep;

        rep = ro.rep;
        inc();
}

int FResourceObject::inc( void )
{
        return ++(rep->refs);
}

int FResourceObject::dec( void )
{
        return --(rep->refs);
}

//A little function so we dont have to repeat ourselves too much
static inline FModule *DefModule(FModule *module) {
        return module?module:FModule::GetDefaultResourceModule();
}

////////////////////////////
// implimentation of FIcon

FIcon::FIcon(HICON hicon, Bool d)
  : FResourceObject(d)
{
        rep->handle = hicon;
}

FIcon::FIcon(int resId, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadIcon( resId );
}

FIcon::FIcon(const char far *pszName, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadIcon(pszName);
}

FIcon::FIcon(const FIcon &i)
  : FResourceObject(i)
{}

FIcon::~FIcon( void )
{
        if(dec() == 0 && rep->shouldDelete && rep->handle)
                DestroyIcon((HICON)rep->handle);
}

HICON FIcon::GetHandle( void )
{
        return (HICON)rep->handle;
}

FIcon& FIcon::operator=(const FIcon &i)
{
        if(&i!=this) {
                if(dec() == 0 && rep->shouldDelete && rep->handle)
                        DestroyIcon((HICON)rep->handle);
                FResourceObject::operator = (i);
        }
        return *this;
}

////////////////////////////////////////////
// Implementation af FPointer

FPointer::FPointer(HCURSOR hcurs, Bool d)
  : FResourceObject(d)
{
        rep->handle=hcurs;
}

FPointer::FPointer(const FPointer& p)
  : FResourceObject(p)
{

}

FPointer::FPointer(int resId, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadCursor(resId);
}

FPointer::FPointer(const char far *pszName, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadCursor(pszName);
}

FPointer::~FPointer()
{
        if(dec() == 0 && rep->shouldDelete && rep->handle)
                DestroyCursor((HCURSOR)rep->handle);
}

HCURSOR FPointer::GetHandle( void )
{
        return (HCURSOR)rep->handle;
}

FPointer& FPointer::operator=(const FPointer &p)
{
        if(&p!=this) {
                if(dec()==0 && rep->shouldDelete && rep->handle)
                        DestroyCursor((HCURSOR)rep->handle);
                FResourceObject::operator = (p);
        }
        return *this;
}

///////////////////////////////////////////
// Implementation af FSystemPointer

class FSystemPointer : public FPointer {
public:
        FSystemPointer(const char far *id)
        : FPointer(LoadCursor(NULL,id),False)
        {}
};

static FSystemPointer sys_arrowPointer(IDC_ARROW);
static FSystemPointer sys_textPointer(IDC_IBEAM);
static FSystemPointer sys_neswPointer(IDC_SIZENESW);
static FSystemPointer sys_nsPointer(IDC_SIZENS);
static FSystemPointer sys_nwsePointer(IDC_SIZENWSE);
static FSystemPointer sys_wePointer(IDC_SIZEWE);
static FSystemPointer sys_waitPointer(IDC_WAIT);

FPointer * const BIFFUNCTION GetSystemArrowPointer( void ) {
        return &sys_arrowPointer;
}
FPointer * const BIFFUNCTION GetSystemTextPointer( void ) {
        return &sys_textPointer;
}
FPointer * const BIFFUNCTION GetSystemNESWPointer( void ) {
        return &sys_neswPointer;
}
FPointer * const BIFFUNCTION GetSystemNSPointer( void ) {
        return &sys_nsPointer;
}
FPointer * const BIFFUNCTION GetSystemNWSEPointer( void ) {
        return &sys_nwsePointer;
}
FPointer * const BIFFUNCTION GetSystemWEPointer( void ) {
        return &sys_wePointer;
}
FPointer * const BIFFUNCTION GetSystemWaitPointer( void ) {
        return &sys_waitPointer;
}

///////////////////////////////////
// Implementation of FMenu

FMenu::FMenu(int resId, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadMenu(resId);
}

FMenu::FMenu(const char far *pszMenu, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadMenu(pszMenu);
}

FMenu::FMenu(FWnd *, int resId, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadMenu(resId);
}

FMenu::FMenu(FWnd *, const char far *pszMenu, FModule *module)
  : FResourceObject(True)
{
        rep->handle = DefModule(module)->LoadMenu(pszMenu);
}

FMenu::FMenu(HMENU hmenu, Bool d)
  : FResourceObject(d)
{
        rep->handle=hmenu;
}

FMenu::FMenu(const FMenu &m)
  : FResourceObject(m)
{
}

FMenu::~FMenu()
{
        if(dec() == 0 && rep->shouldDelete && rep->handle)
                DestroyMenu( (HMENU)rep->handle);
}

FMenu& FMenu::operator=(const FMenu &m) {
        if(&m!=this) {
                if(dec()==0 && rep->shouldDelete && rep->handle)
                        DestroyMenu((HMENU)rep->handle);
                FResourceObject::operator = (m);
        }
        return *this;
}

HMENU FMenu::GetHandle( void )
{
        return (HMENU)rep->handle;
}

Bool FMenu::Popup(FWnd *parent, FWnd *owner, const FPoint *position) {
        //find real HMENU (tracking top-level menus doesn't work under Windows :-(
        HMENU hmenu;
        if(GetMenuItemCount(GetHandle())==1 &&          //only one item
           GetMenuItemID(GetHandle(),0)==(UINT)-1)      //and it's a popupmenu
                hmenu=GetSubMenu(GetHandle(),0);
        else
                hmenu=GetHandle();
                
        //calc position
        int x,y;
        if(position) {
                FPoint p=*position;
                MapWindowPoints(parent->GetHwnd(), HWND_DESKTOP, &p, 1);
                x=p.GetX();
                y=p.GetY();
        } else {
                FPoint p;
                GetCursorPos(&p);
                x=p.GetX();
                y=p.GetY();
        }

        //calc flags
        UINT fuFlags=0;
        if(((UINT)GetKeyState(VK_RBUTTON)) & 0x8000U)
                fuFlags|= TPM_RIGHTBUTTON;
        else
                fuFlags|= TPM_LEFTBUTTON;

        fuFlags|=TPM_LEFTALIGN;
        
        return (Bool)TrackPopupMenu(hmenu,
                                    fuFlags,
                                    x,y,
                                    0,
                                    owner->GetHwnd(),
                                    NULL);
}

Bool FMenu::AppendItem(const char far *pszItem, int nId, Bool enabled, Bool checked )
{
        unsigned int nAttr = MF_STRING;

        nAttr |= (enabled) ? MF_ENABLED : MF_DISABLED;
        nAttr |= (checked) ? MF_CHECKED : MF_UNCHECKED;

        return((Bool)AppendMenu( (HMENU)rep->handle, nAttr, nId, pszItem ));
}

Bool FMenu::AppendItem(const char far *pszItem, FMenu *menu, Bool enabled )
{
        UINT flag = MF_POPUP;

        if(!enabled)
                flag |= MF_GRAYED|MF_DISABLED;

        return (Bool)AppendMenu( (HMENU)rep->handle, flag, (UINT)(menu->GetHandle()), pszItem );
}

void FMenu::AppendSeparator( void )
{
        AppendMenu( (HMENU)rep->handle, MF_SEPARATOR, 0, 0);
}

Bool FMenu::DeleteItem(int nId)
{
        return((Bool)DeleteMenu( (HMENU)rep->handle, nId, MF_BYCOMMAND ));
}


Bool FMenu::GetItem(int Id, char far *pBuf, int buflen)
{
        if(GetMenuString((HMENU)rep->handle, Id, pBuf, buflen, MF_BYCOMMAND)==0)
                return False;
        else
                return True;
}

FMenu FMenu::GetSubmenuFromPos(int nPos)
{
        HMENU hm = (HMENU)rep->handle;
        HMENU hmsub = GetSubMenu(hm,nPos);

        return FMenu(hmsub,False);
}

void FMenu::CheckItem(int Id, Bool check)
{
        CheckMenuItem((HMENU)rep->handle, Id,MF_BYCOMMAND|(check?MF_CHECKED:MF_UNCHECKED));
}

void FMenu::EnableItem(int Id, Bool enable)
{
        EnableMenuItem((HMENU)rep->handle, Id, MF_BYCOMMAND|(enable?MF_ENABLED:MF_DISABLED|MF_GRAYED));
}

Bool FMenu::IsItemEnabled(int Id)
{
        UINT state = GetMenuState((HMENU)rep->handle, Id, MF_BYCOMMAND);

        if(state == (UINT)-1)
                return False;   //error

        if(state & MF_DISABLED)
                return False;
        else
                return True;
}

Bool FMenu::IsItemChecked(int Id)
{
        UINT state = GetMenuState((HMENU)rep->handle, Id, MF_BYCOMMAND);

        if(state == (UINT)-1)
                return False;   //error

        if(state & MF_CHECKED)
                return False;
        else
                return True;
}

////////////////////////////////////////
// Implementation of FResourceString 

FResourceString::FResourceString(int resId, FModule *module)
{
        module = DefModule(module);

        char tmp[ 256 ];
        if(module->LoadString(resId,tmp,256) == 0)
                s = 0;
        else {
                s = new char[ lstrlen(tmp)+1 ];
                if(s)
                        lstrcpy(s,tmp);
        }
}

FResourceString::~FResourceString() {
        delete[] s;
}

FResourceString::FResourceString(const FResourceString &rs) {
        if(rs.s) {
                s = new char[lstrlen(rs.s)+1];
                if(s)
                        lstrcpy(s,rs.s);
        } else
                s=0;
}

FResourceString &FResourceString::operator=(const FResourceString &rs) {
        //avoid unnecessarry newing & deleting
        if(rs.s) {
                if(s && lstrlen(s)>=lstrlen(rs.s))
                        lstrcpy(s,rs.s);
                else {
                        delete[] s;
                        s = new char[lstrlen(rs.s)+1];
                        if(s)
                                lstrcpy(s,rs.s);
                }
        }
        return *this;
}

