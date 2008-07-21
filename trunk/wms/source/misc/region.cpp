#define WMSINCL_REGION
#include <wms.hpp>

Region::Region() {
        areas=0;
}


Region::~Region() {
        link *l=areas;
        while(l) {
                link *next=l->next;
                delete l;
                l=next;
        }
}

Bool Region::Copy(const Region &r) {
        if(&r==this) return True;

        Clear();
        link *prev=0;
        for(link *c=r.areas; c; c=c->next) {
                link *l=new link;
                if(!l) return False;
                l->r=c->r;
                l->next=0;
                if(prev) {
                        prev->next=l;
                        prev=l;
                } else {
                        areas=l;
                        prev=l;
                }
        }
        return True;
}


static int Split(const FRect &r, const FRect &sr, FRect *ra) {
        if(sr.xRight<=r.xLeft) return 1;        //split not necessary
        if(sr.xLeft>=r.xRight) return 1;        //ditto
        if(sr.yBottom<=r.yTop) return 1;        //ditto
        if(sr.yTop>=r.yBottom) return 1;        //ditto

        if(sr.xLeft<=r.xLeft && sr.xRight<r.xRight) {
                // sl rl sr rr
                if(sr.yTop<=r.yTop && sr.yBottom<r.yBottom) {
                        //  ⁄ƒø
                        //  ≥…ÿÕª
                        //  ¿◊Ÿ ∫
                        //   ∫  ∫
                        //   »ÕÕº
                        //
                        ra[0].Set(r.xLeft,  r.yTop,    sr.xRight,sr.yBottom);
                        ra[1].Set(sr.xRight,r.yTop,    r.xRight, sr.yBottom);
                        ra[2].Set(r.xLeft,  sr.yBottom,r.xRight, r.yBottom);
                        return 3;
                } else if(sr.yTop<=r.yTop && sr.yBottom>=r.yBottom) {
                        //  ⁄ƒø
                        //  ≥…ÿÕª
                        //  ≥∫≥ ∫
                        //  ≥∫≥ ∫
                        //  ≥»ÿÕº
                        //  ¿ƒŸ
                        ra[0].Set(r.xLeft,  r.yTop,sr.xRight,r.yBottom);
                        ra[1].Set(sr.xRight,r.yTop,r.xRight, r.yBottom);
                        return 2;
                } else if(sr.yBottom<r.yBottom) {
                        //
                        //   …ÕÕª
                        //  ⁄◊ø ∫
                        //  ¿◊Ÿ ∫
                        //   »ÕÕº
                        //
                        ra[0].Set(r.xLeft,  r.yTop,    r.xRight, sr.yTop);
                        ra[1].Set(r.xLeft,  sr.yTop,   sr.xRight,sr.yBottom);
                        ra[2].Set(sr.xRight,sr.yTop,   r.xRight, sr.yBottom);
                        ra[3].Set(r.xLeft,  sr.yBottom,r.xRight, r.yBottom);
                        return 4;
                } else {
                        //
                        //   …ÕÕª
                        //   ∫  ∫
                        //  ⁄◊ø ∫
                        //  ≥»ÿÕº
                        //  ¿ƒŸ
                        ra[0].Set(r.xLeft,  r.yTop, r.xRight, sr.yTop);
                        ra[1].Set(r.xLeft,  sr.yTop,sr.xRight,r.yBottom);
                        ra[2].Set(sr.xRight,sr.yTop,r.xRight, r.yBottom);
                        return 3;
                }
        } else if(sr.xLeft<=r.xLeft && sr.xRight>=r.xRight) {
                // sl rl rr sr
                if(sr.yTop<=r.yTop && sr.yBottom<r.yBottom) {
                        //  ⁄ƒƒƒƒø
                        //  ≥…ÕÕª≥
                        //  ¿◊ƒƒ◊Ÿ
                        //   ∫  ∫
                        //   »ÕÕº
                        //
                        ra[0].Set(r.xLeft,r.yTop,    r.xRight,sr.yBottom);
                        ra[1].Set(r.xLeft,sr.yBottom,r.xRight,r.yBottom);
                        return 2;
                } else if(sr.yTop<=r.yTop && sr.yBottom>=r.yBottom) {
                        //  ⁄ƒƒƒƒø
                        //  ≥…ÕÕª≥
                        //  ≥∫  ∫≥
                        //  ≥∫  ∫≥
                        //  ≥»ÕÕº≥
                        //  ¿ƒƒƒƒŸ
                        return 1;        //split not necessary
                } else if(sr.yBottom<r.yBottom) {
                        //
                        //   …ÕÕª
                        //  ⁄◊ƒƒ◊ø
                        //  ¿◊ƒƒ◊Ÿ
                        //   »ÕÕº
                        //
                        ra[0].Set(r.xLeft,r.yTop,    r.xRight,sr.yTop);
                        ra[1].Set(r.xLeft,sr.yTop,   r.xRight,sr.yBottom);
                        ra[2].Set(r.xLeft,sr.yBottom,r.xRight,r.yBottom);
                        return 3;
                } else {
                        //
                        //   …ÕÕª
                        //   ∫  ∫
                        //  ⁄◊ƒƒ◊ø
                        //  ≥»ÕÕº≥
                        //  ¿ƒƒƒƒŸ
                        ra[0].Set(r.xLeft,r.yTop, r.xRight,sr.yTop);
                        ra[1].Set(r.xLeft,sr.yTop,r.xRight,r.yBottom);
                        return 2;
                }
        } else if(sr.xRight<r.xRight) {
                // rl sl sr rr
                if(sr.yTop<=r.yTop && sr.yBottom<r.yBottom) {
                        //    ⁄ø
                        //   …ÿÿª
                        //   ∫¿Ÿ∫
                        //   ∫  ∫
                        //   »ÕÕº
                        //
                        ra[0].Set(r.xLeft,  r.yTop,    sr.xLeft, sr.yBottom);
                        ra[1].Set(sr.xLeft, r.yTop,    sr.xRight,sr.yBottom);
                        ra[2].Set(sr.xRight,r.yTop,    r.xRight, sr.yBottom);
                        ra[3].Set(r.xLeft,  sr.yBottom,r.xRight, r.yBottom);
                        return 4;
                } else if(sr.yTop<=r.yTop && sr.yBottom>=r.yBottom) {
                        //    ⁄ø
                        //   …ÿÿª
                        //   ∫≥≥∫
                        //   ∫≥≥∫
                        //   »ÿÿº
                        //    ¿Ÿ
                        ra[0].Set(r.xLeft,  r.yTop,sr.xLeft, r.yBottom);
                        ra[1].Set(sr.xLeft, r.yTop,sr.xRight,r.yBottom);
                        ra[2].Set(sr.xRight,r.yTop,r.xRight, r.yBottom);
                        return 3;
                } else if(sr.yBottom<r.yBottom) {
                        //
                        //   …ÕÕª
                        //   ∫⁄ø∫
                        //   ∫¿Ÿ∫
                        //   »ÕÕº
                        //
                        ra[0].Set(r.xLeft,  r.yTop,    r.xRight, sr.yTop);
                        ra[1].Set(r.xLeft,  sr.yTop,   sr.xLeft, sr.yBottom);
                        ra[2].Set(sr.xLeft, sr.yTop,   sr.xRight,sr.yBottom);
                        ra[3].Set(sr.xRight,sr.yTop,   r.xRight, sr.yBottom);
                        ra[4].Set(r.xLeft,  sr.yBottom,r.xRight, r.yBottom);
                        return 5;
                } else {
                        //
                        //   …ÕÕª
                        //   ∫  ∫
                        //   ∫⁄ø∫
                        //   »ÿÿº
                        //    ¿Ÿ
                        ra[0].Set(r.xLeft,  r.yTop, r.xRight, sr.yTop);
                        ra[1].Set(r.xLeft,  sr.yTop,sr.xLeft, r.yBottom);
                        ra[2].Set(sr.xLeft, sr.yTop,sr.xRight,r.yBottom);
                        ra[3].Set(sr.xRight,sr.yTop,r.xRight, r.yBottom);
                        return 4;
                }
        } else {
                // rl sl rr sr
                if(sr.yTop<=r.yTop && sr.yBottom<r.yBottom) {
                        //     ⁄ƒø
                        //   …Õÿª≥
                        //   ∫ ¿◊Ÿ
                        //   ∫  ∫
                        //   »ÕÕº
                        //
                        ra[0].Set(r.xLeft, r.yTop,    sr.xLeft,sr.yBottom);
                        ra[1].Set(sr.xLeft,r.yTop,    r.xRight,sr.yBottom);
                        ra[2].Set(r.xLeft, sr.yBottom,r.xRight,r.yBottom);
                        return 3;
                } else if(sr.yTop<=r.yTop && sr.yBottom>=r.yBottom) {
                        //     ⁄ƒø
                        //   …Õÿª≥
                        //   ∫ ≥∫≥
                        //   ∫ ≥∫≥
                        //   »Õÿº≥
                        //     ¿ƒŸ
                        ra[0].Set(r.xLeft, r.yTop,sr.xLeft,r.yBottom);
                        ra[1].Set(sr.xLeft,r.yTop,r.xRight,r.yBottom);
                        return 2;
                } else if(sr.yBottom<r.yBottom) {
                        //
                        //   …ÕÕª
                        //   ∫ ⁄◊ø
                        //   ∫ ¿◊Ÿ
                        //   »ÕÕº
                        //
                        ra[0].Set(r.xLeft, r.yTop,    r.xRight,sr.yTop);
                        ra[1].Set(r.xLeft, sr.yTop,   sr.xLeft,sr.yBottom);
                        ra[2].Set(sr.xLeft,sr.yTop,   r.xRight,sr.yBottom);
                        ra[3].Set(r.xLeft, sr.yBottom,r.xRight,r.yBottom);
                        return 4;
                } else {
                        //
                        //   …ÕÕª
                        //   ∫  ∫
                        //   ∫ ⁄◊ø
                        //   »Õÿº≥
                        //     ¿ƒŸ
                        ra[0].Set(r.xLeft, r.yTop, r.xRight,sr.yTop);
                        ra[1].Set(r.xLeft, sr.yTop,sr.xLeft,r.yBottom);
                        ra[2].Set(sr.xLeft,sr.yTop,r.xRight,r.yBottom);
                        return 3;
                }
        }
}


Bool Region::Include(const FRect &r) {
        if(r.IsEmpty()) return True;
        //use Exlude to remove overlapping rects
        if(!Exclude(r)) return False;
        //and then insert rect
        link *l=new link;
        if(!l) return False;
        l->r=r;
        l->next=areas;
        areas=l;
        return True;
}

Bool Region::Include(const Region &r) {
        if(&r==this) return True;
        //union
        for(link *l=r.areas; l; l=l->next)
                if(!Include(l->r))
                        return False;

        return True;
}

Bool Region::Exclude(const FRect &r) {
        if(r.IsEmpty()) return True;

        link *prev=0;
        link *l=areas;
        while(l) {
                if(r.Contains(l->r)) {
                        //delete l->r
                        if(prev) {
                                prev->next=l->next;
                                delete l;
                                l=prev->next;
                        } else {
                                areas=l->next;
                                delete l;
                                l=areas;
                        }
                } else if(r.Intersects(l->r)) {
                        //split and insert
                        FRect ra[5];
                        int s=Split(l->r,r,ra);
                        if(s==1) {
                                //no split???
                                prev=l;
                                l=l->next;
                        } else {
                                //insert before current
                                for(int i=0; i<s; i++) {
                                        if(!ra[i].IsEmpty() && !r.Contains(ra[i])) {
                                                link *n=new link;
                                                if(!l) return False;
                                                n->r=ra[i];
                                                n->next=l;
                                                if(prev) {
                                                        prev->next=n;
                                                } else
                                                        areas=n;
                                                prev=n;
                                        }
                                }
                                //remove original rect
                                if(prev)
                                        prev->next=l->next;
                                else
                                        areas=l->next;
                                delete l;
                                l=prev->next;
                        }
                } else {
                        //advance
                        prev=l;
                        l=l->next;
                }
        }
        return True;
}

Bool Region::Exclude(const Region &r) {
        if(&r==this) {
                Clear();
                return True;
        }

        for(link *l=r.areas; l; l=l->next)
                if(!Exclude(l->r))
                        return False;

        return True;
}

void Region::Clear() {
        link *l=areas;
        while(l) {
                link *n=l->next;
                delete l;
                l=n;
        }
        areas=0;
}

Bool Region::Intersect(const Region &r) {
        if(&r==this) return True;

        Region tmp;
        if(!tmp.Copy(*this)) return False;
        if(!tmp.Exclude(r)) return False;
        return this->Exclude(tmp);
}

void Region::Clip(const FRect &r) {
        link *prev=0;
        link *l=areas;
        while(l) {
                if(l->r.xLeft<r.xLeft)
                        l->r.xLeft=r.xLeft;
                if(l->r.xRight>r.xRight)
                        l->r.xRight=r.xRight;
                if(l->r.yTop<r.yTop)
                        l->r.yTop=r.yTop;
                if(l->r.yBottom>r.yBottom)
                        l->r.yBottom=r.yBottom;
                if(l->r.IsEmpty()) {
                        //remove
                        if(prev) {
                                prev->next=l->next;
                                delete l;
                                l=prev->next;
                        } else {
                                areas=l->next;
                                delete l;
                                l=areas;
                        }
                } else {
                        prev=l;
                        l=l->next;
                }
        }
}


int Region::IsEmpty() const {
        if(areas)
                return False;
        else
                return True;
}

FRect Region::QueryBoundsRect() const {
        if(areas) {
                FRect r=areas->r;
                link *l=areas->next;
                for(; l; l=l->next) {
                        if(l->r.xLeft<r.xLeft)
                                r.xLeft=l->r.xLeft;
                        if(l->r.yTop<r.yTop)
                                r.yTop=l->r.yTop;
                        if(l->r.xRight>r.xRight)
                                r.xRight=l->r.xRight;
                        if(l->r.yBottom>r.yBottom)
                                r.yBottom=l->r.yBottom;
                }
                return r;
        } else {
                return FRect(0,0,0,0);
        }
}

Bool Region::Includes(const FPoint &p) {
        for(link *l=areas; l; l=l->next)
                if(l->r.Contains(p))
                        return True;
        return False;
}


int Region::QueryRectCount() const {
        int i=0;
        for(link *l=areas; l; l=l->next)
                i++;
        return i;
}

int Region::QueryRects(FRect *r, int maxelem) const {
        int i=0;
        for(link *l=areas; l && i<maxelem; l=l->next)
                r[i++]=l->r;
        return i;
}


void Region::Optimize() {
        Bool changed;
        Bool Again;
do {
        Again=False;
        do {
                changed=False;
                for(link *l=areas; l; l=l->next) {
                        link *o=l->next;
                        link *prevo=l;
                        while(o) {
                                Bool d=False;
                                if(l->r.yTop==o->r.yTop &&
                                   l->r.yBottom==o->r.yBottom)
                                {
                                        if(l->r.xRight==o->r.xLeft) {
                                                l->r.xRight=o->r.xRight;
                                                d=True;
                                        } else if(l->r.xLeft==o->r.xRight) {
                                                l->r.xLeft=o->r.xLeft;
                                                d=True;
                                        }
                                }

                                if(d) {
                                        prevo->next=o->next;
                                        delete o;
                                        o=prevo->next;
                                        changed=True;
                                        Again=True;
                                } else {
                                        prevo=o;
                                        o=o->next;
                                }
                        }
                }
        } while(changed);
        do {
                changed=False;
                for(link *l=areas; l; l=l->next) {
                        link *o=l->next;
                        link *prevo=l;
                        while(o) {
                                Bool d=False;
                                if(l->r.xLeft==o->r.xLeft &&
                                   l->r.xRight==o->r.xRight)
                                {
                                        if(l->r.yBottom==o->r.yTop) {
                                                l->r.yBottom=o->r.yBottom;
                                                d=True;
                                        } else if(l->r.yTop==o->r.yBottom) {
                                                l->r.yTop=o->r.yTop;
                                                d=True;
                                        }
                                }

                                if(d) {
                                        prevo->next=o->next;
                                        delete o;
                                        o=prevo->next;
                                        changed=True;
                                        Again=True;
                                } else {
                                        prevo=o;
                                        o=o->next;
                                }
                        }
                }
        } while(changed);
} while(Again);
}


void Region::Move(int dx, int dy) {
        FPoint p(dx,dy);
        for(link *l=areas; l; l=l->next)
                l->r.MoveRelative(p);
}
