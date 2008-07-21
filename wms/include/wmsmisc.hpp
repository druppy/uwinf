#ifndef __MISC_HPP
#define __MISC_HPP

struct FPoint {
        int x,y;

        FPoint()
        {}
        FPoint(int xx,int yy) {
                x=xx; y=yy;
        }

        int GetX() const { return x; }
        int GetY() const { return y; }

        void SetX(int xx) { x=xx; }
        void SetY(int yy) { y=yy; }
        void Set(int xx,int yy) { x=xx; y=yy; }

        int operator==(const FPoint &p) const { return x==p.x && y==p.y; }
        int operator!=(const FPoint &p) const { return x!=p.x || y!=p.y; }

};

struct FRect {
        int xLeft,yTop;
        int xRight,yBottom;

        FRect()
        {}

        FRect(int l, int t, int r, int b) {
                xLeft=l; yTop=t; xRight=r; yBottom=b;
        }

        int GetLeft() const     { return xLeft; }
        int GetTop() const      { return yTop; }
        int GetRight() const    { return xRight; }
        int GetBottom() const   { return yBottom; }
        int GetWidth() const    { return xRight-xLeft; }
        int GetHeight() const   { return yBottom-yTop; }

        void SetLeft(int l)     { xLeft=l; }
        void SetTop(int t)      { yTop=t; }
        void SetRight(int r)    { xRight=r; }
        void SetBottom(int b)   { yBottom=b; }
        void Set(int l, int t, int r, int b) {
                xLeft=l; yTop=t; xRight=r; yBottom=b;
        }
        void SetWidth(int w)    { xRight=xLeft+w; }
        void SetHeight(int h)   { yBottom=yTop+h; }
        void MoveRelative(int dx, int dy) {
                xLeft+=dx; xRight+=dx; yTop+=dy; yBottom+=dy;
        }
        void MoveRelative(const FPoint &p) {
                MoveRelative(p.x,p.y);
        }
        void MoveAbsolute(int x, int y) {
                int w=xRight-xLeft;
                int h=yBottom-yTop;
                xLeft=x; xRight = xLeft+w;
                yTop=y; yBottom = yTop+h;
        }
        void MoveAbsolute(const FPoint &p) {
                MoveAbsolute(p.x,p.y);
        }

        Bool IsEmpty() const {
                return (Bool)(xLeft>=xRight || yTop>=yBottom);
        }

        Bool Contains(const FPoint &p) const {
                return (Bool)(p.x>=xLeft && p.x<xRight &&
                              p.y>=yTop && p.y<yBottom);
        }
        Bool Contains(const FRect &r) const {
                return (Bool)(r.xLeft>=xLeft && r.xRight<=xRight &&
                              r.yTop>=yTop && r.yBottom<=yBottom);
        }
        Bool Intersects(const FRect &r) const {
                return (Bool)(xLeft<r.xRight && xRight>r.xLeft &&
                              yTop<r.yBottom && yBottom>r.yTop);
        }
        void Shrink(int dx, int dy) { xLeft+=dx; xRight-=dx; yTop+=dy; yBottom-=dy; }
        void Expand(int dx, int dy) { xLeft-=dx; xRight+=dx; yTop-=dy; yBottom+=dy; }

};

#endif
