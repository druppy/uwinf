/***
Filename: listbox.cpp
Description: Implementation of the listbox class under WMS
History:
  ISJ 96-08-27 Created
***/

#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_LISTBOX
#define WMSINCL_MSGS
#define WMSINCL_CPI
#include <bif.h>
#include "presparm.h"
#include <stdlib.h>
#include <string.h>



//FListBox_Handler-------------------------------------------------------------
Bool FListBox_Handler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case LM_DELETEALL:
                        lm_deleteall();
                        ev.SetResult((WmsMRESULT)True);
                        return True;
                case LM_DELETEITEM:
                        ev.SetResult((WmsMRESULT)lm_deleteitem((int)UINT161FROMMP(ev.mp1)));
                        return True;
                case LM_INSERTITEM:
                        ev.SetResult((WmsMRESULT)lm_insertitem((const char*)PFROMMP(ev.mp2), (int)(sint16)UINT161FROMMP(ev.mp1)));
                        return True;
                case LM_QUERYITEMCOUNT:
                        ev.SetResult((WmsMRESULT)lm_queryitemcount());
                        return True;
                case LM_QUERYITEMDATA:
                        ev.SetResult((WmsMRESULT)lm_queryitemdata((int)UINT161FROMMP(ev.mp1)));
                        return True;
                case LM_QUERYITEMTEXT:
                        ev.SetResult((WmsMRESULT)lm_queryitemtext((char*)PFROMMP(ev.mp2), (int)UINT162FROMMP(ev.mp1), (int)UINT161FROMMP(ev.mp1)));
                        return True;
                case LM_QUERYITEMTEXTLEN:
                        ev.SetResult((WmsMRESULT)lm_queryitemtextlen((int)UINT161FROMMP(ev.mp1)));
                        return True;
                case LM_QUERYSELECTION:
                        ev.SetResult((WmsMRESULT)lm_queryselection((int)(sint16)UINT161FROMMP(ev.mp1)));
                        return True;
                case LM_QUERYTOPINDEX:
                        ev.SetResult((WmsMRESULT)lm_querytopindex());
                        return True;
                case LM_SEARCHSTRING:
                        ev.SetResult((WmsMRESULT)lm_searchstring((char*)PFROMMP(ev.mp2), (int)(sint16)UINT161FROMMP(ev.mp1), UINT162FROMMP(ev.mp1)));
                        return True;
                case LM_SELECTITEM:
                        ev.SetResult((WmsMRESULT)lm_selectitem((int)UINT161FROMMP(ev.mp1),(Bool)UINT161FROMMP(ev.mp2)));
                        return True;
                case LM_SETITEMDATA:
                        ev.SetResult((WmsMRESULT)lm_setitemdata(UINT32FROMMP(ev.mp2),(int)UINT161FROMMP(ev.mp1)));
                        return True;
                case LM_SETITEMTEXT:
                        ev.SetResult((WmsMRESULT)lm_setitemtext((const char*)PFROMMP(ev.mp2),(int)UINT161FROMMP(ev.mp1)));
                        return True;
                case LM_SETTOPINDEX:
                        ev.SetResult((WmsMRESULT)lm_settopindex((int)UINT161FROMMP(ev.mp1)));
                        return True;
                case WM_PAINT:
                        wm_paint();
                        ev.SetResult((WmsMRESULT)True);
                        return True;
                default:
                        return False;
        }
}


FListBox::FListBox()
  : FListBox_Handler(this),
    FKeyboardHandler(this),
    FFocusHandler(this),
    FTranslatedMouseHandler(this),
    item(0),
    items(0),
    allocatedItems(0),
    topItem(0),
    focusedItem(0)
{
}

FListBox::~FListBox() {
        delete[] item;
}


Bool FListBox::Create(FWnd *pParent, int ID, FRect *rect, uint32 flStyle) {
        return FControl::Create(pParent, ID, "", flStyle, rect);
}

Bool FListBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"nointegralheight")==0) {
                        SetStyle(~(long)lbs_nointegralheight,lbs_nointegralheight);
                } else if(strcmp(name[p],"horizontalscroll")==0) {
                        SetStyle(~(long)lbs_horizontalscroll,lbs_horizontalscroll);
                }
        }
        return True;
}

int FListBox::insertItem(const ListBoxItem &lbi, int pos) {
        if(pos<-1) return -1;
        if(items+1>allocatedItems) {
                int nalloc=allocatedItems+8;
                ListBoxItem *np = new ListBoxItem[nalloc];
                if(!np) {
                        nalloc=allocatedItems+1;
                        np = new ListBoxItem[nalloc];
                }
                if(!np) return -1;

                if(item) {
                        memcpy(np, item, sizeof(ListBoxItem)*items);
                        delete item;
                }
                item = np;
                allocatedItems=nalloc;
        }
        if(pos==-1) pos=items;
        if(pos<items)
                memmove(item+pos+1,item+pos,sizeof(ListBoxItem)*(items-pos));
        item[pos] = lbi;
        items++;

        if(pos<=topItem && items>1) topItem++;
        if(pos<=focusedItem && items>1) focusedItem++;
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        if(pos>=topItem && pos<topItem+r.GetHeight()) {
                r.SetTop(pos-topItem);
                WmsInvalidateRect(GetHwnd(),&r,False);
        }
        return pos;
}

Bool FListBox::removeItem(int pos) {
        if(pos<0 || pos>=items)
                return False;
        memmove(item+pos,item+pos+1,sizeof(ListBoxItem)*(items-pos-1));
        items--;

        if(pos<=topItem) topItem--;
        if(pos<=focusedItem) focusedItem--;
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        if(pos>=topItem && pos<topItem+r.GetHeight()) {
                r.SetTop(pos-topItem);
                WmsInvalidateRect(GetHwnd(),&r,False);
        }
        return True;

}

Bool FListBox::focusItem(int pos) {
        if(pos<0 || pos>=items) return False;
        if(focusedItem!=pos) {
                invalidateItem(focusedItem);
                focusedItem=pos;
                invalidateItem(focusedItem);
        }
        return True;
}

Bool FListBox::invalidateItem(int pos) {
        if(pos<0 || pos>=items)
                return False;
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        r.SetTop(pos-topItem);
        r.SetBottom(pos-topItem+1);
        WmsInvalidateRect(GetHwnd(), &r, False);
        return True;
}


void FListBox::fixFocus() {
        //scroll the focused item into view
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        if(focusedItem<topItem-1) {
                topItem=focusedItem-2;
                if(topItem<0) topItem=0;
                WmsInvalidateRect(GetHwnd(), 0, False);
        } else if(focusedItem==topItem-1) {
                topItem=focusedItem;
                WmsInvalidateRect(GetHwnd(), 0, False);
        } else if(focusedItem==topItem+r.GetHeight()) {
                topItem++;
                WmsInvalidateRect(GetHwnd(), 0, False);
        } else if(focusedItem>topItem+r.GetHeight()) {
                topItem=focusedItem-r.GetHeight()+1;
                if(topItem<0) topItem=0;
                WmsInvalidateRect(GetHwnd(), 0, False);
        }
}


void FListBox::notifyOwner(uint16 code) {
        WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                   WM_CONTROL,
                   MPFROM2UINT16(WmsQueryWindowID(GetHwnd()),code),
                   MPFROMHWND(GetHwnd())
                  );
}

int FListBox::mapPoint2Item(const FPoint& p) {
        int i=topItem+p.GetY();
        if(i<-1) i=-1;
        if(i>items) i=items;
        return i;
}

void FListBox::lm_deleteall() {
        for(unsigned i=0; i<items; i++)
                delete[] item[i].itemData1;
        delete[] item;
        item=0;
        items=allocatedItems=focusedItem=topItem=0;
        WmsInvalidateRect(GetHwnd(),0,False);
}

Bool FListBox::lm_deleteitem(int pos) {
        if(pos<0 || pos>=items) return False;
        delete[] item[pos].itemData1;
        return removeItem(pos);
}

int FListBox::lm_insertitem(const char *s, int pos) {
        if(s==0) return -1;
        ListBoxItem lbi;
        lbi.itemData1 = new char[strlen(s)+1];
        if(!lbi.itemData1) return -1;
        strcpy(lbi.itemData1,s);
        lbi.itemData2 = 0;
        lbi.itemState = 0;
        pos = insertItem(lbi,pos);
        if(pos<0) {
                delete[] lbi.itemData1;
                return -1;
        } else
                return pos;
}

int FListBox::lm_queryitemcount() {
        return items;
}

uint32 FListBox::lm_queryitemdata(int pos) {
        if(pos<0 || pos>=items)
                return 0;
        else
                return item[pos].itemData2;
}

int FListBox::lm_queryitemtext(char *buf, int buflen, int pos) {
        if(buf==0)
                return False;
        if(pos<0 || pos>=items)
                return False;
        if(buflen<1)
                return False;

        int sl=strlen(item[pos].itemData1);
        if(sl+1>buflen) {
                memcpy(buf,item[pos].itemData1,buflen-1);
                buf[buflen-1]='\0';
                return buflen-1;
        } else {
                strcpy(buf,item[pos].itemData1);
                return sl;
        }
}

int FListBox::lm_queryitemtextlen(int pos) {
        if(pos<0 || pos>=items)
                return -1;
        return strlen(item[pos].itemData1);
}

int FListBox::lm_queryselection(int start) {
        if(start==-2) return focusedItem;
        start++;
        if(start<0 || start>=items)
                return -1;
        for(int i=start; i<items; i++)
                if(item[i].itemState&OIS_SELECTED)
                        return i;
        return -1;
}

int FListBox::lm_querytopindex() {
        return topItem;
}

int FListBox::lm_searchstring(const char *s, int start, uint16 options) {
        if(s==0) return -1;
        if(start==-2) start=focusedItem-1;
        start++;
        if(start<0 || start>=items)
                return -1;
        int slen=strlen(s);
        for(int i=start; i<items; i++) {
                int ilen=strlen(item[i].itemData1);
                if(ilen<slen) continue;
                if(options&LSS_PREFIX) {
                        if(options&LSS_CASESENSITIVE) {
                                if(memcmp(item[i].itemData1,s,slen)==0)
                                        return i;
                        } else {
                                if(memicmp(item[i].itemData1,s,slen)==0)
                                        return i;
                        }
                } else if(options&LSS_SUBSTRING) {
                        //todo: case-insensitive
                        if(strstr(item[i].itemData1,s))
                                return i;
                } else {
                        //exact
                        if(slen!=ilen) continue;
                        if(options&LSS_CASESENSITIVE) {
                                if(memcmp(item[i].itemData1,s,slen)==0)
                                        return i;
                        } else {
                                if(memicmp(item[i].itemData1,s,slen)==0)
                                        return i;
                        }
                }
        }
        return -1;
}

Bool FListBox::lm_selectitem(int pos, Bool s) {
        if(pos<-1 || pos>=items) return False;
        if(pos==-1) {
                for(unsigned i=0; i<items; i++) {
                        if(s)
                                item[i].itemState |= OIS_SELECTED;
                        else
                                item[i].itemState &= ~OIS_SELECTED;
                        invalidateItem(i);
                }
                return True;
        }
        if(s) {
                if(item[pos].itemState&OIS_SELECTED) {
                        //nothing
                } else {
                        item[pos].itemState |= OIS_SELECTED;
                        invalidateItem(pos);
                }
        } else {
                if(item[pos].itemState&OIS_SELECTED) {
                        item[pos].itemState &= ~OIS_SELECTED;
                        invalidateItem(pos);
                } else {
                        //nothing
                }
        }
        return True;
}

Bool FListBox::lm_setitemdata(uint32 data, int pos) {
        if(pos<0 | pos>=items) return False;
        item[pos].itemData2 = data;
        return True;
}

Bool FListBox::lm_setitemtext(const char *s, int pos) {
        if(!s || pos<0 || pos>=items)
                return False;
        if(strcmp(item[pos].itemData1,s)==0)
                return True;
        char *d=new char[strlen(s)+1];
        if(!d) return False;
        strcpy(d,s);
        delete[] item[pos].itemData1;
        item[pos].itemData1 = d;
        invalidateItem(pos);
        return True;
}

Bool FListBox::lm_settopindex(int pos) {
        if(pos<0 || pos>=items)
                return False;
        if(pos==topItem) return True;
        topItem=pos;
        WmsInvalidateRect(GetHwnd(), 0, False);
        return True;
}


int FListBox::GetCount() {
        return (int)WmsSendMsg(GetHwnd(), LM_QUERYITEMCOUNT, 0,0);
}
        
void FListBox::Clear() {
        WmsSendMsg(GetHwnd(), LM_DELETEALL, 0,0);
}

int FListBox::GetTopIndex() {
        return (int)WmsSendMsg(GetHwnd(), LM_QUERYTOPINDEX, 0,0);
}

Bool FListBox::SetTopIndex(int index) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_SETTOPINDEX, MPFROMUINT16(index), 0);
}


void FListBox::DisableRedraw() {
        WmsEnableWindowUpdate(GetHwnd(), False);
}

void FListBox::EnableRedraw() {
        WmsSetWindowPos(GetHwnd(), (WmsHWND)0, 0,0, 0,0, SWP_SHOW);
}


Bool FListBox::LostFocus(FKillFocusEvent&) {
        if(focusedItem>=items)
                WmsInvalidateRect(GetHwnd(),0,False);
        else
                invalidateItem(focusedItem);
        notifyOwner(LN_SETFOCUS);
        return False;
}

Bool FListBox::GotFocus(FSetFocusEvent&) {
        if(focusedItem>=items)
                WmsInvalidateRect(GetHwnd(),0,False);
        else
                invalidateItem(focusedItem);
        notifyOwner(LN_KILLFOCUS);
        return False;
}

Bool FListBox::KeyPressed(FKeyboardEvent &ev) {
        if(ev.IsAltDown()) //alt is not a legal modifier
                return False;
        switch(ev.GetLogicalKey()) {
                case lk_up:
                case lk_left:
                        if(focusedItem>0)
                                focusItem(focusedItem-1);
                        break;
                case lk_down:
                case lk_right:
                        if(focusedItem<items-1)
                                focusItem(focusedItem+1);
                        break;
                case lk_pageup: {
                        FRect r; WmsQueryWindowRect(GetHwnd(),&r);
                        int nf=focusedItem-r.GetHeight()+1;
                        if(nf<0) nf=0;
                        focusItem(nf);
                        break;
                }
                case lk_pagedown: {
                        FRect r; WmsQueryWindowRect(GetHwnd(),&r);
                        int nf=focusedItem+r.GetHeight()-1;
                        if(nf>=items) nf=items-1;
                        focusItem(nf);
                        break;
                }
                case lk_home:
                        if(focusedItem!=0)
                                focusItem(0);
                        break;
                case lk_end:
                        if(focusedItem<items-1)
                                focusItem(items-1);
                        break;

                case lk_enter:
                case lk_numenter:
                        notifyOwner(LN_ENTER);
                        //fall-through
                default:
                        return False;
        }
        fixFocus();
        return True;
}

Bool FListBox::MouseOpen(FMouseEvent &ev) {
        notifyOwner(LN_ENTER);
        return FTranslatedMouseHandler::MouseOpen(ev);
}

///////////////////////////////////////////////////////////////////////////////
//FTextListBox

Bool FTextListBox::Create(FWnd *pParent, int ID, FRect *rect, Bool sorted) {
        sortItems = sorted;
        return FListBox::Create(pParent,ID,rect,0);
}


Bool FTextListBox::AddItem(const char *pszText) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_INSERTITEM, MPFROMUINT16(-1), MPFROMP(pszText));
}

Bool FTextListBox::InsertItem(int index, const char *pszText) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_INSERTITEM, MPFROMUINT16(index), MPFROMP(pszText));
}

Bool FTextListBox::GetItemText(int index, char *buffer, int maxbuf) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_QUERYITEMTEXT, MPFROM2UINT16(index,maxbuf), MPFROMP(buffer));
}

int FTextListBox::GetItemTextLen(int index) {
        return (int)WmsSendMsg(GetHwnd(), LM_QUERYITEMTEXTLEN, MPFROMUINT16(index), 0);
}

Bool FTextListBox::RemoveItem(int index) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_DELETEITEM, MPFROMUINT16(index), 0);
}

Bool FTextListBox::ReplaceItem(int index, const char *pszText) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_SETITEMTEXT, MPFROMUINT16(index), MPFROMP(pszText));
}


int FTextListBox::FindItem(const char *pszText, int startIndex) {
        return (int)WmsSendMsg(GetHwnd(), LM_SEARCHSTRING, MPFROM2UINT16(startIndex,0), MPFROMP(pszText));
}


void FTextListBox::wm_paint() {
        FRect wr;
        WmsQueryWindowRect(GetHwnd(),&wr);
        FRect pr;
        WmsHDC hdc=WmsBeginPaint(GetHwnd(), &pr);

        uint8 nfg_clr,nbg_clr;
        uint8 sfg_clr,sbg_clr;
        uint8 ffg_clr,fbg_clr;
        uint8 sffg_clr,sfbg_clr;

        WmsQueryPresParam(GetHwnd(), PP_LBNORMALFOREGROUNDCOLOR, 0, 0, &nfg_clr, 1);
        WmsQueryPresParam(GetHwnd(), PP_LBNORMALBACKGROUNDCOLOR, 0, 0, &nbg_clr, 1);
        WmsQueryPresParam(GetHwnd(), PP_LBSELECTEDFOREGROUNDCOLOR, 0, 0, &sfg_clr, 1);
        WmsQueryPresParam(GetHwnd(), PP_LBSELECTEDBACKGROUNDCOLOR, 0, 0, &sbg_clr, 1);
        WmsQueryPresParam(GetHwnd(), PP_LBFOCUSFOREGROUNDCOLOR, 0, 0, &ffg_clr, 1);
        WmsQueryPresParam(GetHwnd(), PP_LBFOCUSBACKGROUNDCOLOR, 0, 0, &fbg_clr, 1);
        WmsQueryPresParam(GetHwnd(), PP_LBSELECTEDFOCUSFOREGROUNDCOLOR, 0, 0, &sffg_clr, 1);
        WmsQueryPresParam(GetHwnd(), PP_LBSELECTEDFOCUSBACKGROUNDCOLOR, 0, 0, &sfbg_clr, 1);

        //erase background since it may contain unused space
        CpiEraseRect(hdc, pr, nbg_clr|nfg_clr);

        //draw each item
        CpiSetBkMode(hdc, bm_transparent);
        for(unsigned i=topItem+pr.GetTop(); i<topItem+pr.GetBottom() && i<items; i++) {
                unsigned bg_clr,fg_clr;
                if(item[i].itemState&OIS_SELECTED) {
                        if(i==focusedItem && HasFocus())
                                fg_clr=sffg_clr,bg_clr=sfbg_clr;
                        else
                                fg_clr=sfg_clr,bg_clr=sbg_clr;
                } else {
                        if(i==focusedItem && HasFocus())
                                fg_clr=ffg_clr,bg_clr=fbg_clr;
                        else
                                fg_clr=nfg_clr,bg_clr=nbg_clr;
                }
                FRect r(wr.GetLeft(), i-topItem, wr.GetRight(), i-topItem+1);
                CpiEraseRect(hdc, r, bg_clr|fg_clr);
                CpiSetTextColor(hdc,(color)fg_clr);
                CpiOuttext(hdc, r.GetLeft()+1, r.GetTop(), item[i].itemData1);
        }
        if(focusedItem>=items && HasFocus()) {
                //draw focus in empty listbox
                FRect r(wr.GetLeft(), 0, wr.GetRight(), 1);
                CpiEraseRect(hdc, r, fbg_clr|ffg_clr);
        }

        WmsEndPaint(hdc);
}

Bool FTextListBox::CharInput(FKeyboardEvent &ev) {
        char searchstring[2];
        searchstring[0] = ev.GetChar();
        searchstring[1] = '\0';
        int i=(int)WmsSendMsg(GetHwnd(), LM_SEARCHSTRING, MPFROM2UINT16(focusedItem,LSS_CASESENSITIVE|LSS_PREFIX), MPFROMP(searchstring));
        if(i==-1)
                i=(int)WmsSendMsg(GetHwnd(), LM_SEARCHSTRING, MPFROM2UINT16(-1,LSS_CASESENSITIVE|LSS_PREFIX), MPFROMP(searchstring));
        if(i!=-1) {
                focusItem(i);
                fixFocus();
                notifyOwner(LN_SELECT);
                return True;
        }
        return False;
}

int FTextListBox::lm_insertitem(const char *s, int pos) {
        if(sortItems) {
                //Find position with binary search
                int low=0;
                int high=items;
                while(low<high) {
                        int half = (low+high)/2;
                        int r = strcmp(s,item[half].itemData1);
                        if(r<0) {
                                high=half;
                        } else if(r==0) {
                                low=half+1;
                        } else {
                                low=half+1;
                        }
                }
                pos = low;
        }
        return FListBox::lm_insertitem(s,pos);
}

Bool FTextListBox::_ProcessResourceSetup(int props, char *name[], char **) {
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"sort")==0)
                        sortItems=True;
        }
        return True;
}

///////////////////////////////////////////////////////////////////////////////
//FSingleSelectionListBox

int FSingleSelectionListBox::GetCurrentSelection() {
        return (int)WmsSendMsg(GetHwnd(), LM_QUERYSELECTION, MPFROMUINT16(-1), 0);
}

Bool FSingleSelectionListBox::SetCurrentSelection(int index) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_SELECTITEM, MPFROMUINT16(index), MPFROMUINT16(True));
}

Bool FSingleSelectionListBox::lm_selectitem(int pos, Bool s) {
        if(s) {
                //deselect other items
                for(unsigned i=0; i<items; i++) {
                        if(i!=pos && item[i].itemState&OIS_SELECTED) {
                                item[i].itemState &= ~OIS_SELECTED;
                                invalidateItem(i);
                        }
                }
                if(pos>=0 && pos<items) focusedItem=pos;
        }
        return FListBox::lm_selectitem(pos,s);
}


Bool FSingleSelectionListBox::KeyPressed(FKeyboardEvent &ev) {
        int prevfocus=focusedItem;
        if(FListBox::KeyPressed(ev)) {
                if(prevfocus==focusedItem) return True;
                if(!ev.IsShiftDown()) {
                        lm_selectitem(prevfocus,False);
                        lm_selectitem(focusedItem,True);
                        notifyOwner(LN_SELECT);
                }
                return True;
        } else
                return False;
}

Bool FSingleSelectionListBox::CharInput(FKeyboardEvent &ev) {
        if(ev.GetChar()==' ' && focusedItem<items) {
                if(item[focusedItem].itemState&OIS_SELECTED)
                        lm_selectitem(focusedItem,False);
                else
                        lm_selectitem(focusedItem,True);
                        notifyOwner(LN_SELECT);
                return True;
        } else
                return False;
}


Bool FSingleSelectionListBox::MouseSelectStart(FMouseEvent &ev) {
        SetFocus();
        SetCapture();
        int i=mapPoint2Item(ev.GetPoint());
        if(i>=0 && i<items) {
                focusItem(i);
                lm_selectitem(i,True);
        }
        return FTranslatedMouseHandler::MouseSelectStart(ev);
}

Bool FSingleSelectionListBox::MouseMove(FMouseMoveEvent &ev) {
        if(HasCapture()) {
                int i=mapPoint2Item(ev.GetPoint());
                if(i<0) i=0;
                if(i>=items) i=items-1;
                if(i>=0 && i<items) {
                        focusItem(i);
                        fixFocus();
                        lm_selectitem(i,True);
                        notifyOwner(LN_SELECT);
                }
        }
        return FTranslatedMouseHandler::MouseMove(ev);
}

Bool FSingleSelectionListBox::MouseSelectEnd(FMouseEvent &ev) {
        if(HasCapture()) {
                ReleaseCapture();
                notifyOwner(LN_SELECT);
        }
        return FTranslatedMouseHandler::MouseSelectEnd(ev);
}

Bool FSingleSelectionListBox::MouseSingleSelect(FMouseEvent &ev) {
        int i=mapPoint2Item(ev.GetPoint());
        SetFocus();
        if(i>=0 && i<items) {
                focusItem(i);
                lm_selectitem(i,True);
                notifyOwner(LN_SELECT);
        }
        return FTranslatedMouseHandler::MouseSingleSelect(ev);
}


///////////////////////////////////////////////////////////////////////////////
//FSingleSelectionTextListBox

FSingleSelectionTextListBox::FSingleSelectionTextListBox()
  : FListBox(),
    FTextListBox(),
    FSingleSelectionListBox()
{
}

Bool FSingleSelectionTextListBox::Create(FWnd *pParent, int ID, FRect *rect, Bool sorted) {
        return FTextListBox::Create(pParent,ID,rect,sorted);
}

Bool FSingleSelectionTextListBox::CharInput(FKeyboardEvent &ev) {
        int prevfocus=focusedItem;
        if(FTextListBox::CharInput(ev)) {
                if(prevfocus!=focusedItem) {
                        lm_selectitem(focusedItem,True);
                        notifyOwner(LN_SELECT);
                }
                return True;
        } else
                return FSingleSelectionListBox::CharInput(ev);
}

Bool FSingleSelectionTextListBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FListBox::ProcessResourceSetup(props,name,value))
                return False;
        return FTextListBox::_ProcessResourceSetup(props,name,value);
}

///////////////////////////////////////////////////////////////////////////////
//FMultipleSelectionListBox
Bool FMultipleSelectionListBox::SetSelState(int index, Bool selected) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_SELECTITEM, MPFROMUINT16(index), MPFROMUINT16(selected));
}

Bool FMultipleSelectionListBox::SetSelStateAll(Bool selected) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_SELECTITEM, MPFROMUINT16(-1), MPFROMUINT16(selected));
}

int FMultipleSelectionListBox::GetSelCount() {
        int count=0;
        int i=-1;
        do {
                WmsMRESULT mr = WmsSendMsg(GetHwnd(), LM_QUERYSELECTION, MPFROMUINT16(i), 0);
                i=(int)(sint16)UINT161FROMMR(mr);
                if(i>=0)
                        count++;
        } while(i>=0);
        return count;
}

Bool FMultipleSelectionListBox::GetSelItems(int index[], int maxitem) {
        int i=-1;
        do {
                WmsMRESULT mr = WmsSendMsg(GetHwnd(), LM_QUERYSELECTION, MPFROMUINT16(i), 0);
                i=(int)(sint16)UINT161FROMMR(mr);
                if(i>=0 && maxitem>0) {
                        *index++ = i;
                        maxitem--;
                }
        } while(i>=0 && maxitem>0);
        return True;
}

Bool FMultipleSelectionListBox::KeyPressed(FKeyboardEvent &ev) {
        int prevfocus=focusedItem;
        if(FListBox::KeyPressed(ev)) {
                if(prevfocus==focusedItem) return True;
                if(isMultiple()) return True;
                //extended selection:
                if(!ev.IsShiftDown()) {
                        //set selection to focused item
                        lm_selectitem(-1,False); //unselect all
                        lm_selectitem(focusedItem,True);
                } else {
                        extendSelection(prevfocus);
                }
                notifyOwner(LN_SELECT);
                return True;
        } else
                return False;
}

void FMultipleSelectionListBox::extendSelection(int prevfocus) {
        if((item[prevfocus].itemState&OIS_SELECTED)==0)
                lm_selectitem(prevfocus,True);
        //figure out which case it is:
        // 1 extending down
        // 2 reducing down
        // 3 extending up
        // 4 reducing up

        int start,end;
        Bool s;
        if(focusedItem<prevfocus) {
                //focus has moved up
                if(item[focusedItem].itemState&OIS_SELECTED) {
                        //reducing up
                        start=focusedItem+1;
                        end=prevfocus+1;
                        s=False;
                } else {
                        //extending up
                        start=focusedItem;
                        end=prevfocus;
                        s=True;
                }
        } else {
                //focus has moved down
                if(item[focusedItem].itemState&OIS_SELECTED) {
                        //reducing down
                        start=prevfocus;
                        end=focusedItem;
                        s=False;
                } else {
                        //extending down
                        start=prevfocus+1;
                        end=focusedItem+1;
                        s=True;
                }
        }

        for(int i=start; i<end; i++)
                lm_selectitem(i,s);
}

Bool FMultipleSelectionListBox::CharInput(FKeyboardEvent &ev) {
        if(ev.GetChar()=='+' && ev.GetLogicalKey()==lk_numplus) {
                //select all items
                lm_selectitem(-1,True);
                notifyOwner(LN_SELECT);
                return True;
        }
        if(ev.GetChar()=='-' && ev.GetLogicalKey()==lk_numminus) {
                //deselect all items
                lm_selectitem(-1,False);
                notifyOwner(LN_SELECT);
                return True;
        }
        if(ev.GetChar()==' ' && focusedItem<items && isMultiple()) {
                if(item[focusedItem].itemState&OIS_SELECTED)
                        lm_selectitem(focusedItem,False);
                else
                        lm_selectitem(focusedItem,True);
                        notifyOwner(LN_SELECT);
                return True;
        }
        return False;
}

Bool FMultipleSelectionListBox::isMultiple() {
        return (Bool)((WmsQueryWindowStyle(GetHwnd())&LS_MULTIPLESEL)!=0);
}

Bool FMultipleSelectionListBox::isExtended() {
        return (Bool)((WmsQueryWindowStyle(GetHwnd())&LS_EXTENDEDSEL)!=0);
}



Bool FMultipleSelectionListBox::MouseSelectStart(FMouseEvent &ev) {
        SetFocus();
        SetCapture();
        if(isExtended()) {
                if(!ev.IsCtrlDown())
                        lm_selectitem(-1,False);
        }
        return FTranslatedMouseHandler::MouseSelectStart(ev);
}

Bool FMultipleSelectionListBox::MouseMove(FMouseMoveEvent &ev) {
        if(HasCapture()) {
                int i=mapPoint2Item(ev.GetPoint());
                if(i<0) i=0;
                if(i>=items) i=items-1;
                if(i>=0 && i<items) {
                        int prevfocus=focusedItem;
                        focusItem(i);
                        fixFocus();
                        if(isExtended())
                                extendSelection(prevfocus);
                }
        }
        return FTranslatedMouseHandler::MouseMove(ev);
}

Bool FMultipleSelectionListBox::MouseSelectEnd(FMouseEvent &ev) {
        if(HasCapture()) {
                ReleaseCapture();
                notifyOwner(LN_SELECT);
        }
        return FTranslatedMouseHandler::MouseSelectEnd(ev);
}

Bool FMultipleSelectionListBox::MouseSingleSelect(FMouseEvent &ev) {
        SetFocus();
        int i=mapPoint2Item(ev.GetPoint());
        if(isMultiple()) {
                if(i>=0 && i<items) {
                        if(item[i].itemState&OIS_SELECTED)
                                lm_selectitem(i,False);
                        else
                                lm_selectitem(i,True);
                }
                if(i>=0 && i<items) {
                        focusItem(i);
                        fixFocus();
                }
        } else {
                if(!(ev.IsCtrlDown() || ev.IsShiftDown()))
                        lm_selectitem(-1,False);
                if(i>=0 && i<items) {
                        int prevfocus=focusedItem;
                        focusItem(i);
                        fixFocus();
                        if(ev.IsShiftDown())
                                extendSelection(prevfocus);
                        else
                                lm_selectitem(i,True);
                }
        }
        notifyOwner(LN_SELECT);
        return FTranslatedMouseHandler::MouseSingleSelect(ev);
}

Bool FMultipleSelectionListBox::_ProcessResourceSetup(int props, char *name[], char **) {
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"multiple")==0) {
                        SetStyle(~LS_MULTIPLESEL,LS_MULTIPLESEL);
                } else if(strcmp(name[p],"extended")==0) {
                        SetStyle(~LS_EXTENDEDSEL,LS_EXTENDEDSEL);
                }
        }
        return True;
}

///////////////////////////////////////////////////////////////////////////////
//FMultipleSelectionTextListBox

FMultipleSelectionTextListBox::FMultipleSelectionTextListBox()
  : FListBox(),
    FTextListBox(),
    FMultipleSelectionListBox()
{
}

Bool FMultipleSelectionTextListBox::Create(FWnd *pParent, int ID, FRect *rect,
                                           lbs_multiselectstyle msstyle,
                                           Bool sorted)
{
        uint32 flStyle=0;
        switch(msstyle) {
                case lbs_me_selection:
                case lbs_multipleselection:
                        flStyle|= LS_MULTIPLESEL;
                        break;
                case lbs_extendedselection:
                        flStyle|= LS_EXTENDEDSEL;
                        break;
        }
        sortItems=sorted;
        return FListBox::Create(pParent, ID, rect, flStyle);
}

Bool FMultipleSelectionTextListBox::KeyPressed(FKeyboardEvent &ev) {
        if(FMultipleSelectionListBox::KeyPressed(ev)) return True;
        return FTextListBox::KeyPressed(ev);
}

Bool FMultipleSelectionTextListBox::CharInput(FKeyboardEvent &ev) {
        if(FMultipleSelectionListBox::CharInput(ev)) return True;
        return FTextListBox::CharInput(ev);
}

Bool FMultipleSelectionTextListBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FListBox::ProcessResourceSetup(props,name,value))
                return False;
        if(!FTextListBox::_ProcessResourceSetup(props,name,value))
                return False;
        return FMultipleSelectionListBox::_ProcessResourceSetup(props,name,value);
}

///////////////////////////////////////////////////////////////////////////////
//FODListBox
Bool FODListBox::Create(FWnd *pParent, int ID, FRect *rect) {
        return FListBox::Create(pParent,ID,rect,0);
}

Bool FODListBox::AddItem(long itemData) {
        return InsertItem(-1,itemData);
}

Bool FODListBox::InsertItem(int index, long itemData) {
        char nothing='\0';
        WmsMRESULT mr=WmsSendMsg(GetHwnd(), LM_INSERTITEM, MPFROMUINT16(-1), MPFROMP(&nothing));
        index=(int)(sint16)UINT161FROMMR(mr);
        if(index>=0) {
                WmsSendMsg(GetHwnd(), LM_SETITEMDATA, MPFROMUINT16(index), MPFROMUINT32(itemData));
                return True;
        } else
                return False;
}

Bool FODListBox::GetItemData(int index, long *itemData) {
        if(index-1 || index>GetCount())
                return False;
        *itemData = UINT32FROMMR(WmsSendMsg(GetHwnd(), LM_QUERYITEMDATA, MPFROMUINT16(index), 0));
        return True;
}

Bool FODListBox::RemoveItem(int index) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_DELETEITEM, MPFROMUINT16(index), 0);
}

Bool FODListBox::ReplaceItem(int index, long itemData) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_SETITEMDATA, MPFROMUINT16(index), MPFROMUINT32(itemData));
}

Bool FODListBox::OwneeMeasure(FMeasureItemEvent &ev) {
        ev.SetItemHeight(GetItemHeight());
        return True;
}

void FODListBox::wm_paint() {
        FRect wr;
        WmsQueryWindowRect(GetHwnd(),&wr);
        FRect pr;
        WmsHDC hdc=WmsBeginPaint(GetHwnd(), &pr);

        uint8 nfg_clr,nbg_clr;

        WmsQueryPresParam(GetHwnd(), PP_LBNORMALFOREGROUNDCOLOR, 0, 0, &nfg_clr, 1);
        WmsQueryPresParam(GetHwnd(), PP_LBNORMALBACKGROUNDCOLOR, 0, 0, &nbg_clr, 1);

        //erase background since it may contain unused space
        CpiEraseRect(hdc, pr, nbg_clr|nfg_clr);

        OWNERITEM oi;
        oi.controlID = (uint16)GetID();
        oi.controlHwnd = GetHwnd();
        oi.hdc = hdc;
        oi.itemRect.SetLeft(wr.GetLeft());
        oi.itemRect.SetRight(wr.GetRight());
        for(unsigned i=topItem+pr.GetTop(); i<topItem+pr.GetBottom() && i<items; i++) {
                oi.itemID = i;
                oi.itemRect.SetTop(i-topItem);
                oi.itemRect.SetBottom(i-topItem+1);
                oi.itemState = item[i].itemState;
                if(i==focusedItem) oi.itemState|=OIS_FOCUSED;
                oi.itemData1 = (uint32)item[i].itemData1;
                oi.itemData2 = item[i].itemData2;
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                           WM_DRAWITEM,
                           MPFROMUINT16(GetID()),
                           MPFROMP(&oi)
                          );
        }
        if(focusedItem>=items && HasFocus()) {
                //draw focus in empty listbox
                oi.itemID = -1;
                oi.itemRect.SetTop(0);
                oi.itemRect.SetBottom(1);
                oi.itemState = OIS_FOCUSED;
                oi.itemData1 = 0;
                oi.itemData2 = 0;
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()),
                           WM_DRAWITEM,
                           MPFROMUINT16(GetID()),
                           MPFROMP(&oi)
                          );
        }
        WmsEndPaint(hdc);
}



///////////////////////////////////////////////////////////////////////////////
//FSingleSelectionODListBox
FSingleSelectionODListBox::FSingleSelectionODListBox(FOwnerDrawOwner *owner)
  : FListBox(),
    FODListBox(owner),
    FSingleSelectionListBox()
{
}

Bool FSingleSelectionODListBox::Create(FWnd *pParent, int ID, FRect *rect) {
        SetID(ID);
        return FListBox::Create(pParent,ID,rect,0);
}


///////////////////////////////////////////////////////////////////////////////
//FMultipleSelectionODListBox
FMultipleSelectionODListBox::FMultipleSelectionODListBox(FOwnerDrawOwner *owner)
  : FListBox(),
    FODListBox(owner),
    FMultipleSelectionListBox()
{
}

Bool FMultipleSelectionODListBox::Create(FWnd *pParent, int ID, FRect *rect,
                                         lbs_multiselectstyle msstyle)
{
        SetID(ID);
        uint32 flStyle=0;
        switch(msstyle) {
                case lbs_me_selection:
                case lbs_multipleselection:
                        flStyle|= LS_MULTIPLESEL;
                        break;
                case lbs_extendedselection:
                        flStyle|= LS_EXTENDEDSEL;
                        break;
        }
        return FListBox::Create(pParent, ID, rect, flStyle);
}

Bool FMultipleSelectionODListBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FListBox::ProcessResourceSetup(props,name,value))
                return False;
        return FMultipleSelectionListBox::_ProcessResourceSetup(props,name,value);
}
