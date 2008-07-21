#ifndef __REGION_HPP
#define __REGION_HPP

class Region {
        struct link {
                link *next;
                FRect r;
        } *areas;
        Region(const Region &r);                //dont copy
        Region& operator=(const Region &r);     //dont copy
public:
        Region();
        ~Region();

        Bool Copy(const Region &r);

        Bool Include(const FRect &r);
        Bool Include(const Region &r);
        Bool Exclude(const FRect &r);
        Bool Exclude(const Region &r);
        void Clear();
        Bool Intersect(const Region &r);

        void Clip(const FRect &r);
        int IsEmpty() const;
        FRect QueryBoundsRect() const;
        Bool Includes(const FPoint &p);

        int QueryRectCount() const;
        int QueryRects(FRect *r, int maxelem) const;

        void Optimize();

        void Move(int dx, int dy);
};

#endif
