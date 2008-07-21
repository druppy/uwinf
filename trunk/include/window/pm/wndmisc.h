#ifndef __WNDMISC_H_INCLUDED
#define __WNDMISC_H_INCLUDED

enum Bool { False, True };

class FWnd;     //forward
//  Some small basic useful classes

class BIFCLASS FPoint : public POINTL {
public:
        FPoint() 
                {}
        FPoint(int xx, int yy) 
                { x=xx; y=yy; }
        FPoint( POINTL p) 
                { x=p.x; y=p.y; }

        void Set( int xx, int yy ) 
                { x=xx; y=yy; }
        void Set( POINTL p ) 
                { x=p.x; y=p.y; }

        int GetX() const { return x; }
        int GetY() const { return y; }
        
        FPoint operator+(POINTL p)       {return FPoint( (int)(x+p.x), (int)(y+p.y) );}
        FPoint operator-(POINTL p)       {return FPoint( (int)(x-p.x), (int)(y-p.y) );}

        int operator==(const FPoint &p) const { return x==p.x && y==p.y; }
        int operator!=(const FPoint &p) const { return x!=p.x || y!=p.y; }
};

class BIFCLASS FRect : public RECTL {
public:
        FRect() {}
        FRect( int l, int b, int r, int t )
                {xLeft=l; yTop=t; xRight=r; yBottom=b;}
        FRect(const RECTL &r)
                {xLeft=r.xLeft; yTop=r.yTop; xRight=r.xRight; yBottom=r.yBottom;}

        int GetTop( void ) const {return yTop;}
        void SetTop(int t) { yTop=t; }
        int GetLeft( void ) const {return xLeft;}
        void SetLeft(int l) { xLeft=l; }
        int GetRight( void ) const {return xRight;}
        void SetRight(int r) { xRight=r; }
        int GetBottom( void ) const {return yBottom;}
        void SetBottom(int b) { yBottom=b; }

        int GetWidth() const { return (int)(xRight-xLeft); }
        int GetHeight() const { return (int)(yTop-yBottom); }
        void Shrink(int dx, int dy) { xLeft+=dx; xRight-=dx; yTop-=dy; yBottom+=dy; }
        void Expand(int dx, int dy) { xLeft-=dx; xRight+=dx; yTop+=dy; yBottom-=dy; }

        Bool Contains(const FPoint &p) const { return Bool(xLeft<=p.x && p.x<xRight && yBottom<=p.y && p.y<yTop); }
        Bool Contains(const FRect &r) const { return Bool(xLeft<=r.xLeft && r.xRight<=xRight && yBottom<=r.yBottom && r.yTop<=yTop); }
        Bool Intersects(const FRect &r) const { return Bool(xLeft<r.xRight && xRight>r.xLeft && yBottom<r.yTop && yTop>r.yBottom); }
        Bool IsEmpty() const { return (Bool)(xLeft==xRight || yTop==yBottom); }
};

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
struct FResourceObject_rep;
class BIFCLASS FResourceObject {
protected:
        FResourceObject(Bool d);
        FResourceObject(const FResourceObject&);
        virtual ~FResourceObject();

        void operator=(const FResourceObject& ro);
        FResourceObject_rep *rep;
        int inc();
        int dec();
};

class BIFCLASS FIcon : protected FResourceObject {
public:
        FIcon(HPOINTER hicon, Bool d=False);
        FIcon(const FIcon&);
        FIcon(int resId, FModule *module=0);
        virtual ~FIcon();

        HPOINTER GetHandle();

        FIcon& operator=(const FIcon&);
};

class BIFCLASS FPointer : protected FResourceObject {
public:
        FPointer(HPOINTER hptr, Bool d=False);
        FPointer(const FPointer& p);
        FPointer(int resId, FModule *module=0);
        virtual ~FPointer();

        HPOINTER GetHandle();
        FPointer& operator=(const FPointer& p);
};

FPointer * const BIFFUNCTION GetSystemArrowPointer();     //normal arrow pointer
FPointer * const BIFFUNCTION GetSystemTextPointer();      //pointer for text (I-beam)
FPointer * const BIFFUNCTION GetSystemNESWPointer();      //north-east to south-west
FPointer * const BIFFUNCTION GetSystemNSPointer();        //north to south
FPointer * const BIFFUNCTION GetSystemNWSEPointer();      //north-west to south-east
FPointer * const BIFFUNCTION GetSystemWEPointer();        //west to east
FPointer * const BIFFUNCTION GetSystemWaitPointer();      //hourglass


class BIFCLASS FMenu : protected FResourceObject {
public:
        FMenu(HWND hwndOwner, int resId, FModule *module=0);
        FMenu(FWnd *pwndOwner, int resId, FModule *module=0);
        FMenu(HWND hmenu, Bool d=False);
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

        HWND GetHandle();

        FMenu(const FMenu&);
        FMenu& operator=(const FMenu&);
};


class BIFCLASS FResourceString {
        char *s;
public:
        FResourceString(int resId, FModule *module=0);
        virtual ~FResourceString();
        FResourceString(const FResourceString &);
        FResourceString& operator=(const FResourceString &);

        operator const char *() const { return s; }
};

#endif

