/*
File: notebook.cpp
Description:
        Implementation of a notebook control for win16 and win32
        Does _not_ use commctrl.dll
History:
        97-11-09 ISJ Creation
*/
#include "notebook.h"
#include <string.h>


class notebook_special_handler : public FHandler {
        FNoteBook *nb;
public:
        notebook_special_handler(FNoteBook *pwnd)
          : FHandler(pwnd),
            nb(pwnd)
        { SetDispatch(dispatchFunc_t(&notebook_special_handler::Dispatch)); }
protected:
        Bool Dispatch(FEvent &ev);
};

Bool notebook_special_handler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_SETFONT ||
           ev.GetID()==WM_GETFONT)
                return nb->HandleWM_SETGETFONT(ev);
        if(ev.GetID()==WM_GETDLGCODE)
                return nb->HandleWM_GETDLGCODE(ev);
        return False;
}





struct FNoteBook_tab {
        FNoteBook::PageId firstPage;    //first page on tab
        char *text;                     //tab text
        FRect rect;                     //rectangle of tab
};


struct FNoteBook_page {
        int tab_index;                  //index into aTab
        FNoteBook::PageId pageid;       //id of this page
        Bool major;                     //is this a major page
        char *text;                     //page text
        FWnd *wnd;                      //page window
        long data;                      //data
};


//Convenience constant
const FNoteBook::PageId FNoteBook::nopage=0;
//By default notebooks autodetect wether they should draw semi-3D (suitable for
//white dialogs) or 3D suitable for gray dialogs
int FNoteBook::drawMode=2;

FNoteBook::FNoteBook()
  : FControl(),
    FSizeHandler(this),
    FPaintHandler(this),
    FFocusHandler(this),
    FMouseHandler(this),
    aPage(0),
    pages(0),
    pageid_generator(1),
    aTab(0),
    tabs(0),
    leftScrollVisible(False), rightScrollVisible(False),
    firstVisibleTab(0),
    focusedTab(0),
    selectedPage(nopage),
    special_handler(0),
    wnd_text(0)
{
}
  
FNoteBook::~FNoteBook() {
        for(int p=0; p<pages; p++) {
                delete[] aPage[p].text;
        }
        delete[] aPage;

        for(int t=0; t<tabs; t++) {
                delete[] aTab[t].text;
        }
        delete[] aTab;
        
        delete special_handler;
        delete wnd_text;
}


Bool FNoteBook::Create(FWnd *parent, int ID) {
        //since subclassing standard controls and overriding the functionallity
        //_completely_ is a bad idea, we use the existing control as a template
        //and destroy it afterwards.
        HWND hwnd=GetDlgItem(parent->GetHwnd(),ID); //find tamplate
        if(!hwnd) return False;
        //find rectangle of template
        FRect r;
        GetClientRect(hwnd,&r);
        MapWindowPoints(hwnd,parent->GetHwnd(),(LPPOINT)&r,2);
        //create notebook
        if(Create(parent,ID,&r)) {
                //set zorder so notebook is in the same order as the template
                SetWindowPos(GetHwnd(),hwnd,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
                //grab font from template
                hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
                SendMessage(GetHwnd(),WM_SETFONT,(WPARAM)hfont,0);
                //squash template
                DestroyWindow(hwnd);
                return True;
        } else
                return False;
}

Bool FNoteBook::Create(FWnd *pParent, int ID, FRect *r) {
        if(!FControl::Create(pParent, ID,
                             0, "",
                             WS_CHILD|WS_CLIPCHILDREN|WS_TABSTOP|WS_VISIBLE,
                             r
                            ))
                return False;
        special_handler = new notebook_special_handler(this);
        createText();
        
        return True;
}


FNoteBook::PageId FNoteBook::InsertPage(PageId before, const char *name, long data) {
        int i;
        if(before==nopage)
                i=pages; //append
        else {
                i = pageid2aidx(before);
                if(i<0) return False;
        }

        FNoteBook_page *np=new FNoteBook_page[pages+1];
        if(!np) return nopage;

        //insert into aPage[]
        {
                int cp=0;
                for(; cp<i; cp++) {
                        np[cp] = aPage[cp];
                }
                np[cp].tab_index = -1; //calculated later
                np[cp].pageid = pageid_generator++;
                np[cp].major = (Bool)(name!=0);
                np[cp].text = 0;
                np[cp].wnd = 0;
                np[cp].data = data;
                for(;cp<pages; cp++) {
                        np[cp+1] = aPage[cp];
                }
        }
        delete[] aPage;
        aPage = np;
        pages++;

        //insert into aTab[]
        if(aPage[i].major) {
                FNoteBook_tab *nt=new FNoteBook_tab[tabs+1];
                int t;
                int ti = (i>0) ? aPage[i-1].tab_index + 1: 0;
                for(t=0; t<ti; t++)
                        nt[t] = aTab[t];
                nt[ti].firstPage = aPage[i].pageid;
                nt[ti].text = new char[strlen(name)+1];
                strcpy(nt[ti].text,name);
                //nt[ti].rect =
                for(; t<tabs; t++)
                        nt[t+1] = aTab[t];
                delete[] aTab;
                aTab=nt;
                tabs++;
        }
        
        recalcTabIndexes();

        FRect tabr;
        calcLayout(&tabr,0,0,0);
        calcTabs(&tabr);
        Invalidate(&tabr);
        
        return aPage[i].pageid;
}

Bool FNoteBook::DeletePage(PageId page) {
        int i=pageid2aidx(page);
        if(i<0) return False;

        if(aPage[i].major) {
                //remove from aTab[]
                int t=aPage[i].tab_index;
                if(focusedTab>=t) t--;
                delete[] aTab[t].text;
                for(; t<tabs-1; t++)
                        aTab[t] = aTab[i+1];
                tabs--;
                
                //invalidate tabs
                FRect tr;
                calcLayout(&tr,0,0,0);
                calcTabs(&tr);
                Invalidate(&tr);
        }

        //remove from aPage[]
        delete[] aPage[i].text;
        if(aPage[i].wnd && aPage[i].wnd->IsValid())
                aPage[i].wnd->Destroy();
        for(; i<pages-1; i++)
                aPage[i] = aPage[i+1];
        pages--;

        recalcTabIndexes();

        if(selectedPage==page) {
                //removing the currently selected page - reset
                focusedTab=-1;
                selectedPage=nopage;
                firstVisibleTab=0;
                Invalidate();
        }

        return True;        
}


long FNoteBook::GetPageData(PageId page) {
        int i=pageid2aidx(page);
        if(i<0)
                return 0;
        else
                return aPage[i].data;
}

Bool FNoteBook::SetPageData(PageId page, long data) {
        int i=pageid2aidx(page);
        if(i<0) {
                return False;
        } else {
                aPage[i].data = data;
                return True;
        }
}


Bool FNoteBook::SetPageText(PageId page, const char *text) {
        int i=pageid2aidx(page);
        if(i<0)
                return False;
        delete[] aPage[i].text;
        if(text) {
                int sl=strlen(text);
                aPage[i].text = new char[sl+1];
                if(!aPage[i].text)
                        return False;
                memcpy(aPage[i].text,text,sl+1);
                if(page==selectedPage)
                        updateText();
                return True;
        } else
                aPage[i].text=0;
        return True;
}

Bool FNoteBook::GetPageText(PageId page, char *buf, int maxbuf) {
        int i=pageid2aidx(page);
        if(i<0)
                return False;
        if(!aPage[i].text)
                return False;
        strncpy(buf,aPage[i].text,maxbuf);
        buf[maxbuf-1] = '\0';
        return True;
}


FNoteBook::PageId FNoteBook::GetCurrentPage() {
        return selectedPage;
}

Bool FNoteBook::FlipTo(PageId page) {
        if(page==selectedPage) return True;

        int i=pageid2aidx(page);
        if(i<0) return False;

        if(!loadpage(page))
                return False;

        int si=pageid2aidx(selectedPage);

        FRect tabr;
        calcLayout(&tabr,0,0,0);
        calcTabs(&tabr);
        Invalidate(&tabr);

        selectedPage=page;
        if(i>=0) focusedTab=aPage[i].tab_index;
        makeTabVisible(focusedTab);
        showpage(page,2);
        Flipped(page);
        return True;
}

static Bool isFirstTab(HWND hwnd) {
        //is first control with w_tabstop?
        hwnd=GetWindow(hwnd,GW_HWNDPREV);
        while(hwnd) {
                LONG lStyle = GetWindowLong(hwnd,GWL_STYLE);
                if((lStyle&WS_TABSTOP) &&
                   (lStyle&WS_VISIBLE) &&
                   (lStyle&WS_DISABLED)==0)
                        return False;
                hwnd=GetWindow(hwnd,GW_HWNDPREV);
        }
        return True;
}

static Bool isLastTab(HWND hwnd) {
        //is last control with w_tabstop?
        hwnd=GetWindow(hwnd,GW_HWNDNEXT);
        while(hwnd) {
                LONG lStyle = GetWindowLong(hwnd,GWL_STYLE);
                if((lStyle&WS_TABSTOP) &&
                   (lStyle&WS_VISIBLE) &&
                   (lStyle&WS_DISABLED)==0)
                        return False;
                hwnd=GetWindow(hwnd,GW_HWNDNEXT);
        }
        return True;
}

Bool FNoteBook::PumpEvent(FEvent &ev) {
        if(ev.GetID()!=WM_KEYDOWN)
                return False;
        //this code is complicated
        //  - and only half of it is Microsoft's fault :-)

        PageId currentPage = GetCurrentPage();
        PageId newPage=currentPage;
        
        int virtkey=(int)ev.GetWParam();
        int shiftDown = (GetKeyState(VK_SHIFT)&0x8000U)!=0;
        int ctrlDown = (GetKeyState(VK_CONTROL)&0x8000U)!=0;
        int altDown = (GetKeyState(VK_MENU)&0x8000U)!=0;
        if(virtkey==VK_TAB) {
                if(!(ctrlDown || altDown)) {
                        //possible special processing if tabbing/shift-tabbing out of 
                        if(shiftDown) {
                                //back-tab
                                if(ev.GetHwnd()==GetHwnd())
                                        return False; //let default thing happen
                                else if(IsChild(GetHwnd(),GetFocus())) {
                                        if(isFirstTab(ev.GetHwnd())) {
                                                //backtab to notebook
                                                ::SetFocus(GetHwnd());
                                                return True;
                                        } else
                                                return False; //let default thing happen
                                }
                        } else {
                                //forward-tab
                                if(ev.GetHwnd()==GetHwnd()) {
                                        //focus to current page
                                        showpage(GetCurrentPage(),1);
                                        return True;
                                } else if(IsChild(GetHwnd(),GetFocus())) {
                                        if(isLastTab(ev.GetHwnd())) {
                                                //focus to next tabcontrol
                                                HWND nextTabHwnd=GetNextDlgTabItem(GetParent()->GetHwnd(),GetHwnd(),FALSE);
                                                if(nextTabHwnd)
                                                        ::SetFocus(nextTabHwnd);
                                                return True;
                                        } else
                                                return False; //let default thing happen
                                }
                        }
                } else if(ctrlDown && !altDown) {
                        //prev/next tab
                        int i=pageid2aidx(currentPage);
                        if(i<0) return False; //panic - let default thing happen
                        if(shiftDown) {
                                //prev tab
                                i--;
                                while(i>=0 && !aPage[i].major)
                                        i--;
                                if(i>=0)
                                        newPage = aPage[i].pageid;
                        } else {
                                //next tab
                                i++;
                                while(i<pages && !aPage[i].major)
                                        i++;
                                if(i<pages)
                                        newPage = aPage[i].pageid;
                        }
                }
        } else if(virtkey==VK_PRIOR) {
                if(!(shiftDown || ctrlDown || altDown)) {
                        //prev page
                        int i=pageid2aidx(GetCurrentPage());
                        if(i<0) return False; //panic - let default thing happen
                        if(i>0)
                                newPage = aPage[i-1].pageid;
                }
        } else if(virtkey==VK_NEXT) {
                if(!(shiftDown || ctrlDown || altDown)) {
                        //next page
                        int i=pageid2aidx(GetCurrentPage());
                        if(i<0) return False; //panic - let default thing happen
                        if(i<pages-1)
                                newPage = aPage[i+1].pageid;
                }
        } else if(virtkey==VK_LEFT) {
                if(::GetFocus()==GetHwnd() && !(shiftDown || ctrlDown || altDown)) {
                        //prev tab
                        if(focusedTab>0) {
                                invalidateTab(focusedTab);
                                focusedTab--;
                                invalidateTab(focusedTab);
                                makeTabVisible(focusedTab);
                        }
                        return True;
                }
        } else if(virtkey==VK_RIGHT) {
                if(::GetFocus()==GetHwnd() && !(shiftDown || ctrlDown || altDown)) {
                        //next page
                        if(focusedTab<tabs-1) {
                                invalidateTab(focusedTab);
                                focusedTab++;
                                invalidateTab(focusedTab);
                                makeTabVisible(focusedTab);
                        }
                        return True;
                }
        } else if(virtkey==VK_SPACE) {
                if(::GetFocus()==GetHwnd() && !(shiftDown || ctrlDown || altDown)) {
                        if(focusedTab>=0)
                                newPage = aTab[focusedTab].firstPage;
                }
        }
        if(newPage!=currentPage) {
                if(Flipping(currentPage,newPage))
                        FlipTo(newPage);
                return True; //eat event
        } else
                return False;
}


Bool FNoteBook::SizeChanged(FSizeEvent &) {
        layout();
        return False;
}


/*
//pure win standard colors:
#define TABCOLORS 7
static const COLORREF tabColor[TABCOLORS] = {
        RGB(  0,255,255),  //light blue
        RGB(  0,255,  0),  //light green
        RGB(  0,128,  0),  //dark green
        RGB(128,128,  0),  //dark yellow/brown
        RGB(255,255,128),  //light yellow
        RGB(255,  0,255),  //yucky pink
        RGB(255,255,  0)   //yellow
};
*/
//impure colors:
#define TABCOLORS 7
static const COLORREF tabColor[TABCOLORS] = {
        RGB(  0,255,255),  //light blue
        RGB(128,255,128),  //light green
        RGB(  0,128,128),  //dark green/cyan
        RGB(128,128,  0),  //dark yellow/brown
        RGB(255,255,128),  //light yellow
        RGB(255,  0,255),  //yucky pink
        RGB(255,255,  0)   //yellow
};

Bool FNoteBook::Paint(FPaintEvent&) {
        int dm = detectDrawMode();
        PAINTSTRUCT ps;
        if(!BeginPaint(GetHwnd(),&ps)) return False;

        int sdc=SaveDC(ps.hdc);

        if(hfont) SelectObject(ps.hdc,hfont);
        
        FRect r,tabrect,innerrect,pagerect;
        GetClientRect(GetHwnd(),&r);
        calcLayout(&tabrect,&innerrect,0,&pagerect);

        //clear background
        if(dm==0) {
                FillRect(ps.hdc, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
        } else {
                FillRect(ps.hdc, &r, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
        }
        
        //paint border
        {
                FRect ir(r.GetLeft(),tabrect.GetBottom()-1,r.GetRight(),r.GetBottom());
                if(dm==0) {
                        SelectObject(ps.hdc, GetStockObject(BLACK_PEN));
                        Rectangle(ps.hdc, ir.GetLeft(),ir.GetTop(),ir.GetRight(),ir.GetBottom());
                } else {
                        //3D
                        //outer border (raised)
                        FRect lr;
                        lr=FRect(ir.GetLeft(),ir.GetTop(),ir.GetLeft()+1,ir.GetBottom());
                        FillRect(ps.hdc, &lr, (HBRUSH)GetStockObject(WHITE_BRUSH));
                        lr=FRect(ir.GetLeft(),ir.GetTop(),ir.GetRight(),ir.GetTop()+1);
                        FillRect(ps.hdc, &lr, (HBRUSH)GetStockObject(WHITE_BRUSH));

                        lr=FRect(ir.GetLeft()+1,ir.GetBottom()-1,ir.GetRight(),ir.GetBottom());
                        FillRect(ps.hdc, &lr, (HBRUSH)GetStockObject(GRAY_BRUSH));
                        lr=FRect(ir.GetRight()-1,ir.GetTop()+1,ir.GetRight(),ir.GetBottom());
                        FillRect(ps.hdc, &lr, (HBRUSH)GetStockObject(GRAY_BRUSH));

                        //inner border (sunken)
                        lr=FRect(innerrect.GetLeft()-1,innerrect.GetTop()-1,innerrect.GetLeft(),innerrect.GetBottom()+1);
                        FillRect(ps.hdc, &lr, (HBRUSH)GetStockObject(GRAY_BRUSH));
                        lr=FRect(innerrect.GetLeft()-1,innerrect.GetTop()-1,innerrect.GetRight()+1,innerrect.GetTop());
                        FillRect(ps.hdc, &lr, (HBRUSH)GetStockObject(GRAY_BRUSH));

                        lr=FRect(innerrect.GetLeft(),innerrect.GetBottom()+1,innerrect.GetRight()+2,innerrect.GetBottom()+2);
                        FillRect(ps.hdc, &lr, (HBRUSH)GetStockObject(WHITE_BRUSH));
                        lr=FRect(innerrect.GetRight()+1,innerrect.GetTop(),innerrect.GetRight()+2,innerrect.GetBottom()+2);
                        FillRect(ps.hdc, &lr, (HBRUSH)GetStockObject(WHITE_BRUSH));
                }
        }

        //paint tabs
        {
                int selectedTab=-1;
                if(selectedPage!=nopage)
                        selectedTab=aPage[pageid2aidx(selectedPage)].tab_index;
                for(int t=firstVisibleTab; t<tabs && !aTab[t].rect.IsEmpty(); t++) {
                        if(t==selectedTab) continue; //painted last
                        FRect tr=aTab[t].rect;
                        if(dm==0) {
                                //border
                                SelectObject(ps.hdc,GetStockObject(BLACK_PEN));
                                Rectangle(ps.hdc, tr.GetLeft(), tr.GetTop()+2, tr.GetRight(), tr.GetBottom());
                                //interior
                                HBRUSH hbr=CreateSolidBrush(tabColor[t%TABCOLORS]);
                                FRect ir(tr.GetLeft()+1,tr.GetTop()+3,tr.GetRight()-1,tr.GetBottom()-1);
                                FillRect(ps.hdc, &ir, hbr);
                                DeleteObject(hbr);
                                //tab text
                                SetTextColor(ps.hdc,RGB(0,0,0));
                                SetBkMode(ps.hdc,TRANSPARENT);
                                TextOut(ps.hdc, tr.GetLeft()+4,tr.GetTop()+6,aTab[t].text,strlen(aTab[t].text));
                                //focus
                                if(focusedTab==t && ::GetFocus()==GetHwnd()) {
                                        SetTextColor(ps.hdc,RGB(0,0,0));
                                        SetBkColor(ps.hdc,RGB(255,255,255));
                                        FRect fr(tr.GetLeft()+3,tr.GetTop()+5,tr.GetRight()-3,tr.GetBottom()-3);
                                        DrawFocusRect(ps.hdc, &fr);
                                }
                        } else {
                                //interior
                                HBRUSH hbr=CreateSolidBrush(tabColor[t%TABCOLORS]);
                                SelectObject(ps.hdc,GetStockObject(NULL_PEN));
                                SelectObject(ps.hdc, hbr);
                                FPoint ip[5];
                                ip[0].Set(tr.GetLeft()-tr.GetHeight()/4, tr.GetBottom()-1);
                                ip[1].Set(tr.GetLeft()+tr.GetHeight()/4, tr.GetTop());
                                ip[2].Set(tr.GetRight()-tr.GetHeight()/4, tr.GetTop());
                                ip[3].Set(tr.GetRight()+tr.GetHeight()/4, tr.GetBottom()-2);
                                ip[4].Set(tr.GetLeft()-tr.GetHeight()/4, tr.GetBottom()-1);
                                Polygon(ps.hdc, ip, 4);
                                SelectObject(ps.hdc, GetStockObject(NULL_BRUSH));
                                DeleteObject(hbr);
                                
                                //border
                                HPEN gray_pen=CreatePen(PS_SOLID,1,RGB(128,128,128));
                                SelectObject(ps.hdc,GetStockObject(WHITE_PEN));
                                MoveToEx(ps.hdc, tr.GetLeft()-tr.GetHeight()/4, tr.GetBottom()-1,0);
                                LineTo(ps.hdc, tr.GetLeft()+tr.GetHeight()/4, tr.GetTop());
                                LineTo(ps.hdc, tr.GetRight()-tr.GetHeight()/4, tr.GetTop());
                                SelectObject(ps.hdc,gray_pen);
                                LineTo(ps.hdc, tr.GetRight()+tr.GetHeight()/4, tr.GetBottom()-1);
                                SelectObject(ps.hdc,GetStockObject(NULL_PEN));
                                DeleteObject(gray_pen);
                                
                                //tab text
                                SetTextColor(ps.hdc,RGB(0,0,0));
                                SetBkMode(ps.hdc,TRANSPARENT);
                                TextOut(ps.hdc, tr.GetLeft()+6,tr.GetTop()+6,aTab[t].text,strlen(aTab[t].text));
                                
                                //focus
                                if(focusedTab==t && ::GetFocus()==GetHwnd()) {
                                        SetTextColor(ps.hdc,RGB(0,0,0));
                                        SetBkColor(ps.hdc,RGB(255,255,255));
                                        FRect fr(tr.GetLeft()+3,tr.GetTop()+5,tr.GetRight()-3,tr.GetBottom()-3);
                                        if(t==selectedTab) {
                                                fr.SetTop(fr.GetTop()-1);
                                                fr.SetBottom(fr.GetBottom()-1);
                                        }
                                        DrawFocusRect(ps.hdc, &fr);
                                }
                        }
                }
                if(selectedTab>=0 && !aTab[selectedTab].rect.IsEmpty()) {
                        FRect tr=aTab[selectedTab].rect;
                        if(dm==0) {
                                //border
                                FRect br;
                                br=FRect(tr.GetLeft(),tr.GetTop(),tr.GetLeft()+1,tr.GetBottom());
                                FillRect(ps.hdc, &br, (HBRUSH)GetStockObject(BLACK_BRUSH));
                                br=FRect(tr.GetLeft(),tr.GetTop(),tr.GetRight(),tr.GetTop()+1);
                                FillRect(ps.hdc, &br, (HBRUSH)GetStockObject(BLACK_BRUSH));
                                br=FRect(tr.GetRight()-1,tr.GetTop(),tr.GetRight(),tr.GetBottom());
                                FillRect(ps.hdc, &br, (HBRUSH)GetStockObject(BLACK_BRUSH));
                                //interior
                                HBRUSH hbr=CreateSolidBrush(tabColor[selectedTab%TABCOLORS]);
                                FRect ir(tr.GetLeft()+1,tr.GetTop()+1,tr.GetRight()-1,tr.GetBottom());
                                FillRect(ps.hdc, &ir, hbr);
                                DeleteObject(hbr);
                                //tab text
                                SetTextColor(ps.hdc,RGB(0,0,0));
                                SetBkMode(ps.hdc,TRANSPARENT);
                                TextOut(ps.hdc, tr.GetLeft()+4,tr.GetTop()+5,aTab[selectedTab].text,strlen(aTab[selectedTab].text));
                                //focus
                                if(focusedTab==selectedTab && ::GetFocus()==GetHwnd()) {
                                        SetTextColor(ps.hdc,RGB(0,0,0));
                                        SetBkColor(ps.hdc,RGB(255,255,255));
                                        FRect fr(tr.GetLeft()+5,tr.GetTop()+5,tr.GetRight()-5,tr.GetBottom()-3);
                                        DrawFocusRect(ps.hdc, &fr);
                                }
                        } else {
                                //interior
                                HBRUSH hbr=CreateSolidBrush(tabColor[selectedTab%TABCOLORS]);
                                SelectObject(ps.hdc,GetStockObject(NULL_PEN));
                                SelectObject(ps.hdc, hbr);
                                FPoint ip[5];
                                ip[0].Set(tr.GetLeft()-tr.GetHeight()/4, tr.GetBottom()-1);
                                ip[1].Set(tr.GetLeft()+tr.GetHeight()/4, tr.GetTop());
                                ip[2].Set(tr.GetRight()-tr.GetHeight()/4, tr.GetTop());
                                ip[3].Set(tr.GetRight()+tr.GetHeight()/4, tr.GetBottom()-2);
                                ip[4].Set(tr.GetLeft()-tr.GetHeight()/4, tr.GetBottom()-1);
                                Polygon(ps.hdc, ip, 4);
                                SelectObject(ps.hdc, GetStockObject(NULL_BRUSH));
                                DeleteObject(hbr);
                                
                                //border
                                HPEN gray_pen=CreatePen(PS_SOLID,1,RGB(128,128,128));
                                SelectObject(ps.hdc,GetStockObject(WHITE_PEN));
                                MoveToEx(ps.hdc, tr.GetLeft()-tr.GetHeight()/4, tr.GetBottom()-1,0);
                                LineTo(ps.hdc, tr.GetLeft()+tr.GetHeight()/4, tr.GetTop());
                                LineTo(ps.hdc, tr.GetRight()-tr.GetHeight()/4, tr.GetTop());
                                SelectObject(ps.hdc,gray_pen);
                                LineTo(ps.hdc, tr.GetRight()+tr.GetHeight()/4, tr.GetBottom()-1);
                                SelectObject(ps.hdc,GetStockObject(NULL_PEN));
                                DeleteObject(gray_pen);
                                
                                //tab text
                                SetTextColor(ps.hdc,RGB(0,0,0));
                                SetBkMode(ps.hdc,TRANSPARENT);
                                TextOut(ps.hdc, tr.GetLeft()+6,tr.GetTop()+6,aTab[selectedTab].text,strlen(aTab[selectedTab].text));
                                
                                //focus
                                if(focusedTab==selectedTab && ::GetFocus()==GetHwnd()) {
                                        SetTextColor(ps.hdc,RGB(0,0,0));
                                        SetBkColor(ps.hdc,RGB(255,255,255));
                                        FRect fr(tr.GetLeft()+3,tr.GetTop()+5,tr.GetRight()-3,tr.GetBottom()-3);
                                        DrawFocusRect(ps.hdc, &fr);
                                }
                        }
                }
        }

        //paint tabscroll
        if(leftScrollVisible || rightScrollVisible) {
                HGDIOBJ oldpen=SelectObject(ps.hdc,GetStockObject(NULL_PEN));
                HBRUSH hbr=CreateSolidBrush(RGB(0,0,255));
                HGDIOBJ oldbrush=SelectObject(ps.hdc,hbr);
                if(leftScrollVisible) {
                        FRect sr(tabrect.GetLeft(),tabrect.GetTop(),tabrect.GetLeft()+tabrect.GetHeight(),tabrect.GetBottom());
                        FPoint p[4];
                        p[0].Set(sr.GetRight()-4,sr.GetTop()+3);
                        p[1].Set(sr.GetLeft()+3,sr.GetTop()+sr.GetHeight()/2);
                        p[2].Set(sr.GetLeft()+3,sr.GetBottom()-sr.GetHeight()/2);
                        p[3].Set(sr.GetRight()-4,sr.GetBottom()-4);
                        Polygon(ps.hdc, (POINT*)p, 4);
                }
                if(rightScrollVisible) {
                        FRect sr(tabrect.GetRight()-tabrect.GetHeight(),tabrect.GetTop(),tabrect.GetRight(),tabrect.GetBottom());
                        FPoint p[4];
                        p[0].Set(sr.GetLeft()+3,sr.GetTop()+3);
                        p[1].Set(sr.GetRight()-4,sr.GetTop()+sr.GetHeight()/2);
                        p[2].Set(sr.GetRight()-4,sr.GetBottom()-sr.GetHeight()/2);
                        p[3].Set(sr.GetLeft()+3,sr.GetBottom()-4);
                        Polygon(ps.hdc, (POINT*)p, 4);
                }
                SelectObject(ps.hdc,oldbrush);
                SelectObject(ps.hdc,oldpen);
                DeleteObject(hbr);
        }

        RestoreDC(ps.hdc,sdc);
        EndPaint(GetHwnd(),&ps);
        return True;
}


Bool FNoteBook::LostFocus(FKillFocusEvent&) {
        invalidateTab(focusedTab);
        return False;
}

Bool FNoteBook::GotFocus(FSetFocusEvent&) {
        invalidateTab(focusedTab);
        return False;
}


Bool FNoteBook::Button1Down(FButtonDownEvent &ev) {
        for(int t=0; t<tabs; t++)
                if(aTab[t].rect.Contains(ev.GetPoint()))
                        break;
        if(t<tabs) {
                //user clicked on a tab
                if(aTab[t].firstPage==selectedPage) {
                        if(focusedTab!=t) {
                                invalidateTab(focusedTab);
                                focusedTab=t;
                                invalidateTab(focusedTab);
                        }
                        if(::GetFocus()!=GetHwnd())
                                ::SetFocus(GetHwnd());
                        return True;
                }
                if(Flipping(selectedPage,aTab[t].firstPage)) {
                        FlipTo(aTab[t].firstPage);
                        return True;
                }
        }
        if(leftScrollVisible || rightScrollVisible) {
                FRect tabrect;
                calcLayout(&tabrect,0,0,0);
                FRect ls(tabrect.GetLeft(),tabrect.GetTop(),tabrect.GetLeft()+tabrect.GetHeight(),tabrect.GetBottom());
                FRect rs(tabrect.GetRight()-tabrect.GetHeight(),tabrect.GetTop(),tabrect.GetRight(),tabrect.GetBottom());
                if(leftScrollVisible && ls.Contains(ev.GetPoint()) && firstVisibleTab>0) {
                        //scroll left
                        firstVisibleTab--;
                        calcTabs(&tabrect);
                        Invalidate(&tabrect);
                        return True;
                }
                if(rightScrollVisible && rs.Contains(ev.GetPoint()) && firstVisibleTab<tabs-1) {
                        //scroll right
                        firstVisibleTab++;
                        calcTabs(&tabrect);
                        Invalidate(&tabrect);
                        return True;
                }
        }
        return False;
}


int FNoteBook::pageid2aidx(PageId page) {
        for(int i=0; i<pages; i++)
                if(aPage[i].pageid==page)
                        return i;
        return -1;
}

Bool FNoteBook::loadpage(PageId page) {
        int i=pageid2aidx(page);
        if(i<0) return False;
        if(aPage[i].wnd) return True;
        aPage[i].wnd = LoadPage(page,this);
        if(aPage[i].wnd) {
                //force page parent to be us (FDialog may have overridden the specified parent
                ::SetParent(aPage[i].wnd->GetHwnd(),GetHwnd());
                //calc & set pos&size
                FRect pr;
                calcLayout(0,0,0,&pr);
                aPage[i].wnd->Hide();
                ::SetWindowPos(aPage[i].wnd->GetHwnd(),
                               HWND_TOP, 
                               pr.GetLeft(), pr.GetTop(),
                               pr.GetWidth(), pr.GetHeight(),
                               0
                              );
                //force ws_tabstop style on page (otherwise the user will not be able to tab it)
                aPage[i].wnd->SetStyle(~0,WS_TABSTOP);
                //re-install notebook as pump window, since we need special
                //processing of tab/shift-tab in child dialogs
                GetWndMan()->RemovePumpWindow(this);
                GetWndMan()->AddPumpWindow(this);

                return True;
        } else
                return False;
}

Bool FNoteBook::showpage(PageId page, int setfocus) {
        int i=pageid2aidx(page);
        int si=pageid2aidx(selectedPage);

        if(i>=0 && (!aPage[i].wnd) && (!loadpage(page)))
                return False;
        
        //change tab
        if(i!=si) {
                FRect tabr;
                calcLayout(&tabr,0,0,0);
                calcTabs(&tabr);
                Invalidate(&tabr);
                makeTabVisible(aPage[i].tab_index);
                focusedTab = aPage[i].tab_index;
        }

        if(setfocus==2)
        {
                //guess setfocus
                HWND focus=GetFocus();
                if(focus==GetHwnd() ||
                   ::IsChild(GetHwnd(),focus))
                        setfocus=1;
                else
                        setfocus=0;
        }

        //hide other pages and show the wanted one
        {
                for(int ui=0; ui<pages; ui++) {
                        if(ui==i)
                                aPage[ui].wnd->Show();
                        else
                                if(aPage[ui].wnd)
                                        aPage[ui].wnd->Hide();
                }
        }

        updateText();

        //set focus if needed        
        if(setfocus && i>=0 && aPage[i].wnd->IsValid())
                ::SetFocus(aPage[i].wnd->GetHwnd());

        return True;        
}


Bool FNoteBook::HandleWM_SETGETFONT(FEvent &ev) {
        if(ev.GetID()==WM_SETFONT) {
                hfont = (HFONT)ev.GetWParam();
                if(wnd_text) //tell wnd_text about it too
                        SendMessage(wnd_text->GetHwnd(), WM_SETFONT, (WPARAM)hfont, MAKELPARAM(FALSE,0));
                layout();
                return False;
        }
        if(ev.GetID()==WM_GETFONT) {
                ev.SetResult((LRESULT)hfont);
                return True;
        }
        return False;
}

Bool FNoteBook::HandleWM_GETDLGCODE(FEvent &ev) {
        //we do not act like any other control
        //so DLGC_WANTARROWS is the best we can do
        ev.SetResult((LRESULT)DLGC_WANTARROWS);
        return True;
}


void FNoteBook::createText() {
        FControl *w= new FControl;
        if(!w) return;

        FRect tr;
        calcLayout(0, 0, &tr, 0);
        if(!w->Create(this,
                      -1,
                      "STATIC",
                      "",
                      WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|SS_LEFTNOWORDWRAP|SS_NOPREFIX,
                      &tr
                     ))
        {
                delete w;
                return;
        }
        if(hfont) {
                SendMessage(w->GetHwnd(), WM_SETFONT, (WPARAM)hfont, MAKELPARAM(FALSE,0));
        }
        wnd_text = w;
}

void FNoteBook::updateText() {
        if(!wnd_text) return;
        int i=pageid2aidx(selectedPage);
        if(i>=0)
                SendMessage(wnd_text->GetHwnd(), WM_SETTEXT, 0, (LPARAM)(LPSTR)aPage[i].text);
}


void FNoteBook::calcLayout(FRect *tabrect, FRect *innerrect, FRect *textrect, FRect *pagerect) {
        FRect r;
        GetClientRect(GetHwnd(),&r);

        FRect tmp1,tmp2,tmp3,tmp4;
        if(!tabrect) tabrect=&tmp1;
        if(!innerrect) innerrect=&tmp2;
        if(!textrect) textrect=&tmp3;
        if(!pagerect) pagerect=&tmp4;
        
        
        int tabtextheight;
        {
                //measure text height
                HDC hdc=GetDC(GetHwnd());
                if(!hdc) return;
                HGDIOBJ oldobj;
                if(hfont) oldobj=SelectObject(hdc,hfont);

                SIZE size;
#if BIFOS_==BIFOS_WIN32_
                GetTextExtentPoint32(hdc, (LPSTR)"x", 1, &size);
#else
                GetTextExtentPoint(hdc, (LPSTR)"x", 1, &size);
#endif
                tabtextheight = (int)size.cy;

                if(hfont) SelectObject(hdc,oldobj);
                ReleaseDC(GetHwnd(),hdc);
        }

        int dm = detectDrawMode();

        int tabbottom;
        if(dm==0)
                //semi-3D
                tabbottom = r.GetTop() + 2 + 4 + tabtextheight + 4;
        else    //3D
                tabbottom = r.GetTop() + 4 + tabtextheight + 4;
        *tabrect = FRect(r.GetLeft(), r.GetTop(), r.GetRight(), tabbottom);

        //innerrect
        if(dm==0) {
                *innerrect = FRect(r.GetLeft()+ 1, tabrect->GetBottom()+ 0, r.GetRight()- 1, r.GetBottom()- 1);
        } else {
                *innerrect = FRect(r.GetLeft()+13, tabrect->GetBottom()+13, r.GetRight()-13, r.GetBottom()-13);
        }
        
        //textrect
        textrect->SetLeft(innerrect->GetLeft());
        textrect->SetRight(innerrect->GetRight());
        textrect->SetTop(innerrect->GetTop());
        textrect->SetBottom(textrect->GetTop()+tabtextheight);

        //pagerect
        *pagerect = FRect(innerrect->GetLeft(), textrect->GetBottom(),
                          innerrect->GetRight(), innerrect->GetBottom()
                         );
}

void FNoteBook::calcTabs(const FRect *tabrect) {
        int left=tabrect->GetLeft();
        int right=tabrect->GetRight();

        const int scrollWidth=tabrect->GetHeight();
        
        if(firstVisibleTab>0)
                leftScrollVisible=True;
        else
                leftScrollVisible=False;
        
        if(leftScrollVisible)
                left += scrollWidth;

        HDC hdc=GetDC(GetHwnd());
        if(!hdc) return;
        HGDIOBJ oldobj;
        if(hfont) oldobj=SelectObject(hdc,hfont);
        
        int t=0;
        //squash tabs before
        if(firstVisibleTab>=0) {
                //squash tabs before
                for(; t<firstVisibleTab; t++)
                        aTab[t].rect = FRect(0,0,0,0);
        }

        int dm = detectDrawMode();
        if(dm==1) {
                left += tabrect->GetHeight()/4;
                right -= tabrect->GetHeight()/4;
        }
        
        //calc rect of tabs
        while(left<right && t<tabs) {
                int tabtextwidth;
                {
                        SIZE size;
#if BIFOS_==BIFOS_WIN32_
                        GetTextExtentPoint32(hdc, (LPSTR)aTab[t].text, strlen(aTab[t].text), &size);
#else
                        GetTextExtentPoint(hdc, (LPSTR)aTab[t].text, strlen(aTab[t].text), &size);
#endif
                        tabtextwidth = (int)size.cx;
                }
                int tabright;
                if(dm==0)
                        tabright = left + 4 + tabtextwidth + 4;
                else
                        tabright = left + 6 + tabtextwidth + 6;
                if(tabright>right) break;
                if(t<tabs-1 && tabright+scrollWidth>right)
                        break;

                aTab[t].rect = FRect(left, tabrect->GetTop(), tabright, tabrect->GetBottom());
                left = tabright;
                t++;
        }

        if(t<tabs)
                rightScrollVisible = True;
        else
                rightScrollVisible = False;

        //squash rest
        for(; t<tabs; t++)
                aTab[t].rect = FRect(0,0,0,0);
        
        if(hfont) SelectObject(hdc,oldobj);
        ReleaseDC(GetHwnd(),hdc);
}

void FNoteBook::layout() {
        FRect tabr,textr,pager;
        calcLayout(&tabr,0,&textr,&pager);
        if(wnd_text)
                SetWindowPos(wnd_text->GetHwnd(),NULL,textr.GetLeft(),textr.GetTop(),textr.GetWidth(),textr.GetHeight(),SWP_NOZORDER);
        calcTabs(&tabr);
        Invalidate();
        for(int i=0; i<pages; i++) {
                if(aPage[i].wnd)
                        SetWindowPos(aPage[i].wnd->GetHwnd(), NULL,
                                     pager.GetLeft(), pager.GetTop(),
                                     pager.GetWidth(), pager.GetHeight(),
                                     SWP_NOZORDER
                                    );
        }
}


int FNoteBook::detectDrawMode() {
        if(drawMode!=2) return drawMode;
        //find out by sending paren a wm_ctlcolor
        //and look at the text background color
        HDC hdc=GetDC(GetHwnd());
        if(!hdc) return 0;
        int sdc=SaveDC(hdc);
#if BIFOS_==BIFOS_WIN32_
        SendMessage(::GetParent(GetHwnd()), WM_CTLCOLORSTATIC, (WPARAM)hdc, (LPARAM)GetHwnd());
#else
        SendMessage(::GetParent(GetHwnd()), WM_CTLCOLOR, (WPARAM)hdc, MAKELPARAM(GetHwnd(),CTLCOLOR_STATIC));
#endif
        COLORREF rgb=GetBkColor(hdc);
        RestoreDC(hdc,sdc);
        ReleaseDC(GetHwnd(),hdc);
        if(rgb==RGB(255,255,255))
                return 0; //white background = semi-3D
        else
                return 1; //non-white = 
}

void FNoteBook::makeTabVisible(int t) {
        if(t<0 || t>=tabs)
                return;
        if(!aTab[t].rect.IsEmpty())
                return;
        if(t==firstVisibleTab-1) {
                //scroll left
                firstVisibleTab--;
        } else if(t>0 && !aTab[t-1].rect.IsEmpty()) {
                //scroll right
                firstVisibleTab++;
                FRect tr;
                calcLayout(&tr,0,0,0);
                Invalidate(&tr);
                calcTabs(&tr);
                if(aTab[t].rect.IsEmpty() && firstVisibleTab<tabs-1)
                        firstVisibleTab++;
        } else {
                //reset
                firstVisibleTab=t;
        }
        FRect tr;
        calcLayout(&tr,0,0,0);
        Invalidate(&tr);
        calcTabs(&tr);
}

void FNoteBook::recalcTabIndexes() {
        {
                int ti=0;
                for(int tu=0; tu<pages; tu++) {
                        if(aPage[tu].major) {
                                if(tu>0)
                                        ti++;
                                aTab[ti].firstPage = aPage[tu].pageid;
                        }
                        aPage[tu].tab_index = ti;
                }
        }
}

void FNoteBook::invalidateTab(int t) {
        if(t>=0 && t<tabs)
                Invalidate(&aTab[t].rect);
}


