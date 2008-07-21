/***                   
Filename: menuwin.cpp
Description:
        Implementation of FMenuWindow - the low-level menu window.
        Inspired by PM but the mecanisms are not quite the same. For instance
        we do not use a modal loop, instead the menu is non-modal and the way
        mm_startmenumode and mm_endmenumode is handled is very different.
        When items are inserted/deleted/changed/... the menu does _not_
        invalidate itself. It is up to the initiator to ensure the menu is
        redrawn.
        The wm_menuselect and mm_selectitem message is not used at all.
        The mouse interface (tracking the menu with no buttons down) was
        inspired by the win95 luser interface
History:
  ISJ 96-10-12 Creation
***/
#define WMSINCL_MSGS
#define WMSINCL_DESKTOP
#define WMSINCL_FOCUS
#define WMSINCL_CPI
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_OWNERDRAW
#include "menuwin.h"
#include <string.h>
#include <ctype.h>
#include "presparm.h"

Bool FMenuWindow_Handler::Dispatch(FEvent &ev) {
        Bool b;
        sint16 s16;
        uint16 u16;
        switch(ev.GetID()) {
                case MM_DELETEITEM:
                        b = mm_deleteitem(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_ENDMENUMODE:
                        mm_endmenumode();
                        //ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_INSERTITEM:
                        s16 = mm_insertitem((MENUITEM*)PFROMMP(ev.mp1),(char*)PFROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)s16);
                        return True;
                case MM_ISITEMVALID:
                        b = mm_isitemvalid(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_ITEMIDFROMPOSITION:
                        s16 = mm_itemidfromposition(UINT161FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)s16);
                        return True;
                case MM_ITEMPOSITIONFROMID:
                        s16 = mm_itempositionfromid(UINT161FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)s16);
                        return True;
                case MM_QUERYITEM:
                        b = mm_queryitem(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1),(MENUITEM*)PFROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_QUERYITEMATTR:
                        u16 = mm_queryitemattr(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)u16);
                        return True;
                case MM_QUERYITEMCOUNT:
                        u16 = mm_queryitemcount();
                        ev.SetResult((WmsMRESULT)u16);
                        return True;
                case MM_QUERYITEMRECT:
                        b = mm_queryitemrect(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1),(FRect*)PFROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_QUERYITEMTEXT:
                        b = mm_queryitemtext(UINT161FROMMP(ev.mp1),UINT162FROMMP(ev.mp1),(char*)PFROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_QUERYITEMTEXTLENGTH:
                        u16 = mm_queryitemtextlength(UINT161FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)u16);
                        return True;
                case MM_QUERYSELITEMID:
                        u16 = mm_queryselitemid((Bool)UINT162FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)u16);
                        return True;
                case MM_REMOVEITEM:
                        b = mm_removeitem(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_SELECTITEM:
                        b = mm_selectitem(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1),(Bool)UINT161FROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_SETITEM:
                        b = mm_setitem((Bool)UINT162FROMMP(ev.mp1),(MENUITEM*)PFROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_SETITEMATTR:
                        b = mm_setitemattr(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1),UINT161FROMMP(ev.mp2),UINT162FROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_SETITEMHANDLE:
                        b = mm_setitemhandle(UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1),UINT32FROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_SETITEMTEXT:
                        b = mm_setitemtext(UINT161FROMMP(ev.mp1),(char*)PFROMMP(ev.mp2));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case MM_STARTMENUMODE:
                        b = mm_startmenumode((Bool)UINT161FROMMP(ev.mp1),(Bool)UINT162FROMMP(ev.mp1));
                        ev.SetResult((WmsMRESULT)b);
                        return True;
                case WM_INITMENU:
                        //always owner pass-up
                        WmsSendMsg(WmsQueryWindowOwner(ev.hwnd), ev.msg, ev.mp1, ev.mp2);
                        return True;
                case WM_MENUSELECT:
                        //always owner-handled
                        ev.SetResult(WmsSendMsg(WmsQueryWindowOwner(ev.hwnd), ev.msg, ev.mp1, ev.mp2));
                        return True;
                case WM_DRAWITEM:
                        //always owner-handled
                        ev.SetResult(WmsSendMsg(WmsQueryWindowOwner(ev.hwnd), ev.msg, ev.mp1, ev.mp2));
                        return True;
                case WM_ADJUSTWINDOWPOS:
                        return wm_adjustwindowpos((WmsSWP*)PFROMMP(ev.mp1),UINT161FROMMP(ev.mp2));
                case WM_FOCUSCHANGING:
                        return wm_focuschanging(HWNDFROMMP(ev.mp1),(Bool)UINT161FROMMP(ev.mp2),UINT162FROMMP(ev.mp2));
                case WM_COMMAND:
                case WM_SYSCOMMAND:
                case WM_HELP:
                        //forward to owner
                        WmsPostMsg(WmsQueryWindowOwner(ev.hwnd), ev.msg, ev.mp1, ev.mp2);
                        return True;
                default:
                        return False;
        }
}



FMenuWindow::~FMenuWindow() {
        for(unsigned i=0; i<items; i++) {
                delete[] item[i].text;
                if(item[i].style&MIS_SUBMENU) {
                        FWnd *pwnd=GetWndMan()->FindWnd(item[i].hwndSubMenu);
                        WmsDestroyWindow(item[i].hwndSubMenu);
                        if(item[i].deleteWnd) delete pwnd;
                }
        }
        delete[] item;
        items=0;
}

Bool FMenuWindow::Create(FWnd *pwndOwner, int ID) {
        Bool b=FControl::Create(0, ID, "", 0, 0);
        if(b) {
                WmsSetWindowPos(GetHwnd(),0, 0,0,0,0, SWP_HIDE);
                if(pwndOwner) WmsSetWindowOwner(GetHwnd(), pwndOwner->GetHwnd());
                WmsSetWindowParent(GetHwnd(), HWND_DESKTOP);
                return True;
        } else
                return False;
}

int FMenuWindow::findId(int id) {
        //find item with id=<id>
        for(int i=0; i<items; i++)
                if(item[i].id == id) return i;
        return -1;
}

Bool FMenuWindow::mm_deleteitem(uint16 id, Bool includeSubMenus) {
        int i=findId(id);
        if(i!=-1) {
                delete[] item[i].text;
                if(item[i].style&MIS_SUBMENU) {
                        FWnd *pwnd=GetWndMan()->FindWnd(item[i].hwndSubMenu);
                        WmsDestroyWindow(item[i].hwndSubMenu);
                        if(item[i].deleteWnd) delete pwnd;
                }
                for(; i<items-1; i++)
                        item[i] = item[i+1];
                items--;
                adjustItemRects();
                return True;
        } else {
                if(includeSubMenus)
                        return recurseBool(MM_DELETEITEM,MPFROM2UINT16(id,True),0);
                return False;
        }
}

void FMenuWindow::mm_endmenumode() {
        //exit the menu
        int isActionBar = (WmsQueryWindowStyle(GetHwnd())&MS_ACTIONBAR)!=0;
        WmsHWND hwndFocus=WmsQueryFocus(HWND_DESKTOP);
        if(hwndFocus==GetHwnd() ||
           WmsIsOwnee(GetHwnd(),hwndFocus)) {
                WmsSetFocus(HWND_DESKTOP, WmsQueryWindowOwner(GetHwnd()));
        }
        if(!isActionBar) {
                WmsSetWindowPos(GetHwnd(), 0, 0,0,0,0, SWP_HIDE);
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                           MM_ENDMENUMODE,
                           0,0);
        }
}

sint16 FMenuWindow::mm_insertitem(const MENUITEM *ip, const char *t) {
        int pos=ip->pos;
        if(pos==-1) pos = items;
        if(pos>items) pos=items;

        if(ip->style&MIS_SUBMENU && !WmsIsWindow(ip->submenu))
                return False;
        if(ip->style&MIS_SEPARATOR) t="";
        if(ip->style&MIS_OWNERDRAW && t==0) t="";
        char *text=new char[strlen(t)+1];
        if(!text) return -1;
        strcpy(text,t);
        MenuItem *ni=new MenuItem[items+1];
        if(!ni) {
                delete[] text;
                return -1;
        }

        for(int i=0; i<pos; i++)
                ni[i]=item[i];
        for(i=pos; i<items; i++)
                ni[i+1]=item[i];
        delete[] item;
        item=ni;
        items++;

        item[pos].text = text;
        item[pos].style = ip->style;
        item[pos].attr = ip->attr;
        item[pos].hwndSubMenu = ip->submenu;
        item[pos].deleteWnd = False;
        item[pos].id = ip->id;
        item[pos].handle = ip->handle;
        if(item[pos].style&MIS_OWNERDRAW) {
                OWNERITEM oi;
                oi.controlID = WmsQueryWindowID(GetHwnd());
                oi.controlHwnd = GetHwnd();
                oi.itemID = item[pos].id;
                oi.itemRect.Set(0,0,0,0);
                oi.hdc = (WmsHDC)0;
                oi.itemState = item[pos].attr;
                oi.itemData1 = (uint32)item[pos].text;
                oi.itemData2 = item[pos].handle;
                WmsMRESULT mr = WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                                           WM_MEASUREITEM,
                                           MPFROMUINT16(oi.controlID),
                                           MPFROMP(&oi)
                                          );
                item[pos].width = UINT161FROMMR(mr);
                item[pos].height = UINT162FROMMR(mr);
                if(item[pos].height<1) item[pos].height=1;
        } else {
                item[pos].width = 0; //not used on non-ownerdraw
                item[pos].height = 1;
        }
        adjustItemRects();
        if(item[pos].style&MIS_SUBMENU)
                WmsSetWindowOwner(item[pos].hwndSubMenu, GetHwnd());

        return (sint16)pos;
}

Bool FMenuWindow::mm_isitemvalid(uint16 id, Bool includeSubMenus) {
        if(findId(id)!=-1)
                return True;
        if(includeSubMenus)
                return recurseBool(MM_ISITEMVALID,MPFROM2UINT16(id,True),0);
        return False;
}

sint16 FMenuWindow::mm_itemidfromposition(uint16 index) {
        if(/*index>=0 && */index<items)
                return item[index].id;
        else
                return -1;
}

sint16 FMenuWindow::mm_itempositionfromid(uint16 id) {
        for(int i=0; i<items; i++)
                if(item[i].id==id)
                        return (sint16)i;
        return -1;
}

Bool FMenuWindow::mm_queryitem(uint16 id, Bool includeSubMenus, MENUITEM *ip) {
        int i=findId(i);
        if(i!=-1) {
                ip->pos = (sint16)i;
                ip->style = item[i].style;
                ip->attr = item[i].attr;
                ip->id = item[i].id;
                ip->submenu = item[i].hwndSubMenu;
                ip->handle = item[i].handle;
                return True;
        }
        if(includeSubMenus)
                return recurseBool(MM_QUERYITEM, MPFROM2UINT16(id,True), MPFROMP(ip));
        return False;
}

uint16 FMenuWindow::mm_queryitemattr(uint16 id, Bool includeSubMenus) {
        int i=findId(id);
        if(i!=-1)
                return item[i].attr;
        if(includeSubMenus)
                return recurseNN(MM_QUERYITEMATTR,MPFROM2UINT16(id,True),0);
        return -1;
}

uint16 FMenuWindow::mm_queryitemcount() {
        return (uint16)items;
}

Bool FMenuWindow::mm_queryitemrect(uint16 id, Bool includeSubMenus, FRect *r) {
        int i=findId(id);
        if(i!=-1) {
                *r = item[i].r;
                return True;
        }
        if(includeSubMenus)
                return recurseBool(MM_QUERYITEMRECT, MPFROM2UINT16(id,True), MPFROMP(r));
        return False;
}

Bool FMenuWindow::mm_queryitemtext(uint16 id, uint16 maxbuf, char *buf) {
        int i=findId(id);
        if(i!=-1) {
                if(strlen(item[i].text)+1>maxbuf)
                        return False;
                strcpy(buf,item[i].text);
                return True;
        }
        return False;
}

uint16 FMenuWindow::mm_queryitemtextlength(uint16 id) {
        int i=findId(id);
        if(i!=-1)
                return (uint16)(strlen(item[i].text)+1);
        return 0;

}

uint16 FMenuWindow::mm_queryselitemid(Bool includeSubMenus) {
        for(int i=0; i<items; i++)
                if(item[i].attr&MIA_HILITE)
                        return (uint16)i;
        if(includeSubMenus)
                return recurseNN(MM_QUERYSELITEMID,MPFROM2UINT16(0,True),0);
        return -1;
}

Bool FMenuWindow::mm_removeitem(uint16 id, Bool includeSubMenus) {
        //remove item but do not destroy submenu
        int i=findId(id);
        if(i!=-1) {
                for(; i<items-1; i++)
                        item[i] = item[i+1];
                items--;
                adjustItemRects();
                return True;
        }
        if(includeSubMenus)
                return recurseBool(MM_REMOVEITEM, MPFROM2UINT16(id,True),0);
        return False;
}

Bool FMenuWindow::mm_selectitem(uint16 /*id*/, Bool /*includeSubMenus*/, Bool /*dismiss*/) {
        //not used
        return False;
}

Bool FMenuWindow::mm_setitem(Bool includeSubMenus, const MENUITEM *ip) {
        int i=findId(ip->id);
        if(i!=-1) {
                if(item[i].style&MIS_SUBMENU != ip->style&MIS_SUBMENU)
                        return False;
                if(item[i].style&MIS_SEPARATOR != ip->style&MIS_SEPARATOR)
                        return False;
                item[i].style = ip->style;
                item[i].attr = ip->attr;
                item[i].id = ip->id;
                item[i].hwndSubMenu = ip->submenu;
                item[i].handle = ip->handle;
                return True;
        }
        if(includeSubMenus)
                return recurseBool(MM_SETITEM, MPFROM2UINT16(0,True),MPFROMP(ip));
        return False;
}

Bool FMenuWindow::mm_setitemattr(uint16 id, Bool includeSubMenus, uint16 mask, uint16 bits) {
        int i=findId(id);
        if(i!=-1) {
                item[i].attr &= ~mask;
                item[i].attr |= bits;
                return True;
        }
        if(includeSubMenus)
                return recurseBool(MM_SETITEMATTR, MPFROM2UINT16(id,True),MPFROM2UINT16(mask,bits));
        return False;
}

Bool FMenuWindow::mm_setitemhandle(uint16 id, Bool includeSubMenus, uint32 h) {
        int i=findId(id);
        if(i!=-1) {
                item[i].handle = h;
                return True;
        }
        if(includeSubMenus)
                return recurseBool(MM_SETITEMATTR, MPFROM2UINT16(id,True),MPFROMUINT32(h));
        return False;
}

Bool FMenuWindow::mm_setitemtext(uint16 id, const char *text) {
        int i=findId(id);
        if(i!=-1) {
                int ntl=strlen(text);
                int otl=strlen(item[i].text);
                if(ntl<=otl) {
                        strcpy(item[i].text,text);
                        item[i].width = strlen(item[i].text);
                        adjustItemRects();
                        return True;
                }
                char *nt=new char[ntl+1];
                if(nt) {
                        delete[] item[i].text;
                        item[i].text = nt;
                        strcpy(item[i].text,text);
                        //todo
                        item[i].width = strlen(item[i].text);
                        adjustItemRects();

                        return True;
                }
        }
        return False;
}

Bool FMenuWindow::mm_startmenumode(Bool /*showSubMenu*/, Bool resumeMenu) {
        //show ourselves in case we are hidden
        if((WmsQueryWindowStyle(GetHwnd())&WS_VISIBLE)==0)
                WmsSetWindowPos(GetHwnd(), HWND_TOP, 0,0,0,0, SWP_SHOW|SWP_ZORDER);
        else
                WmsSetWindowPos(GetHwnd(), HWND_TOP, 0,0,0,0, SWP_ZORDER);
        //set focus to ourselves
        WmsChangeFocus(HWND_DESKTOP,GetHwnd(),FC_NODEACTIVATE);
        int i=findSelectedItem();
        if((i<0 || i>=items) || !resumeMenu) {
                //deselect previous item
                selectItem(-1);
                //set initial selected item
                i=findNextSelectableItem(-1);
                if(i>=0 && i<items) {
                        item[i].attr |= MIA_HILITE;
                        WmsInvalidateRect(GetHwnd(), &item[i].r, False);
                }
        }
        return True;
}


Bool FMenuWindow::wm_adjustwindowpos(WmsSWP *pswp, uint16 flags) {
        if(flags&SWP_MOVE|SWP_SHOW|SWP_SIZE) {
                //Modify size so the window rectangle fits the items
                int isActionBar = (WmsQueryWindowStyle(GetHwnd())&MS_ACTIONBAR)!=0;
                if((!isActionBar) && items>0)
                {
                        //ensure entire menu is visible
                        FRect dr;
                        WmsQueryWindowRect(HWND_DESKTOP,&dr);
                        FRect wr;
                        WmsQueryWindowRect(GetHwnd(),&wr);
                        WmsMapWindowRects(GetHwnd(),HWND_DESKTOP,&wr,1);

                        pswp->cx = item[0].r.GetWidth()+2;
                        pswp->cy = item[items-1].r.GetBottom() + 1;
                        pswp->flags |= SWP_SIZE;

                        wr.SetWidth(pswp->cx);
                        wr.SetHeight(pswp->cy);
                        
                        if(flags&SWP_MOVE)
                                wr.MoveAbsolute(pswp->x,pswp->y);
                        if(wr.GetRight()>dr.GetRight()) {
                                pswp->x -= wr.GetRight()-dr.GetRight();
                                pswp->flags |= SWP_MOVE;
                        }
                        if(wr.GetLeft()<dr.GetLeft()) {
                                pswp->x += dr.GetLeft()-wr.GetLeft();
                                pswp->flags |= SWP_MOVE;
                        }
                        if(wr.GetBottom()>dr.GetBottom()) {
                                pswp->y -= wr.GetBottom()-dr.GetBottom();
                                pswp->flags |= SWP_MOVE;
                        }
                        if(wr.GetTop()<dr.GetTop()) {
                                pswp->y += dr.GetTop()-wr.GetTop();
                                pswp->flags |= SWP_MOVE;
                        }
                }
                if(isActionBar) {
                        if(flags&SWP_SIZE) {
                                adjustItemRects(pswp->cx);
                                if(items>0)
                                        pswp->cy = item[items-1].r.GetBottom();
                                else
                                        pswp->cy = 1;
                        }
                }
        }
        if(flags&SWP_HIDE) {
                //ensure submenus are rolled up
                rollupSubMenus();
                //inform the parent of the event
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                           WM_MENUEND,
                           MPFROMUINT16(WmsQueryWindowID(GetHwnd())),
                           MPFROMHWND(GetHwnd())
                          );
        }
        if(flags&SWP_SHOW) {
                //inform the parent of the event
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                           WM_INITMENU,
                           MPFROMUINT16(WmsQueryWindowID(GetHwnd())),
                           MPFROMHWND(GetHwnd())
                          );
        }
        return False;
}

Bool FMenuWindow::Paint() {
        FRect pr;
        WmsHDC hdc = WmsBeginPaint(GetHwnd(),&pr);
        if(!hdc) return False;

        FRect wr;
        WmsQueryWindowRect(GetHwnd(),&wr);
        int isActionBar = (WmsQueryWindowStyle(GetHwnd())&MS_ACTIONBAR)!=0;

        color fg_clr,bg_clr;
        //erase background
        WmsQueryPresParam(GetHwnd(), PP_MBACKGROUNDCOLOR,0,0, &bg_clr,1);
        WmsQueryPresParam(GetHwnd(), PP_MFOREGROUNDCOLOR,0,0, &fg_clr,1);
        CpiEraseRect(hdc, wr, fg_clr|bg_clr);
        uint16 d;
        if(!isActionBar) {
                //draw border
                WmsQueryPresParam(GetHwnd(), PP_MBORDERFOREGROUNDCOLOR,PP_MBACKGROUNDCOLOR,0, &bg_clr,1);
                WmsQueryPresParam(GetHwnd(), PP_MBORDERBACKGROUNDCOLOR,PP_MFOREGROUNDCOLOR,0, &fg_clr,1);
                char buf[8];
                WmsQueryPresParam(GetHwnd(), PP_MBORDERCHARS,0,0,buf,8);
                d =  (uint16)((((uint16)fg_clr)|bg_clr)<<8);
                CpiSetCellData(hdc, FPoint(wr.GetLeft(),wr.GetTop()), d|buf[0]);
                for(int x=wr.GetLeft()+1; x<wr.GetRight()-1; x++) {
                        CpiSetCellData(hdc, FPoint(x,wr.GetTop()), d|buf[1]);
                        CpiSetCellData(hdc, FPoint(x,wr.GetBottom()-1), d|buf[6]);
                }
                CpiSetCellData(hdc, FPoint(wr.GetRight()-1,wr.GetTop()), d|buf[2]);
                for(int y=wr.GetTop()+1; y<wr.GetBottom()-1; y++) {
                        CpiSetCellData(hdc, FPoint(wr.GetLeft(),y), d|buf[3]);
                        CpiSetCellData(hdc, FPoint(wr.GetRight()-1,y), d|buf[4]);
                }
                CpiSetCellData(hdc, FPoint(wr.GetLeft(),wr.GetBottom()-1), d|buf[5]);
                CpiSetCellData(hdc, FPoint(wr.GetRight()-1,wr.GetBottom()-1), d|buf[7]);
        }
        //draw items
        color sepfg_clr=fg_clr,sepbg_clr=bg_clr;
        color nfg_clr,nbg_clr;
        WmsQueryPresParam(GetHwnd(), PP_MNORMALFOREGROUNDCOLOR,PP_MFOREGROUNDCOLOR,0,&nfg_clr,1);
        WmsQueryPresParam(GetHwnd(), PP_MNORMALBACKGROUNDCOLOR,PP_MBACKGROUNDCOLOR,0,&nbg_clr,1);
        color hotfg_clr,hotbg_clr;
        WmsQueryPresParam(GetHwnd(), PP_MHOTKEYFOREGROUNDCOLOR,PP_MFOREGROUNDCOLOR,0,&hotfg_clr,1);
        WmsQueryPresParam(GetHwnd(), PP_MHOTKEYBACKGROUNDCOLOR,PP_MBACKGROUNDCOLOR,0,&hotbg_clr,1);
        color disfg_clr,disbg_clr;
        WmsQueryPresParam(GetHwnd(), PP_MDISABLEDFOREGROUNDCOLOR,PP_MFOREGROUNDCOLOR,0,&disfg_clr,1);
        WmsQueryPresParam(GetHwnd(), PP_MDISABLEDBACKGROUNDCOLOR,PP_MBACKGROUNDCOLOR,0,&disbg_clr,1);
        color selfg_clr,selbg_clr;
        WmsQueryPresParam(GetHwnd(), PP_MSELECTEDFOREGROUNDCOLOR,0,0,&selfg_clr,1);
        WmsQueryPresParam(GetHwnd(), PP_MSELECTEDBACKGROUNDCOLOR,0,0,&selbg_clr,1);
        color statfg_clr,statbg_clr;
        WmsQueryPresParam(GetHwnd(), PP_MSTATICFOREGROUNDCOLOR,PP_MFOREGROUNDCOLOR,0,&statfg_clr,1);
        WmsQueryPresParam(GetHwnd(), PP_MSTATICBACKGROUNDCOLOR,PP_MBACKGROUNDCOLOR,0,&statbg_clr,1);
        int maxl;
        measureItemParts(&maxl,0,0);
        for(int i=0; i<items; i++) {
                FRect r=item[i].r;
                if(!r.Intersects(pr))
                        continue;
                if(item[i].attr&MIA_DISABLED)
                        fg_clr=disfg_clr,bg_clr=disbg_clr;
                else if(item[i].style&MIS_STATIC)
                        fg_clr=statfg_clr,bg_clr=statbg_clr;
                else if(item[i].style&MIS_SEPARATOR)
                        fg_clr=sepfg_clr,bg_clr=sepbg_clr;
                else if(item[i].attr&MIA_HILITE)
                        fg_clr=selfg_clr,bg_clr=selbg_clr;
                else 
                        fg_clr=nfg_clr,bg_clr=nbg_clr;
                d = (uint16) ((((uint16)fg_clr)|bg_clr)<<8);
                if(item[i].style&MIS_SEPARATOR) {
                        char c;
                        WmsQueryPresParam(GetHwnd(), PP_MSEPARATORCHAR,0,0,&c,1);
                        for(int x=r.GetLeft(); x<r.GetRight(); x++)
                                CpiSetCellData(hdc, FPoint(x,r.GetTop()), d|c);
                        continue;
                }
                if(!isActionBar) {
                        char c;
                        if(item[i].attr&MIA_CHECKED)
                                WmsQueryPresParam(GetHwnd(), PP_MCHECKCHAR,0,0,&c,1);
                        else
                                c=' ';
                        CpiSetCellData(hdc, FPoint(r.GetLeft(),r.GetTop()), d|c);
                        if(item[i].style&MIS_SUBMENU)
                                WmsQueryPresParam(GetHwnd(), PP_MSUBMENUCHAR,0,0,&c,1);
                        else
                                c=' ';
                        CpiSetCellData(hdc, FPoint(r.GetRight()-1,r.GetTop()), d|c);
                        CpiEraseRect(hdc, r.GetLeft()+1,r.GetTop(),r.GetRight()-1,r.GetBottom(), fg_clr|bg_clr);
                } else
                        CpiEraseRect(hdc, r.GetLeft(),r.GetTop(),r.GetRight(),r.GetBottom(), fg_clr|bg_clr);

                if(item[i].style&MIS_OWNERDRAW) {
                        OWNERITEM oi;
                        oi.controlID = WmsQueryWindowID(GetHwnd());
                        oi.controlHwnd = GetHwnd();
                        oi.itemID = item[i].id;
                        oi.itemRect = r;
                        oi.hdc = hdc;
                        oi.itemState = item[i].attr;
                        oi.itemData1 = (uint32)item[i].text;
                        oi.itemData2 = item[i].handle;
                        WmsMRESULT mr = WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                                                   WM_DRAWITEM,
                                                   MPFROMUINT16(oi.controlID),
                                                   MPFROMP(&oi)
                                                  );
                        if(UINT161FROMMR(mr))
                                continue;
                        //else: draw the item ourselves
                }
                char buf[256];
                strcpy(buf,item[i].text);
                char *p=buf;
                while(*p && *p!='\t') p++;
                if(*p) *p++='\0';
                uint8 nclr,hclr;
                if((item[i].attr&MIA_DISABLED)==0 && (item[i].attr&MIA_HILITE)==0)
                        nclr=(uint8)(fg_clr|bg_clr), hclr=(uint8)(hotfg_clr|hotbg_clr);
                else
                        hclr=nclr=(uint8)(fg_clr|bg_clr);

                CpiHotkeyTextout(hdc,
                                 r.GetLeft()+1,r.GetTop(),r.GetRight()-1,r.GetBottom(),
                                 buf,'@',
                                 nclr,hclr
                                );
                if(*p) //draw accelerator
                        CpiHotkeyTextout(hdc,
                                         r.GetLeft()+1+maxl+1,r.GetTop(),r.GetRight()-1,r.GetBottom(),
                                         p,'@',
                                         nclr,hclr
                                        );
        }

        WmsEndPaint(hdc);
        return True;
}

Bool FMenuWindow::KeyPressed(FKeyboardEvent &ev) {
        int isActionBar = (WmsQueryWindowStyle(GetHwnd())&MS_ACTIONBAR)!=0;
        logical_key lk=ev.GetLogicalKey();
        switch(lk) {
                case lk_left:
                        if(isActionBar) {
                                int i=findSelectedItem();
                                i = findPrevSelectableItem(i);
                                if(i<0) {
                                        WmsMRESULT mr = WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                                                                   WM_NEXTMENU,
                                                                   MPFROMHWND(GetHwnd()),
                                                                   MPFROMUINT16(True)
                                                                  );
                                        if(WmsIsWindow((WmsHWND)mr)) {
                                                WmsSendMsg((WmsHWND)mr, MM_STARTMENUMODE, MPFROM2UINT16(False,True), 0);
                                                return True;
                                        }
                                }
                                //todo: keep submenus dropped
                                WmsChangeFocus(HWND_DESKTOP,GetHwnd(),FC_NODEACTIVATE);
                                rollupSubMenus();
                                if(i>=0) {
                                        selectItem(i);
                                } else {
                                        //todo
                                }
                        } else {
                                if(WmsQueryFocus(HWND_DESKTOP)==GetHwnd()) {
                                        return False; //let owner handle it
                                } else {
                                        WmsChangeFocus(HWND_DESKTOP,GetHwnd(),FC_NODEACTIVATE);
                                        rollupSubMenus();
                                        return True;
                                }
                        }
                        return True;
                case lk_right:
                        if(isActionBar) {
                                int i=findSelectedItem();
                                i = findNextSelectableItem(i);
                                if(i>=items) {
                                        WmsMRESULT mr = WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                                                                   WM_NEXTMENU,
                                                                   MPFROMHWND(GetHwnd()),
                                                                   MPFROMUINT16(False)
                                                                  );
                                        if(WmsIsWindow((WmsHWND)mr)) {
                                                WmsSendMsg((WmsHWND)mr, MM_STARTMENUMODE, MPFROM2UINT16(False,True), 0);
                                                return True;
                                        }
                                }
                                //todo: keep submenus dropped
                                WmsChangeFocus(HWND_DESKTOP,GetHwnd(),FC_NODEACTIVATE);
                                rollupSubMenus();
                                if(i<items) {
                                        selectItem(i);
                                } else {
                                        //todo
                                }
                        } else {
                                int i=findSelectedItem();
                                if(i>=0 && i<items) {
                                        if(item[i].style&MIS_SUBMENU &&
                                           WmsQueryFocus(HWND_DESKTOP)==GetHwnd())
                                        {
                                                dropdownSubMenu(i,True);
                                                WmsPostMsg(item[i].hwndSubMenu, MM_STARTMENUMODE, MPFROM2UINT16(False,True),0);
                                        } else {
                                                //owner pass-up
                                                return False;
                                        }
                                } else {
                                        //ignore
                                }
                        }
                        return True;
                case lk_up:
                        if(!isActionBar) {
                                int i=findSelectedItem();
                                if(i<0 | i>=items)
                                        i=findPrevSelectableItem(items);
                                else
                                        i=findPrevSelectableItem(i);
                                if(i<0 || i>=items)
                                        i=findPrevSelectableItem(items);
                                if(i>=0 && i<items) {
                                        rollupSubMenus();
                                        selectItem(i);
                                }
                        }
                        return True;
                case lk_down:
                        if(isActionBar) {
                                int i=findSelectedItem();
                                if(i>=0 && i<items && item[i].style&MIS_SUBMENU) {
                                        dropdownSubMenu(i,False);
                                        WmsPostMsg(item[i].hwndSubMenu, MM_STARTMENUMODE, MPFROM2UINT16(False,False),0);
                                }
                        } else {
                                int i=findSelectedItem();
                                if(i<0 || i>=items)
                                        i=findNextSelectableItem(-1);
                                else
                                        i=findNextSelectableItem(i);
                                if(i<0 || i>=items)
                                        i=findNextSelectableItem(-1);
                                if(i>=0 && i<items) {
                                        rollupSubMenus();
                                        selectItem(i);
                                }
                        }
                        return True;
                case lk_enter: {
                case lk_numenter:
                        int i=findSelectedItem();
                        if(i>=0 && i<items && (item[i].attr&MIA_DISABLED)==0) {
                                if(item[i].style&MIS_SUBMENU) {
                                        rollupSubMenus();
                                        dropdownSubMenu(i,True);
                                        WmsPostMsg(item[i].hwndSubMenu, MM_STARTMENUMODE, MPFROM2UINT16(False,True),0);
                                } else
                                        chooseItem(i,False);
                        }
                        return True;
                }
                case lk_esc: {
                        WmsChangeFocus(HWND_DESKTOP, WmsQueryWindowOwner(GetHwnd()),FC_NODEACTIVATE);
                        return True;
                }
                default:
                        if(lk>=lk_a && lk<=lk_z) {
                                processMnemonic((char)('a'+lk-lk_a));
                                return True;
                        }
                        if(lk>=lk_1 && lk<=lk_9) {
                                processMnemonic((char)('1'+lk-lk_1));
                                return True;
                        }
                        if(lk==lk_0) {
                                processMnemonic('0');
                                return True;
                        }
                        return False;
        }
}

Bool FMenuWindow::CharInput(FKeyboardEvent &ev) {
        return processMnemonic(ev.GetChar());
}

Bool FMenuWindow::MatchMnemonic(char c) {
        c = (char)tolower(c);
        for(int i=0; i<items; i++) {
                if(isSelectableItem(i) && item[i].text) {
                        char *p=strchr(item[i].text,'@');
                        if(p && tolower(p[1])==c)
                                return True;
                }
        }
        return False;
}

Bool FMenuWindow::processMnemonic(char c) {
        c = (char)tolower(c);
        for(int i=0; i<items; i++) {
                if(isSelectableItem(i) && item[i].text) {
                        char *p=strchr(item[i].text,'@');
                        if(p && tolower(p[1])==c) {
                                if(item[i].style&MIS_SUBMENU) {
                                        rollupSubMenus();
                                        selectItem(i);
                                        dropdownSubMenu(i,True);
                                        WmsPostMsg(item[i].hwndSubMenu, MM_STARTMENUMODE, MPFROM2UINT16(False,True),0);
                                } else
                                        chooseItem(i,False);
                                return True;
                        }
                }
        }
        return False;
}

Bool FMenuWindow::wm_focuschanging(WmsHWND otherHwnd, Bool get, uint16 /*flags*/) {
        if(get) return False;
        //focus is changing to <otherHwnd>
        if(otherHwnd!=GetHwnd() &&
           !WmsIsOwnee(GetHwnd(),otherHwnd))
        {
                //losing focus to non-ownee
                rollupSubMenus();
                uint32 style=WmsQueryWindowStyle(GetHwnd());
                if(style&MS_ACTIONBAR)
                        selectItem(-1);
                if((style&WS_VISIBLE) &&
                   (style&MS_ACTIONBAR)==0)
                        WmsSetWindowPos(GetHwnd(), 0,0,0,0,0, SWP_HIDE);
        }
        return False;
}


Bool FMenuWindow::ButtonDown(FButtonDownEvent &ev) {
        int i=point2Item(ev.GetPoint());
        if(!isSelectableItem(i)) return False;
        WmsChangeFocus(HWND_DESKTOP, GetHwnd(), FC_NODEACTIVATE);
        if(i==findSelectedItem()) return False;
        rollupSubMenus();
        selectItem(i);
        if(item[i].style&MIS_SUBMENU) {
                dropdownSubMenu(i,False);
        } else
                chooseItem(i,True);
        return False;
}

Bool FMenuWindow::ButtonUp(FButtonUpEvent &ev) {
        if(WmsQueryWindowStyle(GetHwnd())&MS_ACTIONBAR) {
                //only do mouse stuff if we or a submenu has the focus
                WmsHWND hwndFocus=WmsQueryFocus(HWND_DESKTOP);
                if(hwndFocus!=GetHwnd() &&
                   !WmsIsOwnee(GetHwnd(),hwndFocus))
                        return False;
        }
        int i=point2Item(ev.GetPoint());
        if(!isSelectableItem(i)) return False;
        if(i!=findSelectedItem()) return False;
        if(item[i].style&MIS_SUBMENU) return False;
        chooseItem(i,True);
        
        return False;
}

Bool FMenuWindow::MouseMove(FMouseMoveEvent &ev) {
        if(WmsQueryWindowStyle(GetHwnd())&MS_ACTIONBAR) {
                //only do mouse stuff if we or a submenu has the focus
                WmsHWND hwndFocus=WmsQueryFocus(HWND_DESKTOP);
                if(hwndFocus!=GetHwnd() &&
                   !WmsIsOwnee(GetHwnd(),hwndFocus))
                        return False;
        }
        int i=point2Item(ev.GetPoint());
        if(!isSelectableItem(i)) return False;
        if(i==findSelectedItem()) return False; //optimize
        rollupSubMenus();
        selectItem(i);
        if(item[i].style&MIS_SUBMENU)
                dropdownSubMenu(i,False);
        
        return False;
}

int FMenuWindow::point2Item(FPoint p) {
        for(int i=0; i<items;i++) {
                if(item[i].r.Contains(p))
                        return i;
        }
        return -1;
}

void FMenuWindow::adjustItemRects() {
        if(WmsQueryWindowStyle(GetHwnd())&MS_ACTIONBAR) {
                //"flow" items
                FRect wr;
                WmsQueryWindowRect(GetHwnd(),&wr);
                adjustItemRects(wr.GetWidth());
        } else {
                int maxl,
                    maxa,
                    maxo;
                measureItemParts(&maxl,&maxa,&maxo);
                int maxwidth;
                if(maxl+maxa>maxo)
                        maxwidth = maxl+maxa;
                else
                        maxwidth = maxo;
                if(maxa>0) maxwidth++; //include space between text and accelerator
                maxwidth += 2; //include checkmark and submenu indicator
                int y=1;
                for(int i=0; i<items; i++) {
                        item[i].r.Set(1,y,1+maxwidth,y+item[i].height);
                        y += item[i].height;
                }
        }
}

void FMenuWindow::adjustItemRects(int w) {
        //flow items in an actionbar
        int x=0;
        int y=0;
        for(int i=0; i<items; i++) {
                int iw=0;
                if(item[i].style&MIS_OWNERDRAW) {
                        iw=item[i].width;
                } else {
                        iw=0;
                        char *p=item[i].text;
                        while(*p) {
                                if(*p!='@') iw++;
                                p++;
                        }
                }
                if(x>0 && x+iw+2>w)
                        x=0,y++;
                item[i].r.Set(x,y,x+iw+2,y+1);
                x += iw+2;
        }
};

void FMenuWindow::measureItemParts(int *maxLeft, int *maxAccel, int *maxOwnerdraw) {
        int maxl=0,
            maxa=0,
            maxo=0;
        for(int i=0; i<items; i++) {
                if(item[i].style&MIS_OWNERDRAW) {
                        if(item[i].width>maxo) maxo=item[i].width;
                } else if(item[i].style&MIS_SEPARATOR) {
                        //nothing
                } else {
                        char *p=item[i].text;
                        char *s;
                        //measure left part
                        s=p;
                        while(*p && *p!='\t') {
                                if(*p=='@') s++;
                                p++;
                        }
                        int w=p-s;
                        if(w>maxl) maxl=w;
                        if(*p=='\t') p++;
                        //measure accelerator part
                        s=p;
                        while(*p) {
                                if(*p=='@') s++;
                                p++;
                        }
                        w=p-s;
                        if(w>maxa) maxa=w;
                }
        }
        if(maxLeft) *maxLeft=maxl;
        if(maxAccel) *maxAccel=maxa;
        if(maxOwnerdraw) *maxOwnerdraw=maxo;
}

void FMenuWindow::rollupSubMenus() {
        for(int i=0; i<items; i++)
                if(item[i].style&MIS_SUBMENU && WmsQueryWindowStyle(item[i].hwndSubMenu)&WS_VISIBLE)
                        WmsSetWindowPos(item[i].hwndSubMenu,0,0,0,0,0,SWP_HIDE);
}

void FMenuWindow::dropdownSubMenu(int i, Bool resume) {
        FPoint p;
        if(WmsQueryWindowStyle(GetHwnd())&MS_ACTIONBAR) {
                p.Set(item[i].r.GetLeft()-1,item[i].r.GetBottom());
        } else {
                p.Set(item[i].r.GetRight(),item[i].r.GetTop()-1);
        }
        if(!resume)
                WmsSendMsg(item[i].hwndSubMenu,MM_SELECTITEM, MPFROM2UINT16(-1,False),MPFROM2UINT16(0,False));
        WmsMapWindowPoints(GetHwnd(), WmsQueryWindowParent(item[i].hwndSubMenu), &p, 1);
        WmsSetWindowPos(item[i].hwndSubMenu,
                        HWND_TOP,
                        p.GetX(),p.GetY(),
                        0,0,
                        SWP_MOVE|SWP_ZORDER|SWP_SHOW
                       );
}

int FMenuWindow::findSelectedItem() {
        for(int i=0; i<items; i++)
                if(item[i].attr&MIA_HILITE)
                        return i;
        return -1;
}

int FMenuWindow::findPrevSelectableItem(int i) {
        i--;
        while(i>=0) {
                if(isSelectableItem(i))
                        break;
                i--;
        }
        return i;
}

int FMenuWindow::findNextSelectableItem(int i) {
        i++;
        while(i<items) {
                if(isSelectableItem(i))
                        break;
                i++;
        }
        return i;
}

void FMenuWindow::selectItem(int i) {
        int oi=findSelectedItem();
        if(oi>=0 && oi<items) {
                item[oi].attr &= ~MIA_HILITE;
                WmsInvalidateRect(GetHwnd(),&item[oi].r,False);
        }
        if(i>=0 && i<items) {
               item[i].attr |= MIA_HILITE;
               WmsInvalidateRect(GetHwnd(),&item[i].r,False);
        }
}

void FMenuWindow::chooseItem(int i, Bool pointerDidIt) {
        if(!isSelectableItem(i))
                return;
        WmsMSG msg;
        if(item[i].style&MIS_HELP)
                msg = WM_HELP;
        else if(item[i].style&MIS_SYSCOMMAND)
                msg = WM_SYSCOMMAND;
        else
                msg = WM_COMMAND;

        WmsPostMsg(GetHwnd(), msg, MPFROMUINT16(item[i].id), MPFROM2UINT16(CMDSRC_MENU,pointerDidIt));
        if((item[i].attr&MIA_NODISMISS)==0)
                WmsSendMsg(GetHwnd(), MM_ENDMENUMODE, MPFROMUINT16(True), 0);
}

Bool FMenuWindow::isSelectableItem(int i) {
        if(i<0 || i>=items ||
           (item[i].style&MIS_SEPARATOR) ||
           (item[i].style&MIS_STATIC) ||
           (item[i].attr&MIA_DISABLED))
                return False;
        return True;
}

Bool FMenuWindow::recurseBool(WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2) {
        //recurse submenus until one of them returns true
        for(int i=0; i<items; i++) {
                if(item[i].style&MIS_SUBMENU) {
                        WmsMRESULT mr=WmsSendMsg(item[i].hwndSubMenu,msg,mp1,mp2);
                        if(UINT161FROMMR(mr))
                                return True;
                }
        }
        return False;
}

uint16 FMenuWindow::recurseNN(WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2) {
        //recurse submenus until one of them returns a non-negative number
        for(int i=0; i<items; i++) {
                if(item[i].style&MIS_SUBMENU) {
                        WmsMRESULT mr=WmsSendMsg(item[i].hwndSubMenu,msg,mp1,mp2);
                        if((sint16)UINT161FROMMR(mr) >= 0)
                                return (uint16)mr;
                }
        }
        return -1;
}

