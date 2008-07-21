#ifndef __WNDMISC_H_INCLUDED
#define __WNDMISC_H_INCLUDED

class FWnd;     //forward
struct WmsQMSG;

//WMS declares FPoint and FRect suitable to be used directly

//A few rectangle functions
inline FRect Intersection(const FRect &r1, const FRect &r2) {
        return r1.Intersects(r2)
               ? FRect(r1.xLeft>r2.xLeft?r1.xLeft:r2.xLeft, r1.yBottom>r2.yBottom?r1.yBottom:r2.yBottom, r1.xRight<r2.xRight?r1.xRight:r2.xRight, r1.yTop<r2.yTop?r1.yTop:r2.yTop)
               : FRect(0,0,0,0);
}

inline FRect Union(const FRect &r1, const FRect &r2) {
        return FRect(r1.xLeft<r2.xLeft?r1.xLeft:r2.xLeft, r1.yBottom<r2.yBottom?r1.yBottom:r2.yBottom, r1.xRight>r2.xRight?r1.xRight:r2.xRight, r1.yTop>r2.yTop?r1.yTop:r2.yTop);
}

class FModule;
//Icons, Pointers and Menus

class FIcon {
        const void *r;
public:
        FIcon(int resID, FModule *module=0);
        const void *GetIcon() const { return r; }
};
class FPointer {
        const void *r;
public:
        FPointer(int resID, FModule *module=0);
        const void *GetPointer() const { return r; }
        void UsePointer(); //very, very, very undocumented
};

FPointer * const BIFFUNCTION GetSystemArrowPointer();     //normal arrow pointer
FPointer * const BIFFUNCTION GetSystemTextPointer();      //pointer for text (I-beam)
FPointer * const BIFFUNCTION GetSystemNESWPointer();      //north-east to south-west
FPointer * const BIFFUNCTION GetSystemNSPointer();        //north to south
FPointer * const BIFFUNCTION GetSystemNWSEPointer();      //north-west to south-east
FPointer * const BIFFUNCTION GetSystemWEPointer();        //west to east
FPointer * const BIFFUNCTION GetSystemWaitPointer();      //ourglass

class BIFCLASS FMenu {
        struct Rep {
                int refs;
                FWnd *pwnd;
                Bool shouldDelete;
        } *rep;
public:
        FMenu(WmsHWND hwndOwner, int resId, FModule *module=0);
        FMenu(FWnd *pwndOwner, int resId, FModule *module=0);
        FMenu(WmsHWND hmenu, Bool d=False);
        virtual ~FMenu();

        Bool Popup(FWnd *parent, FWnd *owner, const FPoint *position=0);

        Bool AppendItem(const char *string, int nId, Bool enabled = True, Bool checked = False);
        Bool AppendItem(const char *pszItem, FMenu *submenu, Bool enabled);
        void AppendSeparator();
        Bool DeleteItem(int Id);

        Bool GetItem(int Id, char *buf, int buflen);
        FMenu GetSubmenuFromPos(int nPos);

        void CheckItem(int Id, Bool check);
        void EnableItem(int Id, Bool enable);
        Bool IsItemEnabled(int Id);
        Bool IsItemChecked(int Id);

        WmsHWND GetHandle();

        FMenu(const FMenu&);
        FMenu& operator=(const FMenu&);
//very undocumented:
        void TurnIntoActionBar();
        void EnterActionBar();
};

class BIFCLASS FResourceString {
        char *s;
        FResourceString(const FResourceString &); //don't copy
        FResourceString& operator=(const FResourceString &); //don't copy
public:
        FResourceString(int resId, FModule *module=0);
        virtual ~FResourceString();

        operator const char *() const { return s; }
};

class BIFCLASS FAcceleratorTable {
        const void *r;
public:
        FAcceleratorTable(int resID, FModule *module=0);
        const void *GetAccelTable() const { return r; }
};

#endif

