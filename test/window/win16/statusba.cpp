#define BIFINCL_WINDOW
#define BIFINCL_CONTROL
#define BIFINCL_FRAMEWINDOW
#define BIFINCL_APPWND
#define BIFINCL_APPLICATION
#define BIFINCL_CLIENTHANDLERS
#include <bif.h>

class FStatusBar;
class FStatusBar_specialhandler : public FHandler {
protected:
        FStatusBar_specialhandler(FStatusBar *statusbar);

        Bool Dispatch(FEvent &ev);
        virtual Bool Paint(FEvent &)      =0;
        virtual Bool SetFont(FEvent &)    =0;
        virtual Bool GetFont(FEvent &)    =0;
};


class FStatusBarAutoAdjustHandler;
class FStatusBar : public FControl,
                   private FStatusBar_specialhandler
{
public:
        FStatusBar();
        ~FStatusBar();
        
        Bool Create(FWnd *pParent, int ID);
        Bool Create(FWnd *pParent, int ID, const char *pane0text);

        Bool SetPanes(unsigned panes, const int width[]);
        Bool GetPanes(unsigned &panes, int width[]);
        unsigned GetPanes() { return panes; }
        
        enum alignment { align_left, align_right, align_center };
        Bool SetPaneText(unsigned pane, const char *pszText, alignment align=align_left);
        Bool GetPaneText(unsigned pane, char *buf, unsigned maxbuf);
        unsigned GetPaneTextLen(unsigned pane);

        Bool SetBorders(unsigned bx, unsigned by, unsigned bp);
        Bool GetBorders(unsigned &bx, unsigned &by, unsigned &bp);
        
        Bool AdjustSize();
        Bool EnableAutoAdjust();
        Bool DisableAutoAdjust();

private:
        struct Pane {
                char *text;
                int width;
                alignment align;
        };

        Pane *pane;
        unsigned panes;
        HFONT hfont;
        unsigned borderx,bordery,borderpane;
        FStatusBarAutoAdjustHandler *autoadjust;
protected:
        Bool Paint(FEvent &);
        Bool SetFont(FEvent &);
        Bool GetFont(FEvent &);
};

class FStatusBarAutoAdjustHandler : public FHandler {
        FStatusBar *sb;
        UINT wm_adjuststatusbar;
public:
        FStatusBarAutoAdjustHandler(FWnd *pParent, FStatusBar *statusbar);

        Bool Dispatch(FEvent &ev);
};


FStatusBarAutoAdjustHandler::FStatusBarAutoAdjustHandler(FWnd *pParent, FStatusBar *statusbar)
  : FHandler(pParent), sb(statusbar), wm_adjuststatusbar(RegisterWindowMessage("BIF-wm_adjuststatusbar"))
{
        SetDispatch(dispatchFunc_t(&FStatusBarAutoAdjustHandler::Dispatch));
}

Bool FStatusBarAutoAdjustHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_SIZE) {
                PostMessage(GetWnd()->GetHwnd(),wm_adjuststatusbar,0,0);
                return False;
        } else if(ev.GetID()==wm_adjuststatusbar) {
                sb->AdjustSize();
                return True;
        } else
                return False;
}

FStatusBar_specialhandler::FStatusBar_specialhandler(FStatusBar *statusbar)
  : FHandler(statusbar)
{ SetDispatch(dispatchFunc_t(&FStatusBar_specialhandler::Dispatch)); }

Bool FStatusBar_specialhandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_PAINT:
                        return Paint(ev);
                case WM_SETFONT:
                        return SetFont(ev);
                case WM_GETFONT:
                        return GetFont(ev);
                case WM_SIZE:
                        InvalidateRect(GetWnd()->GetHwnd(),NULL,TRUE);
                        return False;
                default:
                        return False;
        }
}


FStatusBar::FStatusBar()
  : FStatusBar_specialhandler(this),
    pane(0), panes(0),
    hfont(NULL),
    autoadjust(0)
{
        //calculate default borders depending on screen resolution
        int screenwidth=GetSystemMetrics(SM_CXSCREEN);
        if(screenwidth>800) {
                //mode 1024x768, 1280x1024, ...
                borderx=4;
                bordery=2;
                borderpane=4;
        } else if(screenwidth>640) {
                borderx=2;
                bordery=2;
                borderpane=2;
        } else {
                //standard VGA 640x480 or worse
                borderx=1;
                bordery=1;
                borderpane=1;
        }
}

FStatusBar::~FStatusBar() {
        delete[] pane;
        delete autoadjust;
}

Bool FStatusBar::Create(FWnd *pParent, int ID) {
        if(!FControl::Create(pParent,ID)) return False;

        int t=0;
        SetPanes(1,&t);
        AdjustSize();

        return True;
}

Bool FStatusBar::Create(FWnd *pParent, int ID, const char *pane0text) {
        FRect r(0,0,0,0);
        if(!FControl::Create(pParent,ID,0,0,0,&r))
                return False;

        int t=0;
        SetPanes(1,&t);
        if(pane0text) SetPaneText(0,pane0text);
        AdjustSize();

        return True;
}

Bool FStatusBar::SetPanes(unsigned p, const int width[]) {
        if(p==0) return False;
        Pane *newpane = new Pane[p];
        if(!newpane) return False;
        
        //delete old panes
        for(unsigned i=0; i<panes; i++)
                delete[] pane[i].text;
        delete[] pane;

        pane=newpane;
        panes=p;
        for(i=0; i<panes; i++) {
                pane[i].text=0;
                pane[i].width=width[i];
                pane[i].align=align_left;
        }
        InvalidateRect(GetHwnd(),NULL,TRUE);

        return True;
}

Bool FStatusBar::GetPanes(unsigned &p, int width[]) {
        for(unsigned i=0; i<p && i<panes; i++)
                width[i] = pane[i].width;
        p = i;
        return True;
}


Bool FStatusBar::SetPaneText(unsigned p, const char *pszText, alignment align) {
        if(p>=panes) return False;
        delete[] pane[p].text;
        if(pszText) {
                int sl=lstrlen(pszText);
                pane[p].text = new char[sl+1];
                if(!pane[p].text) return False;
                lstrcpy(pane[p].text,pszText);
                pane[p].align=align;
                InvalidateRect(GetHwnd(),NULL,TRUE);
        } else {
                pane[p].text = 0;
        }
        return True;
}

Bool FStatusBar::GetPaneText(unsigned p, char *buf, unsigned maxbuf) {
        if(p>=panes) return False;
        if(pane[p].text) {
                lstrcpyn(buf,pane[p].text,maxbuf);
                buf[maxbuf-1] = '\0';
        } else
                buf[0] = '\0';
        return True;
}

unsigned FStatusBar::GetPaneTextLen(unsigned p) {
        if(p>=panes) return 0;
        if(pane[p].text)
                return lstrlen(pane[p].text);
        else
                return 0;
}

Bool FStatusBar::SetBorders(unsigned bx, unsigned by, unsigned bp) {
        if(bx>0) borderx=bx; else borderx=2;
        if(by>0) bordery=by; else bordery=1;
        if(bp>0) borderpane=bp; else borderpane=2;
        AdjustSize();
        return True;
}

Bool FStatusBar::GetBorders(unsigned &bx, unsigned &by, unsigned &bp) {
        bx = borderx;
        by = bordery;
        bp = borderpane;
        return True;
}

Bool FStatusBar::EnableAutoAdjust() {
        if(autoadjust) return False; //already enabled
        FWnd *pParent = GetParent();
        if(!pParent) return False; //could not find parent
        autoadjust = new FStatusBarAutoAdjustHandler(pParent,this);
        if(!autoadjust) return False;
        AdjustSize();
        return True;
}

Bool FStatusBar::DisableAutoAdjust() {
        if(!autoadjust) return False;
        delete autoadjust; autoadjust=0;
        return True;
}

Bool FStatusBar::AdjustSize() {
        FWnd *pParent = GetParent();
        FSizeHandler psz(pParent);
        int pw,ph; psz.GetSize(&pw,&ph);

        int h = GetSystemMetrics(SM_CYMENU); //best guess so far
        //best guess is font height
        HDC hdc=GetDC(GetHwnd());
        if(hdc) {
                if(hfont) {
                        HGDIOBJ hfontOld=SelectObject(hdc,hfont);
                        if(hfontOld) {
                                SIZE sz;
                                GetTextExtentPoint(hdc, "ABCXjgh", 7, &sz);
                                h = sz.cy;
                                SelectObject(hdc,hfontOld);
                        }
                } else {
                        SIZE sz;
                        GetTextExtentPoint(hdc, "ABCXjgh", 7, &sz);
                        h = sz.cy;
                }
                ReleaseDC(GetHwnd(),hdc);
        }
        h+=bordery*2;

        FMoveHandler m(this);
        m.Move(0,ph-h);
        FSizeHandler sz(this);
        sz.SetSize(pw,h);
        return True;
}


Bool FStatusBar::Paint(FEvent &) {
        PAINTSTRUCT ps;
        HDC hdc=BeginPaint(GetHwnd(),&ps);
        int sdc=SaveDC(hdc);
        
        if(hfont)
                SelectObject(hdc, hfont);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
        
        FSizeHandler sz(this);
        int sbwidth,sbheight; sz.GetSize(&sbwidth,&sbheight);
        
        //erase background
        {
                FRect r(0,0,sbwidth,sbheight);
                HBRUSH hbr=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
                if(hbr) {
                        FillRect(hdc,&r,hbr);
                        DeleteObject(hbr);
                } else
                        FillRect(hdc,&r,(HBRUSH)GetStockObject(LTGRAY_BRUSH));
        }

        //draw the panes
        HPEN darkpen=CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNSHADOW));
        HPEN brightpen=CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNHIGHLIGHT));
        //draw panes
        int panetop=bordery;
        int panebottom=sbheight-bordery;
        int paneleft=borderx;
        for(int p=0; p<panes && paneleft<sbwidth; p++) {
                int paneright;
                if(pane[p].width<0) {
                        //width is negative minimum width
                        unsigned textwidth;
                        if(pane[p].text) {
                                SIZE textsize;
                                GetTextExtentPoint(hdc, pane[p].text, lstrlen(pane[p].text), &textsize);
                                textwidth=textsize.cx;
                        } else
                                textwidth=0;
                        if(textwidth<-pane[p].width)
                                paneright = paneleft+(-pane[p].width) +2;
                        else
                                paneright = paneleft+textwidth +2;
                } else if(pane[p].width==0) {
                        paneright = sbwidth-borderx; //rest of statusbar
                } else {
                        paneright = paneleft+pane[p].width+2;
                }
                if(paneright>sbwidth-borderx)
                        paneright = sbwidth-borderx;
                
                SelectObject(hdc,darkpen);
                MoveTo(hdc, paneleft,    panebottom-2);
                LineTo(hdc, paneleft,    panetop);
                LineTo(hdc, paneright-1, panetop);
                SelectObject(hdc,brightpen);
                MoveTo(hdc, paneleft+1, panebottom-1);
                LineTo(hdc, paneright-1,panebottom-1);
                LineTo(hdc, paneright-1,panetop);

                if(pane[p].text && pane[p].text[0]) {
                        //draw text
                        FRect textrect(paneleft+1,panetop+1,paneright-1,panebottom-1);
                        UINT align;
                        if(pane[p].align==align_left)
                                align=DT_LEFT;
                        else if(pane[p].align==align_right)
                                align=DT_RIGHT;
                        else
                                align=DT_CENTER;
                        DrawText(hdc, pane[p].text, lstrlen(pane[p].text), &textrect, align|DT_SINGLELINE|DT_NOPREFIX|DT_VCENTER);
                }
                paneleft=paneright+borderpane;
        }
        
        RestoreDC(hdc,sdc);
        DeleteObject(darkpen);
        DeleteObject(brightpen);
        
        EndPaint(GetHwnd(),&ps);

        return True;
}

Bool FStatusBar::SetFont(FEvent &ev) {
        hfont = (HFONT)ev.GetWParam();
        if(LOWORD(ev.GetLParam())) {
                AdjustSize();
                InvalidateRect(GetHwnd(),0,TRUE);
        }
        return False;
}
Bool FStatusBar::GetFont(FEvent &ev) {
        if(hfont) {
                ev.SetResult((LRESULT)hfont);
                return True;
        } else
                return False;
}



class MyClient : public FClientWindow, public FPaintHandler {
        FStatusBar statbar;
public:
        MyClient() : FPaintHandler(this) {}
        Bool Create(FAppWindow *fappwnd);
        Bool Paint(FPaintEvent &);
};

Bool MyClient::Create(FAppWindow *fappwnd) {
        if(!FClientWindow::Create(fappwnd))
                return False;
        if(!statbar.Create(this,0,""))
                return False;
        const pr[4]={-50,-100,150,0};
        statbar.SetPanes(4,pr);
        statbar.SetPaneText(0,"Line: 370");
        statbar.SetPaneText(1,"Col: 32");
        statbar.SetPaneText(2,"Mode: insert",FStatusBar::align_center);
        statbar.SetPaneText(3,"");
        statbar.EnableAutoAdjust();
        
        HFONT hfont=CreateFont(16,0, 0,0, FW_LIGHT, 0,0,0, ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,"Arial");
        if(hfont) {
                SendMessage(statbar.GetHwnd(),WM_SETFONT,(WPARAM)hfont,0);
        }
        
        return True;
}

Bool MyClient::Paint(FPaintEvent &) {
        PAINTSTRUCT ps;
        HDC hdc=BeginPaint(GetHwnd(),&ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
        EndPaint(GetHwnd(),&ps);
        return True;
}



class MyApp : public FApplication {
        FAppWindow appwnd;
        MyClient client;
public:
        Bool StartUp(int,char **) {
                if(!appwnd.Create(0,
                                  0,
                                  FFrameWindow::fcf_sysmenu|
                                  FFrameWindow::fcf_titlebar|
                                  FFrameWindow::fcf_minmaxbuttons|
                                  FFrameWindow::fcf_sizeborder,
                                  FFrameWindow::alf_none,
                                  (FRect*)0
                                 ))
                        return False;
                appwnd.SetCaption("BIF test program (statusbar)");
                if(!client.Create(&appwnd))
                        return False;
                appwnd.Show();
                appwnd.BringToFront();
                return True;
        }
        int ShutDown() {
                appwnd.Destroy();
                return 0;
        }
};

DEFBIFWINDOWMAIN(MyApp);

