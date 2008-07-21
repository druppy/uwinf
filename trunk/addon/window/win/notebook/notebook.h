#ifndef __NOTEBOOK_H
#define __NOTEBOOK_H

#define BIFINCL_WINDOW
#define BIFINCL_CONTROL
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_MOUSEHANDLERS
#define BIFINCL_CLIENTHANDLERS
#include <bif.h>


struct FNoteBook_tab;
struct FNoteBook_page;

class FNoteBook : public FControl,
                  public FSizeHandler,
                  public FPaintHandler,
                  public FFocusHandler,
                  public FMouseHandler
{
public:
        FNoteBook();
        ~FNoteBook();

        typedef int PageId;
        static const PageId nopage;
        
        Bool Create(FWnd *parent, int ID);
        Bool Create(FWnd *parent, int ID, FRect *r);

        PageId InsertPage(PageId before, const char *name, long data=0);
        Bool DeletePage(PageId page);
        
        long GetPageData(PageId page);
        Bool SetPageData(PageId page, long data);
        
        Bool SetPageText(PageId page, const char *text);
        Bool GetPageText(PageId page, char *buf, int maxbuf);

        virtual Bool Flipping(PageId /*from*/, PageId /*to*/) { return True; }
        virtual void Flipped(PageId /*to*/) { }
        virtual FWnd *LoadPage(PageId page, FWnd *parent) =0;

        PageId GetCurrentPage();
        Bool FlipTo(PageId page);
        Bool SetCurrentPage(PageId page) { return FlipTo(page); }

        //win16/32 only:
        static int drawMode; //0=semi-3D, 1=3D, 2=autodetect
protected:
        Bool PumpEvent(FEvent &ev);
        Bool SizeChanged(FSizeEvent &ev);
        Bool Paint(FPaintEvent&);
        Bool LostFocus(FKillFocusEvent&);
        Bool GotFocus(FSetFocusEvent&);
        Bool Button1Down(FButtonDownEvent &ev);
private:
        FNoteBook_page *aPage;
        int pages;
        PageId pageid_generator;

        FNoteBook_tab *aTab;
        int tabs;

        Bool leftScrollVisible, rightScrollVisible;
        int firstVisibleTab,
            focusedTab;
        PageId selectedPage;
        
        int pageid2aidx(PageId page);
        Bool loadpage(PageId page);
        Bool showpage(PageId page, int setfocus);
        
        FHandler *special_handler;
        Bool HandleWM_SETGETFONT(FEvent &ev);
        Bool HandleWM_GETDLGCODE(FEvent &ev);
        HFONT hfont;
        
        friend class notebook_special_handler;
        
        FWnd *wnd_text;
        void createText();
        void updateText();

        void calcLayout(FRect *tabrect, FRect *innerrect, FRect *textrect, FRect *pagerect);
        void calcTabs(const FRect *tabrect);
        void layout();
        int detectDrawMode();
        void makeTabVisible(int t);
        void recalcTabIndexes();
        void invalidateTab(int t);
};

#endif


