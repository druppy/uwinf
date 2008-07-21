#ifndef __WNDMISC_H_INCLUDED
#define __WNDMISC_H_INCLUDED

//The boolean type used by BIF/Window
enum Bool {False, True};


class BIFCLASS FWnd;     //forward
/*******************************
        Some small useful classes
*******************************/
class BIFCLASS FPoint : public POINT {
public:
        FPoint()
          {}
        FPoint(int xx, int yy)
          { x=xx; y=yy; }
        FPoint( POINT p)
          { x=p.x; y=p.y; }

        void Set( int xx, int yy )
          { x=xx; y=yy; }
        void Set( POINT p )
          { x=p.x; y=p.y; }

        int GetX() const { return x; }
        int GetY() const { return y; }
        
        FPoint operator+(POINT p)       { return FPoint( x+p.x, y+p.y ); }
        FPoint operator-(POINT p)       { return FPoint( x-p.x, y-p.y ); }

        int operator==(const FPoint &p) const { return x==p.x && y==p.y; }
        int operator!=(const FPoint &p) const { return x!=p.x || y!=p.y; }
};

class BIFCLASS FRect : public RECT {
public:
        FRect() {}
        FRect( int l, int t, int r, int b )
                {left = l;top=t;right=r;bottom=b;}
        FRect(RECT r)
                {left = r.left;top=r.top;right=r.right;bottom=r.bottom;}

        int GetTop( void ) const {return(top);}
        void SetTop(int t) { top=t; }
        int GetLeft( void ) const {return(left);}
        void SetLeft(int l) { left=l; }
        int GetRight( void ) const {return(right);}
        void SetRight(int r) { right=r; }
        int GetBottom( void ) const {return(bottom);}
        void SetBottom(int b) { bottom=b; }

        int GetWidth() const { return right-left; }
        int GetHeight() const { return bottom-top; }
        void Shrink(int dx, int dy) { left+=dx; right-=dx; top+=dy; bottom-=dy; }
        void Expand(int dx, int dy) { left-=dx; right+=dx; top-=dy; bottom+=dy; }
        Bool Contains(const FPoint &p) const { return Bool(left<=p.x && p.x<right && top<=p.y && p.y<bottom); }
        Bool Contains(const FRect &r) const { return Bool(left<=r.left && r.right<=right && top<=r.top && r.bottom<=bottom); }
        Bool Intersects(const FRect &r) const { return Bool(r.left<right && r.right>left && r.top<bottom && r.bottom>top); }
        Bool IsEmpty() const { return Bool(left==right || top==bottom); }
};

inline FRect Intersection(const FRect &r1, const FRect &r2) {
        return r1.Intersects(r2)
               ? FRect(r1.left>r2.left?r1.left:r2.left, r1.top>r2.top?r1.top:r2.top, r1.right<r2.right?r1.right:r2.right, r1.bottom<r2.bottom?r1.bottom:r2.bottom)
               : FRect(0,0,0,0);
}

inline FRect Union(const FRect &r1, const FRect &r2) {
        return FRect(r1.left<r2.left?r1.left:r2.left, r1.top<r2.top?r1.top:r2.top, r1.right>r2.right?r1.right:r2.right, r1.bottom>r2.bottom?r1.bottom:r2.bottom);
}

        
class BIFCLASS FModule;
//Icons, Pointers and Menus
struct FResourceObject_rep;
class BIFCLASS FResourceObject {
protected:
        static FModule *DefModule();
        FResourceObject(Bool d);
        FResourceObject(const FResourceObject&);
        virtual ~FResourceObject();

        void operator=(const FResourceObject &ro);
        FResourceObject_rep *rep;
        int inc();
        int dec();
};

class BIFCLASS FIcon : protected FResourceObject {
public:
        FIcon(HICON hicon, Bool d=False);
        FIcon(const FIcon&);
        FIcon(int resId, FModule *module=0);
        FIcon(const char *lpszName, FModule *module=0);
        virtual ~FIcon();

        HICON GetHandle();

        FIcon& operator=(const FIcon&);
};

class BIFCLASS FPointer : protected FResourceObject {
public:
        FPointer(HCURSOR hcurs, Bool d=False);
        FPointer(const FPointer& p);
        FPointer(int resId, FModule *module=0);
        FPointer(const char *pszName, FModule *module=0);
        virtual ~FPointer();

        HCURSOR GetHandle();
        FPointer& operator=(const FPointer &p);
};

FPointer * const BIFFUNCTION GetSystemArrowPointer();   //normal arrow pointer
FPointer * const BIFFUNCTION GetSystemTextPointer();    //pointer for text (I-beam)
FPointer * const BIFFUNCTION GetSystemNESWPointer();    //north-east to south-west
FPointer * const BIFFUNCTION GetSystemNSPointer();      //north to south
FPointer * const BIFFUNCTION GetSystemNWSEPointer();    //north-west to south-east
FPointer * const BIFFUNCTION GetSystemWEPointer();      //west to east
FPointer * const BIFFUNCTION GetSystemWaitPointer();    //ourglass

class BIFCLASS FMenu : protected FResourceObject {
public:
        FMenu(int resId, FModule *module=0);            //win16
        FMenu(const char *pszName, FModule *module=0);  //win16
        FMenu(FWnd *pwndOwner, int resId, FModule *module=0);
        FMenu(FWnd *pwndOwner, const char *pszMenu, FModule *module=0);
        FMenu(HMENU hmenu, Bool d=False);
        FMenu(const FMenu&);
        FMenu& operator=(const FMenu&);
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

        HMENU GetHandle();

};


class BIFCLASS FResourceString {
        char *s;
public:
        FResourceString(int resId, FModule *module=0);
        virtual ~FResourceString();
        FResourceString(const FResourceString &rs);
        FResourceString& operator=(const FResourceString &rs);
        
        operator const char *() const { return s; }
};

#endif

