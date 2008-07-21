/***
FILE
  notebook.cpp
DESCRIPTION
  Implementation of the FNoteBook control for win32 using comctl32.dll
HOST
  WC 11.0
HISTORY
  97-10-01 ISJ Created
***/
#include "notebook.h"

#define NOTOOLBAR
#define NOUPDOWN
#define NOSTATUSBAR
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOPROGRESS
#define NMOHOTKEY
#define NOHEADER
#define NOLISTVIEW
#define NOTREEVIEW
#define NOANIMATE
#include <commctrl.h>
#include <string.h>


struct notebook_page {
        int tab_index;
        FNoteBook::PageId pageid;
        Bool major;
        char *text;
        FWnd *wnd;
        long data;
};

#ifdef __WATCOMC__
//automagically pull in comctl32.lib
#pragma library(comctl32)
#endif


//A little handler that grabs WM_NOTIFY in the notebook's parent
class notebook_notify_handler : public FHandler {
        FNoteBook *nb;
public:
        notebook_notify_handler(FWnd *pwnd, FNoteBook *n)
          : FHandler(pwnd),
            nb(n)
        { SetDispatch(dispatchFunc_t(&notebook_notify_handler::Dispatch)); }
protected:
        Bool Dispatch(FEvent &ev);
};

Bool notebook_notify_handler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_NOTIFY &&
           ev.GetWParam()==nb->GetID())
        {
                NMHDR *pnmh=(NMHDR*)ev.GetLParam();
                if(pnmh->hwndFrom==nb->GetHwnd())
                        return nb->HandleWM_NOTIFY(ev);
        }
        return False;
}



static const FNoteBook::PageId FNoteBook::nopage = (FNoteBook::PageId)-1;

FNoteBook::FNoteBook()
  : FControl(),
    FSizeHandler(this),
    aPage(0),
    pages(0),
    pageid_generator(1),
    notify_handler(0),
    wnd_text(0)
{
}

FNoteBook::~FNoteBook() {
        for(int i=0; i<pages; i++) {
                delete[] aPage[i].text;
        }
        delete[] aPage;
        delete notify_handler;
        delete wnd_text;
}


Bool FNoteBook::Create(FWnd *parent, int ID) {
        InitCommonControls();
        if(!FControl::Create(parent,ID))
                return False;
        notify_handler = new notebook_notify_handler(parent,this);
        //createText(); for reasons unknown does not work until at least one item is present
        return True;
}


Bool FNoteBook::Create(FWnd *pParent, int ID, FRect *r) {
        InitCommonControls();
        if(!FControl::Create(pParent, ID,
                             WC_TABCONTROL, "",
                             WS_CLIPCHILDREN|WS_TABSTOP|WS_VISIBLE|TCS_TABS|TCS_SINGLELINE,
                             r
                            ))
                return False;
        notify_handler = new notebook_notify_handler(pParent,this);
        //createText(); for reasons unknown does not work until at least one item is present
        return True;
}


FNoteBook::PageId FNoteBook::InsertPage(PageId before, const char *name, long data) {
        int i;
        if(before==nopage)
                i = pages;
        else {
                i = pageid2aidx(before);
        }

        notebook_page *np=new notebook_page[pages+1];
        if(!np) return nopage;

        //copy
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

        //recalc tab_index
        {
                int ti=0;
                for(int tu=0; tu<pages; tu++) {
                        if(tu>0 && aPage[tu].major)
                                ti++;
                        aPage[tu].tab_index = ti;
                }
        }

        if(aPage[i].major) {
                //tell tabcontrol about the page
                TC_ITEM tc_item;
                memset(&tc_item, 0, sizeof(tc_item));
                tc_item.mask = TCIF_TEXT|TCIF_PARAM;
                tc_item.pszText = (LPSTR)name;
                tc_item.lParam = aPage[i].pageid;

                LRESULT rc = SendMessage(GetHwnd(), TCM_INSERTITEM, (WPARAM)aPage[i].tab_index, (LPARAM)(&tc_item));
                if(rc==-1)
                        return nopage;
        }

        if(!wnd_text)
                createText();
        
        return aPage[i].pageid;
}


Bool FNoteBook::DeletePage(PageId page) {
        int i=pageid2aidx(page);
        if(i<0) return False; //no such page
        int wasCurrent = (page==GetCurrentPage());

        //todo: not-major page processing
        SendMessage(GetHwnd(), TCM_DELETEITEM, (WPARAM)aPage[i].tab_index, 0);
        
        delete[] aPage[i].text;
        for(int di=i; di<pages-1; di++)
                aPage[di] = aPage[di+1];
        pages--;

        if(wasCurrent) {
                if(i>0)
                        FlipTo(aPage[i-1].pageid);
                else if(i<pages-1)
                        FlipTo(aPage[i].pageid);
                else {
                }
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
        if(i<0)
                return False;
        else {
                aPage[i].data = data;
                return True;
        }
}


Bool FNoteBook::SetPageText(PageId page, const char *text) {
        int i=pageid2aidx(page);
        if(i<0) return False;

        delete[] aPage[i].text;
        aPage[i].text = 0;
        if(text) {
                int sl=strlen(text);
                aPage[i].text = new char[sl+1];
                if(aPage[i].text)
                        strcpy(aPage[i].text,text);
                else
                        return False;
        }
        if(page==GetCurrentPage())
                updatetext();
        return True;
}

Bool FNoteBook::GetPageText(PageId page, char *buf, int maxbuf) {
        if(maxbuf<1) return False;
        int i=pageid2aidx(page);
        if(i<0) return False;

        if(aPage[i].text==0)
                *buf='\0';
        else {
                strncpy(buf,aPage[i].text,maxbuf);
                buf[maxbuf-1]='\0';
        }
        return True;
}


FNoteBook::PageId FNoteBook::GetCurrentPage() {
        LRESULT rc = SendMessage(GetHwnd(), TCM_GETCURSEL, 0,0);
        if(rc==(LRESULT)-1)
                return nopage;

        TC_ITEM tc_item;
        memset(&tc_item, 0, sizeof(tc_item));
        tc_item.mask = TCIF_PARAM;
        rc = SendMessage(GetHwnd(), TCM_GETITEM, (WPARAM)rc, (LPARAM)&tc_item);

        return (PageId)(tc_item.lParam);
}

Bool FNoteBook::FlipTo(PageId page) {
        int i=pageid2aidx(page);
        if(i<0)
                return False;
        if(!loadpage(page))
                return False;
        LRESULT rc=SendMessage(GetHwnd(), TCM_SETCURSEL, (WPARAM)aPage[i].tab_index, 0);

        if(rc==(LRESULT)-1)
                return False;

        TC_ITEM tc_item;
        memset(&tc_item, 0, sizeof(tc_item));
        tc_item.mask = TCIF_PARAM;
        tc_item.lParam = (LPARAM)page;
        SendMessage(GetHwnd(), TCM_SETITEM, (WPARAM)aPage[i].tab_index, (LPARAM)&tc_item);
        
        showpage(page,2);

        Flipped(page);
        
        return True;
}


int FNoteBook::pageid2aidx(PageId page) {
        //map pageid -> index into aPage[]
        for(int i=0; i<pages; i++)
                if(aPage[i].pageid==page)
                        return i;
        return -1;
}


Bool FNoteBook::HandleWM_NOTIFY(FEvent &ev) {
        NMHDR *pnmh=(NMHDR*)ev.GetLParam();
        switch(pnmh->code) {
                case TCN_SELCHANGING: {
                        //selection is chainging, ask derived class wether this
                        //is OK or not
                        if(!Flipping(GetCurrentPage(),nopage))
                                ev.SetResult((LRESULT)TRUE);
                        else
                                ev.SetResult((LRESULT)FALSE);
                        return True;
                }
                case TCN_SELCHANGE: {
                        LRESULT rc = SendMessage(GetHwnd(), TCM_GETCURSEL, 0,0);
                        if(rc==(LRESULT)-1)
                                return False;
                        for(int i=0; i<pages && aPage[i].tab_index!=rc; i++)
                                ;
                        if(i<pages) {
                                PageId page=aPage[i].pageid;
                                if(loadpage(page)) {
                                        TC_ITEM tc_item;
                                        memset(&tc_item, 0, sizeof(tc_item));
                                        tc_item.mask = TCIF_PARAM;
                                        tc_item.lParam = (LPARAM)aPage[i].pageid;
                                        SendMessage(GetHwnd(), TCM_SETITEM, (WPARAM)aPage[i].tab_index, (LPARAM)&tc_item);
                                        showpage(page,1);
                                }
                                Flipped(page);
                        }
                        return True;
                }
        }
        return False;
}

Bool FNoteBook::SizeChanged(FSizeEvent &ev) {
        //resize pages and text
        FRect r = FRect(0,0,ev.GetWidth(),ev.GetHeight());
        ::SendMessage(GetHwnd(), TCM_ADJUSTRECT, FALSE, (LPARAM)&r);
        FRect tr;
        {
                if(wnd_text) {
                        GetClientRect(wnd_text->GetHwnd(), &tr);
                        MapWindowPoints(wnd_text->GetHwnd(), GetHwnd(), (POINT*)&tr, 2);
                } else {
                        tr=r;
                        r.top += 18;
                }
        }
        r.top += tr.GetHeight();

        //resize pages
        for(int i=0; i<pages; i++) {
                if(aPage[i].wnd) {
                        SetWindowPos(aPage[i].wnd->GetHwnd(),
                                     NULL,
                                     r.left,r.top,
                                     r.GetWidth(), r.GetHeight(),
                                     SWP_NOZORDER
                                    );
                }
        }

        //resize text
        if(wnd_text)
                SetWindowPos(wnd_text->GetHwnd(), NULL,
                             r.left,tr.top,
                             r.GetWidth(),tr.GetHeight(),
                             SWP_NOZORDER
                            );

        return False;
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
        }
        if(newPage!=currentPage) {
                if(Flipping(currentPage,newPage))
                        FlipTo(newPage);
                return True; //eat event
        } else
                return False;
}



Bool FNoteBook::loadpage(PageId page) {
        int i=pageid2aidx(page);
        if(i<0)
                return False;
        
        if(aPage[i].wnd)
                return True; //already loaded

        aPage[i].wnd = LoadPage(page, this);
        if(aPage[i].wnd) {
                //force page parent to be us (FDialog may have overridden the specified parent
                ::SetParent(aPage[i].wnd->GetHwnd(),GetHwnd());
                //calc & set pos&size
                FRect r;
                ::GetClientRect(GetHwnd(),&r);
                ::SendMessage(GetHwnd(), TCM_ADJUSTRECT, FALSE, (LPARAM)&r);
                ::MapWindowPoints(GetHwnd(), aPage[i].wnd->GetParent()->GetHwnd(), (POINT*)&r, 2);
                if(wnd_text) {
                        FRect tr; GetWindowRect(wnd_text->GetHwnd(),&tr);
                        r.top += tr.GetHeight();
                }
                ::SetWindowPos(aPage[i].wnd->GetHwnd(),
                               HWND_TOP, //GetHwnd(),
                               r.GetLeft(), r.GetTop(),
                               r.GetWidth(), r.GetHeight(),
                               SWP_HIDEWINDOW
                              );
                //force ws_tabstop style on page (otherwise the user will not be able to tab to it)
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
        if(i<0)
                return False;

        //ensure page is loaded
        if(!aPage[i].wnd)
                if(!loadpage(page))
                        return False;
        
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
        for(int si=0; si<pages; si++) {
                if(si==i)
                        aPage[si].wnd->Show();
                else
                        if(aPage[si].wnd)
                                aPage[si].wnd->Hide();
        }

        updatetext();

        //set focus if needed
        if(setfocus)
                ::SetFocus(aPage[i].wnd->GetHwnd());
        
        return True;
}


void FNoteBook::createText() {
        //create page text control
        //we use a static for this

        HFONT hfont=(HFONT)SendMessage(GetHwnd(), WM_GETFONT, 0,0);
        int h;
        //measure font height
        {
                HDC hdc=GetDC(GetHwnd());
                if(hdc) {
                        int s=SaveDC(hdc);
                        if(hfont) SelectObject(hdc,hfont);
                        SIZE size;
#if BIFOS_==BIFOS_WIN32_
                        GetTextExtentPoint32(hdc,(LPSTR)"X",1,&size);
#else
                        GetTextExtentPoint(hdc,(LPSTR)"X",1,&size);
#endif
                        if(s) RestoreDC(hdc,s);
                        ReleaseDC(GetHwnd(),hdc);
                        h = (int)size.cy;
                } else
                        h = 18;
        }
        FRect r;
        ::GetClientRect(GetHwnd(), &r);
        ::SendMessage(GetHwnd(), TCM_ADJUSTRECT, FALSE, (LPARAM)&r);
        r.bottom = r.top + h;

        FControl *tc=new FControl;
        wnd_text = new FControl;
        if(tc) {
                tc->Create(this,
                           -1,
                           "STATIC",
                           "",
                           WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|SS_LEFTNOWORDWRAP|SS_NOPREFIX,
                           &r
                          );
                if(tc->IsValid())
                        wnd_text=tc;
                else
                        delete tc;
        }
        if(wnd_text && hfont);
                SendMessage(wnd_text->GetHwnd(), WM_SETFONT, (WPARAM)hfont, MAKELPARAM(FALSE,0));
}

void FNoteBook::updatetext() {
        //refresh the contents of page text control
        if(wnd_text) {
                int i=pageid2aidx(GetCurrentPage());
                if(i>=0)
                        if(aPage[i].text)
                                SendMessage(wnd_text->GetHwnd(), WM_SETTEXT, 0, (LPARAM)(LPSTR)aPage[i].text);
                        else
                                SendMessage(wnd_text->GetHwnd(), WM_SETTEXT, 0, (LPARAM)(LPSTR)"");
        }
}

