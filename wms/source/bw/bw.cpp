#define WMSINCL_ANCHORBLOCK
#define WMSINCL_BW
#define WMSINCL_REGION
#include <wms.hpp>

#include "hwndmap.hpp"
#include "paintq.hpp"
#include "bw_int.hpp"
#include "../wmsmutex.hpp"
#include "../anchor/anchor.hpp"

struct Window {
        WmsHWND hwnd;
        Bool    isBeingDestroyed;       //to prevent recursive hooks
        unsigned long tid;
        WmsHMQ  hmq;

        Window  *ownerWindow;
        Window  *nextOwnee,
                *prevOwnee;
        Window  *firstOwnee;

        Window  *parentWindow;
        Window  *nextSibling,
                *prevSibling;
        Window  *firstChild;

        uint32  style;
        uint16  id;

        FRect   vrect;                  //in parent coordinates
        FRect   hrect;                  //while hidden

        Region  ir;                     //invalid region

        uint32  userData;
};

static WmsMutex WndMutex;                 //mutex to lock this module
class WndLock {
public:
        WndLock() { WndMutex.Request(); }
        ~WndLock() { WndMutex.Release(); }
};

static WmsHWND hwndSequencer;           //WmsHWND generator
static HwndMap hwndMap;                 //WmsHWND->Window* mapper

static WmsMutex hookModify;

static PaintQueue paintQueue;

//owner/parent relations-------------------------------------------------------
WmsHWND WmsQueryWindowOwner(WmsHWND hwnd) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(wnd && wnd->ownerWindow)
                return wnd->ownerWindow->hwnd;
        else
                return 0;
}

Bool WmsSetWindowOwner(WmsHWND hwnd, WmsHWND hwndNewOwner) {
        WndLock wl;
        if(!WmsIsWindow(hwnd)) return False;
        if(hwndNewOwner) {
                if(!WmsIsWindow(hwndNewOwner)) return False;
                if(WmsIsOwnee(hwnd,hwndNewOwner)) return False;      //cycle
                if(hwnd==hwndNewOwner) return False;                 //cycle
        }

        Window *wnd=hwndMap.Find(hwnd);
        Window *newOwner=hwndMap.Find(hwndNewOwner);
        if(wnd->isBeingDestroyed)
                return False;
        if(newOwner && newOwner->isBeingDestroyed)
                return False;

        if(wnd->ownerWindow) {
                //unlink
                if(wnd->ownerWindow->firstOwnee==wnd)
                        wnd->ownerWindow->firstOwnee=wnd->nextOwnee;
                if(wnd->nextOwnee)
                        wnd->nextOwnee->prevOwnee=wnd->prevOwnee;
                if(wnd->prevOwnee)
                        wnd->prevOwnee->nextOwnee=wnd->nextOwnee;
        }

        wnd->ownerWindow=newOwner;
        if(newOwner) {
                //link in
                wnd->nextOwnee=newOwner->firstOwnee;
                newOwner->firstOwnee=wnd;
                if(wnd->nextOwnee)
                        wnd->nextOwnee->prevOwnee=wnd;
                wnd->prevOwnee=0;
        }

        return True;
}

WmsHWND WmsQueryWindowParent(WmsHWND hwnd) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(wnd && wnd->parentWindow)
                return wnd->parentWindow->hwnd;
        else
                return 0;
}

Bool WmsSetWindowParent(WmsHWND hwnd, WmsHWND hwndNewParent) {
        WndLock wl;

        if(!WmsIsWindow(hwnd)) return False;
        if(hwndNewParent) {
                if(!WmsIsWindow(hwndNewParent)) return False;
                if(WmsIsChild(hwnd,hwndNewParent)) return False;     //cycle
                if(hwnd==hwndNewParent) return False;                //cycle
        }

        Window *wnd=hwndMap.Find(hwnd);
        Window *newParent=hwndMap.Find(hwndNewParent);
        if(wnd->isBeingDestroyed)
                return False;
        if(newParent && newParent->isBeingDestroyed)
                return False;

        if(wnd->parentWindow) {
                //unlink
                if(wnd->parentWindow->firstChild==wnd)
                        wnd->parentWindow->firstChild=wnd->nextSibling;
                if(wnd->nextSibling)
                        wnd->nextSibling->prevSibling=wnd->prevSibling;
                if(wnd->prevSibling)
                        wnd->prevSibling->nextSibling=wnd->nextSibling;

                //invalidate previous area
                WmsInvalidateRect(wnd->parentWindow->hwnd, &wnd->vrect, False);
        }

        if(newParent) {
                //link
                wnd->prevSibling=0;
                wnd->nextSibling=newParent->firstChild;
                if(wnd->nextSibling)
                        wnd->nextSibling->prevSibling=wnd;
                newParent->firstChild=wnd;
                wnd->parentWindow=newParent;
                //invalidate
                wnd->ir.Clear();
                WmsInvalidateRect(hwnd,0,True);
        } else {
                wnd->parentWindow=0;
                wnd->prevSibling=wnd->nextSibling=0;
        }

        return True;
}

Bool WmsIsChild(WmsHWND hwndParent, WmsHWND hwndChild) {
        WndLock wl;

        Window *parent=hwndMap.Find(hwndParent);
        Window *child=hwndMap.Find(hwndChild);
        if(!parent || !child) return False;

        do {
                child=child->parentWindow;
                if(child==parent) return True;
        } while(child);
        return False;
}

Bool WmsIsOwnee(WmsHWND hwndOwner, WmsHWND hwndOwnee) {
        WndLock wl;

        Window *owner=hwndMap.Find(hwndOwner);
        Window *ownee=hwndMap.Find(hwndOwnee);
        if(!owner || !ownee) return False;

        do {
                ownee=ownee->ownerWindow;
                if(ownee==owner) return True;
        } while(ownee);
        return False;
}


struct ChildEnum {
        WmsHWND *a;
        int wnds;
        int current;
};

ChildEnum *WmsBeginEnum(WmsHWND hwndParent) {
        WndLock wl;
        Window *parent=hwndMap.Find(hwndParent);
        if(!parent)
                return False;

        //count children
        int c=0;
        for(Window *child=parent->firstChild; child; child=child->nextSibling)
                c++;

        WmsHWND *a=new WmsHWND[c];
        if(!a) return 0;
        ChildEnum *ce=new ChildEnum;
        if(!ce) {
                delete[] a;
                return 0;
        }

        c=0;
        for(child=parent->firstChild; child; child=child->nextSibling)
                a[c++]=child->hwnd;

        ce->a=a;
        ce->wnds=c;
        ce->current=0;
        return ce;
}

WmsHWND WmsQueryNextWindow(ChildEnum *ce) {
        if(ce->current>=ce->wnds)
                return 0;
        return ce->a[ce->current++];
}

void WmsEndEnum(ChildEnum *ce) {
        delete[] ce->a;
        delete ce;
}


WmsHWND WmsQueryWindow(WmsHWND hwnd, int fl) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd) return 0;
        Window *w=0;
        switch(fl) {
                case QW_PARENT:      w=wnd->parentWindow; break;
                case QW_OWNER:       w=wnd->ownerWindow; break;
                case QW_FIRSTSIBLING:w=wnd; while(w->prevSibling) w=w->prevSibling; break;
                case QW_PREVSIBLING: w=wnd->prevSibling; break;
                case QW_NEXTSIBLING: w=wnd->nextSibling; break;
                case QW_LASTSIBLING: w=wnd; while(w->nextSibling) w=w->nextSibling; break;
                case QW_FIRSTCHILD:  w=wnd->firstChild; break;
                case QW_PREVOWNEE:   w=wnd->prevOwnee; break;
                case QW_NEXTOWNEE:   w=wnd->nextOwnee; break;
                case QW_FIRSTOWNEE:  w=wnd->firstOwnee; break;
        }
        if(!w)
                return 0;
        else
                return w->hwnd;
}


//style----------------------------------------------------------------------
static volatile SetStyleHook_f pfn_setstyle=0;

WmsHWND WmsSearchStyleBit(WmsHWND hwnd, uint32 b, uint32 v) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        while(wnd) {
                if((wnd->style&b)==v)
                        return wnd->hwnd;
                wnd=wnd->parentWindow;
        }
        return 0;
}


uint32 WmsQueryWindowStyle(WmsHWND hwnd) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd) return 0;

        return wnd->style;
}

Bool WmsSetWindowStyle(WmsHWND hwnd, uint32 s) {
        uint32 oldstyle,newstyle;
        {
                WndLock wl;
                Window *wnd=hwndMap.Find(hwnd);
                if(!wnd) return False;
                if(wnd->isBeingDestroyed) return False;

                oldstyle=wnd->style;
                newstyle=s;
                if(oldstyle==newstyle) return True;

                wnd->style = newstyle;
        }

        //call hook
        SetStyleHook_f pfn=pfn_setstyle;
        if(pfn)
                (*pfn)(hwnd,oldstyle,newstyle);

        return True;
}

Bool WmsSetWindowStyleBits(WmsHWND hwnd, uint32 bits, uint32 mask) {
        uint32 oldstyle,newstyle;
        {
                WndLock wl;
                Window *wnd=hwndMap.Find(hwnd);
                if(!wnd) return False;
                if(wnd->isBeingDestroyed) return False;

                oldstyle = wnd->style;
                newstyle = (oldstyle&~mask)|(bits&mask);
                if(oldstyle==newstyle) return True;

                wnd->style = newstyle;
        }

        //call hook
        SetStyleHook_f pfn=pfn_setstyle;
        if(pfn)
                (*pfn)(hwnd,oldstyle,newstyle);

        return True;
}

SetStyleHook_f WmsSetSetStyleHook(SetStyleHook_f pfn) {
        hookModify.Request();
        SetStyleHook_f oldpfn=pfn_setstyle;
        pfn_setstyle=pfn;
        hookModify.Release();
        return oldpfn;
}


//ID---------------------------------------------------------------------------
uint16 WmsQueryWindowID(WmsHWND hwnd) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd) return 0;

        return wnd->id;
}

Bool WmsSetWindowID(WmsHWND hwnd, uint16 ID) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd) return False;
        if(wnd->isBeingDestroyed) return False;

        wnd->id = ID;
        return True;
}

WmsHWND WmsWindowFromID(WmsHWND hwndParent, uint16 ID) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwndParent);
        if(!wnd) return 0;

        for(Window *child=wnd->firstChild; child; child=child->nextSibling) {
                if(child->id==ID)
                        return child->hwnd;
        }
        return 0;
}


//user data--------------------------------------------------------------------
uint32 WmsQueryWindowUserData(WmsHWND hwnd) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd) return 0;

        return wnd->userData;
}

Bool WmsSetWindowUserData(WmsHWND hwnd, uint32 ud) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd) return False;
        if(wnd->isBeingDestroyed) return False;

        wnd->userData = ud;
        return True;
}

//HMQ -------------------------------------------------------------------------
WmsHMQ WmsQueryWindowMsgQueue(WmsHWND hwnd) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(wnd)
                return wnd->hmq;
        else
                return 0;
}

//visibility, position & zorder------------------------------------------------

static volatile SWPHook_f pfn_swp=0;
SWPHook_f WmsSetSWPHook(SWPHook_f pfn) {
        hookModify.Request();
        SWPHook_f oldpfn=pfn_swp;
        pfn_swp=pfn;
        hookModify.Release();
        return oldpfn;
}

static Bool validateSWP(WmsSWP *swp) {
        //validate a SWP structure
        Window *wnd=hwndMap.Find(swp->hwnd);
        Window *behind=hwndMap.Find(swp->hwndBehind);

        //parameter check:
        if(swp->flags==0)
                return True;
        if(!wnd)
                return False;
        if(wnd->isBeingDestroyed)
                return False;
        if((swp->flags&SWP_ZORDER)) {
                if(!wnd->parentWindow)
                        return False;   //can't zorder orphan window
                if(swp->hwndBehind!=HWND_TOP && swp->hwndBehind!=HWND_BOTTOM) {
                        if(!behind)
                                return False;
                        if(behind->parentWindow!=wnd->parentWindow)
                                return False;   //can't zorder to non-sibling
                }
        }
        if((swp->flags&SWP_SIZE) && (swp->cx<0 || swp->cy<0))
                return False;           //negative width/height
        if(swp->flags&~(SWP_MOVE|SWP_SIZE|SWP_HIDE|SWP_SHOW|SWP_ZORDER))
                return False;           //illegal bits
        if((swp->flags&SWP_HIDE) && (swp->flags&SWP_SHOW))
                return False;           //cannot both hide and show
        return True;
}

static void optimizeSWP(WmsSWP *swp) {
        //Optimize the SWP flags by removing unnecesarry flags
        Window *wnd=hwndMap.Find(swp->hwnd);
        Window *behind=hwndMap.Find(swp->hwndBehind);
        if(swp->flags&SWP_MOVE) {
                if(wnd->hrect.GetLeft()==swp->x &&
                   wnd->hrect.GetTop()==swp->y)
                        swp->flags &= ~SWP_MOVE;
        }
        if(swp->flags&SWP_SIZE) {
                if(wnd->hrect.GetWidth()==swp->cx &&
                   wnd->hrect.GetHeight()==swp->cy)
                        swp->flags &= ~SWP_SIZE;
        }
        if(swp->flags&SWP_HIDE) {
                if(!(wnd->style&WS_VISIBLE))
                        swp->flags &= ~SWP_HIDE;
        }
        if(swp->flags&SWP_SHOW) {
                if(wnd->style&WS_VISIBLE)
                        swp->flags &= ~SWP_SHOW;
        }
        if(swp->flags&SWP_ZORDER) {
                if(swp->hwndBehind==HWND_TOP) {
                        if(wnd->prevSibling==0)
                                swp->flags &= ~SWP_ZORDER;
                } else if(swp->hwndBehind==HWND_BOTTOM) {
                        if(wnd->nextSibling==0)
                                swp->flags &= ~SWP_ZORDER;
                } else {
                        if(behind==wnd->prevSibling)
                                swp->flags &= ~SWP_ZORDER;
                        else if(behind->nextSibling==0)
                                swp->hwndBehind=HWND_BOTTOM;
                }
        }
}

Bool WmsSetWindowPos(WmsHWND hwnd,
                     WmsHWND hwndBehind,
                     int x, int y,
                     int cx, int cy,
                     int flags)
{
        WmsSWP swp;
        swp.flags = flags;
        swp.cx = cx;
        swp.cy = cy;
        swp.x = x;
        swp.y = y;
        swp.hwndBehind = hwndBehind;
        swp.hwnd = hwnd;
        return WmsSetMultWindowPos(&swp,1);
}

Bool WmsSetMultWindowPos(WmsSWP *pswp, unsigned swps) {
        if(swps==0) return True;
        if(pswp==0) return False;

        //todo: prevent multiple threads swp'ing the same window at the same time
        //todo: release WndLock while style hook is called
        //todo: optimize when multiple windows are SWP'd
        {
                WndLock wl;
                int anyFlagsNonZero=0;
                for(unsigned i=0; i<swps; i++) {
                        if(!validateSWP(pswp+i))
                                return False;
                        optimizeSWP(pswp+i);
                        if(pswp[i].flags!=0) anyFlagsNonZero=1;
                }
                if(!anyFlagsNonZero)
                        return True;    //nothing to do
        }

        SWPHook_f pfn=pfn_swp;
        if(pfn) {
                for(unsigned i=0; i<swps; i++) {
                        pswp[i].flags |= SWP_CHANGING;
                        (*pfn)(pswp+i);
                        pswp[i].flags &= ~SWP_CHANGING;
                        if(!validateSWP(pswp+i))
                                return False;
                }
        }

        for(unsigned i=0; i<swps; i++) {
                WndLock wl;
                optimizeSWP(pswp+i);

                if(pswp[i].flags==0)
                        continue;    //nothing to do

                //now do it
                Window *wnd=hwndMap.Find(pswp[i].hwnd);
                Window *behind=hwndMap.Find(pswp[i].hwndBehind);
                if(pswp[i].flags&SWP_MOVE) {
                        FRect oldRect=wnd->vrect;
                        FRect newRect;
                        newRect.SetLeft(pswp[i].x);
                        newRect.SetTop(pswp[i].y);
                        newRect.SetWidth(wnd->hrect.GetWidth());
                        newRect.SetHeight(wnd->hrect.GetHeight());

                        wnd->vrect=wnd->hrect=newRect;
                        if((wnd->style&WS_VISIBLE)==0) {
                                wnd->vrect.SetWidth(0);
                                wnd->vrect.SetHeight(0);
                        }
                        if(wnd->parentWindow)
                                WmsInvalidateRect(wnd->parentWindow->hwnd,&oldRect,True);
                        //WmsValidateRect(wnd->parentWindow->hwnd,&wnd->vrect,True);
                        WmsInvalidateRect(pswp[i].hwnd,0,True);
                }
                if(pswp[i].flags&SWP_SIZE) {
                        FRect oldRect=wnd->vrect;
                        wnd->hrect.SetWidth(pswp[i].cx);
                        wnd->hrect.SetHeight(pswp[i].cy);

                        if(wnd->style&WS_VISIBLE) {
                                wnd->vrect.SetWidth(pswp[i].cx);
                                wnd->vrect.SetHeight(pswp[i].cy);
                        }
                        if(wnd->parentWindow) {
                                WmsInvalidateRect(wnd->parentWindow->hwnd,&oldRect,True);
                                //WmsValidateRect(wnd->parentWindow->hwnd,&wnd->vrect,True);
                        }
                        WmsInvalidateRect(pswp[i].hwnd,0,True);
                }
                if(pswp[i].flags&SWP_SHOW) {
                        WmsSetWindowStyleBits(pswp[i].hwnd,WS_VISIBLE,WS_VISIBLE);
                        wnd->vrect=wnd->hrect;
                        WmsInvalidateRect(pswp[i].hwnd,0,True);
                }
                if(pswp[i].flags&SWP_HIDE) {
                        FRect oldRect=wnd->vrect;
                        WmsSetWindowStyleBits(pswp[i].hwnd,0,WS_VISIBLE);
                        if(wnd->parentWindow)
                                WmsInvalidateRect(wnd->parentWindow->hwnd,&oldRect,True);
                        wnd->ir.Clear();
                        paintQueue.Remove(wnd->hwnd);
                        wnd->vrect.SetWidth(0);
                        wnd->vrect.SetHeight(0);
                }
                if(pswp[i].flags&SWP_ZORDER) {
                        if(pswp[i].hwndBehind==HWND_TOP) {
                                wnd->prevSibling->nextSibling = wnd->nextSibling;
                                if(wnd->nextSibling) wnd->nextSibling->prevSibling = wnd->prevSibling;
                                wnd->prevSibling=0;
                                wnd->nextSibling=wnd->parentWindow->firstChild;
                                wnd->parentWindow->firstChild=wnd;
                                wnd->nextSibling->prevSibling=wnd;
                        } else if(pswp[i].hwndBehind==HWND_BOTTOM) {
                                if(wnd==wnd->parentWindow->firstChild)
                                        wnd->parentWindow->firstChild=wnd->nextSibling;
                                else
                                        wnd->prevSibling->nextSibling = wnd->nextSibling;
                                wnd->nextSibling->prevSibling = wnd->prevSibling;
                                for(Window *last=wnd->nextSibling;
                                    last->nextSibling;
                                    last=last->nextSibling
                                   )
                                        ;
                                wnd->nextSibling=0;
                                wnd->prevSibling=last;
                                last->nextSibling=wnd;
                        } else {
                                wnd->prevSibling->nextSibling = wnd->nextSibling;
                                wnd->nextSibling->prevSibling = wnd->prevSibling;
                                wnd->prevSibling = behind;
                                wnd->nextSibling = behind->nextSibling;
                                behind->nextSibling = wnd;
                                if(wnd->nextSibling)
                                        wnd->nextSibling->prevSibling=wnd;
                        }
                        //the easy way:
                        WmsInvalidateRect(pswp[i].hwnd,0,True);
                        for(Window *inFront=wnd->prevSibling;
                            inFront;
                            inFront=inFront->prevSibling) {
                                FRect r=wnd->vrect;
                                WmsMapWindowRects(pswp[i].hwnd,inFront->hwnd,&r,1);
                                WmsInvalidateRect(inFront->hwnd,&r,True);
                        }
                }
        }

        //call hook
        pfn=pfn_swp;
        if(pfn) {
                for(unsigned i=0; i<swps; i++)
                        (*pfn)(pswp+i);
        }
        return True;
}


Bool WmsQueryWindowRect(WmsHWND hwnd, FRect *prect) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd || prect==0)
                return False;
        //This is different from OS/2 PM, because we return the window's
        //logical rect not its visible rect
        prect->Set(0,0,wnd->hrect.GetWidth(),wnd->hrect.GetHeight());
        return True;
}


static Bool computeWindowOffset(const Window *from, const Window *to, FPoint *p) {
        p->Set(0,0);
        for(; from->parentWindow; from=from->parentWindow) {
                p->x += from->hrect.GetLeft();
                p->y += from->hrect.GetTop();
        }
        for(; to->parentWindow; to=to->parentWindow) {
                p->x -= to->hrect.GetLeft();
                p->y -= to->hrect.GetTop();
        }
        if(from!=to) {
                //no common parent
                return False;
        } else
                return True;
}

Bool WmsMapWindowPoints(WmsHWND hwndFrom, WmsHWND hwndTo, FPoint *p, int points) {
        WndLock wl;
        Window *from=hwndMap.Find(hwndFrom);
        Window *to=hwndMap.Find(hwndTo);
        if(!from || !to)
                return False;

        FPoint offset;
        if(computeWindowOffset(from,to,&offset)) {
                for(int i=0; i<points; i++) {
                        p[i].x += offset.GetX();
                        p[i].y += offset.GetY();
                }
                return True;
        } else {
                return False;
        }
}

Bool WmsMapWindowRects(WmsHWND hwndFrom, WmsHWND hwndTo, FRect *r, int rects) {
        WndLock wl;
        Window *from=hwndMap.Find(hwndFrom);
        Window *to=hwndMap.Find(hwndTo);
        if(!from || !to)
                return False;

        FPoint offset;
        if(computeWindowOffset(from,to,&offset)) {
                for(int i=0; i<rects; i++) {
                        r[i].xLeft += offset.GetX();
                        r[i].yTop += offset.GetY();
                        r[i].xRight += offset.GetX();
                        r[i].yBottom += offset.GetY();
                }
                return True;
        } else {
                return False;
        }
}

WmsHWND WmsWindowFromPoint(WmsHWND hwndTop, const FPoint *pp, Bool fTestChildren, WmsHWND hwndBelow) {
        WndLock wl;
        Window *topWindow=hwndMap.Find(hwndTop);
        if(!topWindow)
                return False;

        FPoint p=*pp;
        Window *wnd=topWindow;
        if(!wnd->vrect.Contains(p))
                return 0;

        if(!fTestChildren)
                return wnd->hwnd;

        Window *child;
l:
        if(hwndBelow==HWND_TOP)
                child=wnd->firstChild;
        else {
                child=hwndMap.Find(hwndBelow);
                if(child==0 || child->parentWindow!=wnd)
                        return 0;
        }
        Window *saveChild=child;
        for(; child; child=child->nextSibling) {
                if((child->style&WS_CLIPSIBLINGS)==0 && child->vrect.Contains(p)) {
                        wnd=child;
                        p.x -= wnd->vrect.xLeft;
                        p.y -= wnd->vrect.yTop;
                        hwndBelow=HWND_TOP;
                        goto l;
                }
        }
        child=saveChild;
        for(; child; child=child->nextSibling) {
                if((child->style&WS_CLIPSIBLINGS)!=0 && child->vrect.Contains(p)) {
                        wnd=child;
                        p.x -= wnd->vrect.xLeft;
                        p.y -= wnd->vrect.yTop;
                        hwndBelow=HWND_TOP;
                        goto l;
                }
        }
        return wnd->hwnd;
}



//valid region---------------------------------------------------------------

Bool WmsInvalidateRect(WmsHWND hwnd, const FRect *rect, Bool includeChildren) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd)
                return False;
        if(wnd->isBeingDestroyed)
                return False;

        if(!WmsIsWindowVisible(hwnd))
                return True;    //optimize

        FRect r;
        if(rect==0) {
                r.Set(0,0,wnd->vrect.GetWidth(),wnd->vrect.GetHeight());
                wnd->ir.Clear();
        } else {
                r=*rect;
        }
        wnd->ir.Include(r);


        if(includeChildren) {
                //invalidate children
                for(Window *child=wnd->firstChild;
                    child;
                    child=child->nextSibling
                   )
                {
                        FRect cr=r;
                        cr.MoveRelative(FPoint(-child->vrect.xLeft,-child->vrect.yTop));
                        WmsInvalidateRect(child->hwnd,&cr,True);
                }
        }

        paintQueue.Insert(wnd->hwnd);

        return True;
}

Bool WmsValidateRect(WmsHWND hwnd, const FRect *rect, Bool includeChildren) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd)
                return False;
        if(wnd->isBeingDestroyed)
                return False;

        if(!WmsIsWindowVisible(hwnd))
                return True;    //optimize

        FRect r;
        if(rect==0) {
                r.Set(0,0,wnd->vrect.GetWidth(),wnd->vrect.GetHeight());
                wnd->ir.Clear();
                paintQueue.Remove(hwnd);
        } else {
                r=*rect;
                wnd->ir.Exclude(r);
                if(wnd->ir.IsEmpty())
                        paintQueue.Remove(hwnd);
        }

        if(includeChildren) {
                //validate children
                for(Window *child=wnd->firstChild;
                    child;
                    child=child->nextSibling
                   )
                {
                        FRect cr=r;
                        cr.MoveRelative(FPoint(-child->vrect.xLeft,-child->vrect.yTop));
                        WmsValidateRect(child->hwnd,&cr,True);
                }
        }
        return True;
}


Bool WmsEnableWindowUpdate(WmsHWND hwnd, Bool visible) {
        if(!WmsIsWindow(hwnd))
                return False;

        //Change the window's WS_VISIBLE style without invalidation
        if(visible)
                WmsSetWindowStyleBits(hwnd,WS_VISIBLE,WS_VISIBLE);
        else
                WmsSetWindowStyleBits(hwnd,0,WS_VISIBLE);

        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd)
                return False;
        if(wnd->isBeingDestroyed)
                return False;
        wnd->ir.Clear();
        paintQueue.Remove(wnd->hwnd);

        return True;
}

Bool WmsQueryWindowUpdateRegion(WmsHWND hwnd, Region *rgn) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd)
                return False;
        if(wnd->isBeingDestroyed)
                rgn->Clear();
        else
                rgn->Copy(wnd->ir);

        return True;
}

Bool WmsQueryWindowUpdateRect(WmsHWND hwnd, FRect *r) {
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd)
                return False;
        if(wnd->isBeingDestroyed)
                r->Set(0,0,0,0);
        else
                *r=wnd->ir.QueryBoundsRect();

        return True;
}


Bool WmsQueryWindowVisibleRegion(WmsHWND hwnd, Region *rgn) {
        return WmsQuerySpecialRegion(hwnd,QSR_NOCLIPCHILDREN,rgn);
}

Bool WmsQueryWindowClipRegion(WmsHWND hwnd, Region *rgn) {
        return WmsQuerySpecialRegion(hwnd,0,rgn);
}

Bool WmsQuerySpecialRegion(WmsHWND hwnd, uint32 fl, Region *rgn) {
        if(fl&QSR_CLIPCHILDREN && fl&QSR_NOCLIPCHILDREN)
                return False;
        if(fl&QSR_CLIPSIBLINGSABOVE && fl&QSR_NOCLIPSIBLINGSABOVE)
                return False;
        if(fl&QSR_CLIPSIBLINGSBELOW && fl&QSR_NOCLIPSIBLINGSBELOW)
                return False;
        WndLock wl;
        Window *wnd=hwndMap.Find(hwnd);
        if(!wnd)
                return False;
        if(fl&QSR_PARENTCLIP && !wnd->parentWindow)
                return False;

        rgn->Clear();
        if(wnd->isBeingDestroyed)
                return True;
        if(!WmsIsWindowVisible(hwnd))
                return True;

        if(wnd->parentWindow) {
                WmsQuerySpecialRegion(wnd->parentWindow->hwnd,QSR_NOCLIPCHILDREN,rgn);
                rgn->Move(-wnd->vrect.xLeft,-wnd->vrect.yTop);
                if((fl&QSR_PARENTCLIP)==0)
                        rgn->Clip(FRect(0,0,wnd->vrect.GetWidth(),wnd->vrect.GetHeight()));
        } else
                if(!rgn->Include(wnd->vrect)) return False;

        if(fl&QSR_CLIPSIBLINGSABOVE) {
                for(Window *s=wnd->prevSibling; s; s=s->prevSibling) {
                        FRect sr=s->vrect;
                        sr.MoveAbsolute(sr.xLeft-wnd->vrect.xLeft,
                                        sr.yTop-wnd->vrect.yTop);
                        if(!rgn->Exclude(sr)) return False;
                }
        } else if(fl&QSR_NOCLIPSIBLINGSABOVE) {
                //nothing
        } else {
                //std clipping
                for(Window *s=wnd->prevSibling; s; s=s->prevSibling) {
                        if(s->style&WS_CLIPSIBLINGS) continue;
                        FRect sr=s->vrect;
                        sr.MoveAbsolute(sr.xLeft-wnd->vrect.xLeft,
                                        sr.yTop-wnd->vrect.yTop);
                        if(!rgn->Exclude(sr)) return False;
                }
        }


        if(fl&QSR_CLIPSIBLINGSBELOW) {
                for(Window *s=wnd->nextSibling; s; s=s->nextSibling) {
                        FRect sr=s->vrect;
                        sr.MoveAbsolute(sr.xLeft-wnd->vrect.xLeft,
                                        sr.yTop-wnd->vrect.yTop);
                        if(!rgn->Exclude(sr)) return False;
                }
        } else if(fl&QSR_NOCLIPSIBLINGSBELOW) {
                //nothing
        } else {
                //std clipping
                if(wnd->style&WS_CLIPSIBLINGS) {
                        for(Window *s=wnd->nextSibling; s; s=s->nextSibling) {
                                FRect sr=s->vrect;
                                sr.MoveAbsolute(sr.xLeft-wnd->vrect.xLeft,
                                                sr.yTop-wnd->vrect.yTop);
                                if(!rgn->Exclude(sr)) return False;
                        }
                }
        }

        //exclude children
        if(fl&QSR_CLIPCHILDREN || ((fl&QSR_NOCLIPCHILDREN)==0 && wnd->style&WS_CLIPCHILDREN))
        {
                for(Window *child=wnd->firstChild; child; child=child->nextSibling)
                        if(!rgn->Exclude(child->vrect)) return False;
        }

        return True;
}


WmsHWND WmsQueryInvalidWindow() {
        for(;;) {
                WmsHWND hwnd=paintQueue.Get();
                if(hwnd==0) break;
                WndLock wl;
                Window *wnd=hwndMap.Find(hwnd);
                if(wnd==0) continue;
                if(wnd->ir.IsEmpty()) continue;

/*                //if any of its ancestors are invalid draw them first
                for(Window *p=wnd->parentWindow; p; p=p->parentWindow)
                        if(!p->ir.IsEmpty())
                                wnd=p;
                //if any siblings above is invalid draw them first
                for(Window *s=wnd->prevSibling; s; s=s->prevSibling)
                        if(!s->ir.IsEmpty())
                                wnd=s;
                if(hwnd!=wnd->hwnd) {
                        paintQueue.Remove(wnd->hwnd);
                        paintQueue.Insert(hwnd);
                }
*/                return wnd->hwnd;
        }
        return 0;
}


void WmsStopPaintQueue_internal(void) {
        paintQueue.PleaseTerminate();
}


//creation and destruction---------------------------------------------------

WmsHWND WmsCreateWindow(WmsHWND hwndParent,
                        int x, int y,
                        int cx, int cy,
                        WmsHWND hwndBehind,
                        WmsHWND hwndOwner,
                        uint32 style,
                        uint16 id,
                        uint32 userData
                       )
{
        unsigned tid=FThreadManager::GetCurrentThread()->GetTID();
        WmsHAB hab=WmsQueryThreadAnchorBlock(tid);
        if(!hab) return 0;
        WmsHMQ hmq=WmsQueryAnchorBlockMsgQueue(hab);
        if(!hmq) return 0;

        WndLock wl;
        Window *parent=hwndMap.Find(hwndParent);
        Window *behind=hwndMap.Find(hwndBehind);
        Window *owner=hwndMap.Find(hwndOwner);
        if(hwndParent && !parent)
                return 0;
        if(parent && parent->isBeingDestroyed)
                return 0;
        if(cx<0 || cy<0)
                return 0;           //negative width or height
        if(hwndBehind!=HWND_TOP &&
           hwndBehind!=HWND_BOTTOM) {
                if(!WmsIsWindow(hwndBehind))
                        return 0;
                if(behind->parentWindow!=parent)
                        return 0;
                if(behind->isBeingDestroyed)
                        return 0;
        }
        if(hwndOwner && !owner)
                return 0;
        if(owner && owner->isBeingDestroyed)
                return 0;


        Window *wnd = new Window;
        if(!wnd) return 0;


        //make owner links
        wnd->ownerWindow=owner;
        if(owner) {
                wnd->nextOwnee=owner->firstOwnee;
                if(wnd->nextOwnee)
                        wnd->nextOwnee->prevOwnee=wnd;
                wnd->prevOwnee=0;
                owner->firstOwnee=wnd;
        } else {
                wnd->nextOwnee=wnd->prevOwnee=0;
        }
        wnd->firstOwnee=0;

        //make parent links
        wnd->parentWindow=parent;
        if(parent) {
                if(hwndBehind==HWND_TOP || parent->firstChild==0) {
                        wnd->nextSibling=parent->firstChild;
                        if(wnd->nextSibling)
                                wnd->nextSibling->prevSibling=wnd;
                        wnd->prevSibling=0;
                        parent->firstChild=wnd;
                } else {
                        if(hwndBehind==HWND_BOTTOM) {
                                behind=parent->firstChild;
                                while(behind->nextSibling)
                                        behind=behind->nextSibling;
                        }
                        wnd->prevSibling=behind;
                        wnd->nextSibling=behind->nextSibling;
                        behind->nextSibling=wnd;
                        if(wnd->nextSibling)
                                wnd->nextSibling->prevSibling=wnd;
                }
        } else {
                wnd->nextSibling=wnd->prevSibling=0;
        }
        wnd->firstChild=0;


        wnd->tid=tid;
        wnd->hmq=hmq;

        wnd->style = style;

        wnd->id = id;

        if(wnd->style&WS_VISIBLE)
                wnd->vrect.Set(x,y,x+cx,y+cy);
        else
                wnd->vrect.Set(x,y,x,y);
        wnd->hrect.Set(x,y,x+cx,y+cy);
        wnd->userData = userData;

        wnd->isBeingDestroyed=False;

        //insert into map
        hwndSequencer = (WmsHWND)(((uint32)hwndSequencer)+1);
        wnd->hwnd=hwndSequencer;
        hwndMap.Insert(wnd->hwnd,wnd);

        WmsInvalidateRect(wnd->hwnd,0,False);

        return wnd->hwnd;
}


static volatile DestroyHook_f pfn_predestroy=0;
static volatile DestroyHook_f pfn_postdestroy=0;

Bool WmsDestroyWindow(WmsHWND hwnd) {
        unsigned long tid=FThreadManager::GetCurrentThread()->GetTID();
        Window *wnd;
        {
                WndLock wl;
                wnd=hwndMap.Find(hwnd);
                if(!wnd)
                        return False;           //not a window
                if(wnd->isBeingDestroyed)
                        return False;           //is being destroyed
                if(wnd->tid!=tid)
                        return False;           //only thread itself can destroy
                wnd->isBeingDestroyed=True;
        }

        //call pre-hook
        DestroyHook_f pfn=pfn_predestroy;
        if(pfn)
                pfn(hwnd);

        //destroy the children
        for(;;) {
                WndMutex.Request();
                Window *child=wnd->firstChild;
                if(child==0) {
                        //no more children
                        WndMutex.Release();
                        break;
                }
                if(child->tid!=tid) {
                        //child is owned by another thread
                        //just unlink
                        wnd->firstChild=child->nextSibling;
                        if(child->nextSibling)
                                child->nextSibling->prevSibling=0;
                        child->nextSibling=0;
                        child->parentWindow=0;
                        WndMutex.Release();
                } else {
                        //child is owned by this thread - destroy
                        WndMutex.Release();
                        WmsDestroyWindow(child->hwnd);
                }
        }


        //destroy the ownees
        for(;;) {
                WndMutex.Request();
                Window *ownee=wnd->firstOwnee;
                if(ownee==0) {
                        //no more ownees
                        WndMutex.Release();
                        break;
                }
                if(ownee->tid!=tid) {
                        //ownee is owned by another thread
                        //just unlink
                        wnd->firstOwnee=ownee->nextOwnee;
                        if(ownee->nextOwnee)
                                ownee->nextOwnee->prevOwnee=0;
                        ownee->nextOwnee=0;
                        ownee->ownerWindow=0;
                        WndMutex.Release();
                } else {
                        //ownee is owned by this thread - destroy
                        WndMutex.Release();
                        WmsDestroyWindow(ownee->hwnd);
                }
        }

        WndMutex.Request();

        //unlink from parent
        if(wnd->parentWindow) {
                if(wnd->parentWindow->firstChild==wnd)
                        wnd->parentWindow->firstChild= wnd->nextSibling;
                if(wnd->nextSibling)
                        wnd->nextSibling->prevSibling=wnd->prevSibling;
                if(wnd->prevSibling)
                        wnd->prevSibling->nextSibling=wnd->nextSibling;
                WmsInvalidateRect(wnd->parentWindow->hwnd,&wnd->vrect,True);
        }
        //unlink from owner
        if(wnd->ownerWindow) {
                if(wnd->ownerWindow->firstOwnee==wnd)
                        wnd->ownerWindow->firstOwnee = wnd->nextOwnee;
                if(wnd->nextOwnee)
                        wnd->nextOwnee->prevOwnee=wnd->prevOwnee;
                if(wnd->prevOwnee)
                        wnd->prevOwnee->nextOwnee=wnd->nextOwnee;
        }

        //remove from mapper
        hwndMap.Remove(hwnd);

        WndMutex.Release();

        //clear fields (not necessarry)
        wnd->ownerWindow=wnd->nextOwnee=wnd->prevOwnee=wnd->firstOwnee=0;
        wnd->parentWindow=wnd->nextSibling=wnd->prevSibling=wnd->firstChild=0;
        wnd->style=0;
        wnd->id=0;
        wnd->vrect.Set(0,0,0,0);
        wnd->hrect.Set(0,0,0,0);
        wnd->userData=0;

        delete wnd;

        pfn=pfn_postdestroy;
        if(pfn_postdestroy)
                pfn_postdestroy(hwnd);

        return True;
}

Bool WmsIsWindow(WmsHWND hwnd) {
        WndLock wl;
        //If the window is in the hwndmap, it's a window
        if(hwndMap.Find(hwnd)!=0)
                return True;
        else
                return False;
}


//destroy hook---------------------------------------------------------------

DestroyHook_f WmsSetPreDestroyHook(DestroyHook_f pfn) {
        hookModify.Request();
        DestroyHook_f old=pfn_predestroy;
        pfn_predestroy=pfn;
        hookModify.Release();
        return old;
}

DestroyHook_f WmsSetPostDestroyHook(DestroyHook_f pfn) {
        hookModify.Request();
        DestroyHook_f old=pfn_postdestroy;
        pfn_postdestroy=pfn;
        hookModify.Release();
        return old;
}
