#define INCL_GPI
#define BIFINCL_WINDOW
#define BIFINCL_CONTROL
#define BIFINCL_FRAMEWINDOW
#define BIFINCL_APPWND
#define BIFINCL_APPLICATION
#define BIFINCL_CLIENTHANDLERS
#include <bif.h>
#include <string.h>

class FStatusBar;
class FStatusBar_specialhandler : public FHandler {
protected:
        FStatusBar_specialhandler(FStatusBar *statusbar);

        Bool Dispatch(FEvent &ev);
        virtual Bool Paint(FEvent &)      =0;
        virtual Bool PresParamChanged(FEvent&) =0;
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
        unsigned borderx,bordery,borderpane;
        FStatusBarAutoAdjustHandler *autoadjust;
protected:
        Bool Paint(FEvent &);
        Bool PresParamChanged(FEvent&);
};

class FStatusBarAutoAdjustHandler : public FHandler {
        FStatusBar *sb;
        UINT wm_adjuststatusbar;
public:
        FStatusBarAutoAdjustHandler(FWnd *pParent, FStatusBar *statusbar);

        Bool Dispatch(FEvent &ev);
};


FStatusBarAutoAdjustHandler::FStatusBarAutoAdjustHandler(FWnd *pParent, FStatusBar *statusbar)
  : FHandler(pParent), sb(statusbar), wm_adjuststatusbar(WM_USER+1)
{
        SetDispatch(dispatchFunc_t(&FStatusBarAutoAdjustHandler::Dispatch));
}

Bool FStatusBarAutoAdjustHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_SIZE) {
                WinPostMsg(GetWnd()->GetHwnd(),wm_adjuststatusbar,0,0);
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
                case WM_SIZE:
                        WinInvalidateRect(GetWnd()->GetHwnd(),NULL,TRUE);
                        return False;
                case WM_PRESPARAMCHANGED:
                        return PresParamChanged(ev);
                default:
                        return False;
        }
}


FStatusBar::FStatusBar()
  : FStatusBar_specialhandler(this),
    pane(0), panes(0),
    autoadjust(0)
{
        //calculate default borders depending on screen resolution
        int screenwidth=WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
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
        WinInvalidateRect(GetHwnd(),NULL,TRUE);

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
                int sl=strlen(pszText);
                pane[p].text = new char[sl+1];
                if(!pane[p].text) return False;
                strcpy(pane[p].text,pszText);
                pane[p].align=align;
                WinInvalidateRect(GetHwnd(),NULL,TRUE);
        } else {
                pane[p].text = 0;
        }
        return True;
}

Bool FStatusBar::GetPaneText(unsigned p, char *buf, unsigned maxbuf) {
        if(p>=panes) return False;
        if(pane[p].text) {
                strncpy(buf,pane[p].text,maxbuf);
                buf[maxbuf-1] = '\0';
        } else
                buf[0] = '\0';
        return True;
}

unsigned FStatusBar::GetPaneTextLen(unsigned p) {
        if(p>=panes) return 0;
        if(pane[p].text)
                return strlen(pane[p].text);
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

        int h;
        //best guess is font height plus yborder plus 2
        HPS hps=WinGetPS(GetHwnd());
        if(hps) {
                FONTMETRICS fm;
                GpiQueryFontMetrics(hps, sizeof(fm), &fm);
                h = fm.lMaxBaselineExt;
                WinReleasePS(hps);
        } else
                 h = WinQuerySysValue(HWND_DESKTOP,SV_CYTITLEBAR); //best guess without font knowledge
        h+=bordery*2+2;

        FMoveHandler m(this);
        m.Move(0,0);
        FSizeHandler sz(this);
        sz.SetSize(pw,h);
        return True;
}


Bool FStatusBar::Paint(FEvent &) {
        HPS hps=WinBeginPaint(GetHwnd(), NULLHANDLE, NULL);
        
        
        FSizeHandler sz(this);
        int sbwidth,sbheight; sz.GetSize(&sbwidth,&sbheight);
        

        //calc colors to use
        ULONG backgroundcolor;
        ULONG foregroundcolor;
        ULONG light;
        ULONG dark;
        {
                ULONG attrFound=0;
                ULONG cbRetLen;
                cbRetLen = WinQueryPresParam(GetHwnd(),
                                             PP_BACKGROUNDCOLORINDEX,
                                             PP_BACKGROUNDCOLOR,
                                             &attrFound,
                                             sizeof(backgroundcolor),
                                             &backgroundcolor,
                                             0
                                            );
                if(cbRetLen==0) {
                        //not found at all
                        backgroundcolor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONMIDDLE, 0);
                        backgroundcolor = GpiQueryColorIndex(hps, 0, backgroundcolor);
                } else if(attrFound==PP_BACKGROUNDCOLOR) {
                        //convert RGB to color index
                        backgroundcolor = GpiQueryColorIndex(hps, 0, backgroundcolor);
                }
        }
        {
                ULONG attrFound=0;
                ULONG cbRetLen;
                cbRetLen = WinQueryPresParam(GetHwnd(),
                                             PP_FOREGROUNDCOLORINDEX,
                                             PP_FOREGROUNDCOLOR,
                                             &attrFound,
                                             sizeof(foregroundcolor),
                                             &foregroundcolor,
                                             0
                                            );
                if(cbRetLen==0) {
                        //not found at all
                        foregroundcolor = WinQuerySysColor(HWND_DESKTOP, SYSCLR_OUTPUTTEXT, 0);
                        foregroundcolor = GpiQueryColorIndex(hps, 0, foregroundcolor);
                } else if(attrFound==PP_FOREGROUNDCOLOR) {
                        //convert RGB to color index
                        foregroundcolor = GpiQueryColorIndex(hps, 0, foregroundcolor);
                }
        }

        light = GpiQueryColorIndex(hps, 0, WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONLIGHT, 0));
        dark = GpiQueryColorIndex(hps, 0, WinQuerySysColor(HWND_DESKTOP, SYSCLR_BUTTONDARK, 0));

        //erase background
        {
                FRect r(0,0,sbwidth,sbheight);
                WinFillRect(hps, &r, backgroundcolor);
        }

        //draw panes
        int panetop=sbheight-bordery;
        int panebottom=bordery;
        int paneleft=borderx;
        for(int p=0; p<panes && paneleft<sbwidth; p++) {
                int paneright;
                if(pane[p].width<0) {
                        int minimumWidth = -pane[p].width;
                        int textWidth;
                        if(pane[p].text) {
                                FRect textrect(0,0,panetop-panebottom,10000);
                                WinDrawText(hps, strlen(pane[p].text),pane[p].text, &textrect, 0,0, DT_LEFT|DT_QUERYEXTENT);
                                textWidth = textrect.GetWidth();
                        } else
                                textWidth=0;
                        if(textWidth<minimumWidth)
                                paneright = paneleft + minimumWidth +2;
                        else
                                paneright = paneleft + textWidth +2;
                } else if(pane[p].width==0) {
                        paneright = sbwidth-borderx; //rest of statusbar
                } else {
                        paneright = paneleft+pane[p].width+2;
                }
                if(paneright>sbwidth-borderx)
                        paneright = sbwidth-borderx;
                
                {
                        GpiSetColor(hps, dark);
                        POINTL aptlPoints[3] = {{paneleft,panebottom+1},   {paneleft, panetop-1},        {paneright-2, panetop-1}};
                        GpiMove(hps, aptlPoints+0);
                        GpiPolyLine(hps, 2, aptlPoints+1);
                }
                {
                        GpiSetColor(hps, light);
                        POINTL aptlPoints[3] = {{paneleft+1,panebottom}, {paneright-1,panebottom}, {paneright-1,panetop-2}};
                        GpiMove(hps, aptlPoints+0);
                        GpiPolyLine(hps, 2, aptlPoints+1);
                }

                if(pane[p].text) {
                        //draw text
                        FRect textrect(paneleft+1,panebottom+1,paneright-1,panetop-1);
                        ULONG align;
                        if(pane[p].align==align_left)
                                align=DT_LEFT;
                        else if(pane[p].align==align_center)
                                align=DT_CENTER;
                        else 
                                align=DT_RIGHT;
                        WinDrawText(hps, strlen(pane[p].text),pane[p].text, &textrect, foregroundcolor, 0, align|DT_TOP);
                }
                paneleft = paneright+borderpane;
        }
        
        
        WinEndPaint(hps);

        return True;
}

Bool FStatusBar::PresParamChanged(FEvent&) {
        //the easy way...
        AdjustSize();
        WinInvalidateRect(GetHwnd(),NULL,TRUE);
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
        
//font change test missing
        return True;
}

Bool MyClient::Paint(FPaintEvent &) {
        FRect r;
        HPS hps=WinBeginPaint(GetHwnd(),NULLHANDLE,&r);
        WinFillRect(hps, &r, CLR_WHITE);
        WinEndPaint(hps);
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

