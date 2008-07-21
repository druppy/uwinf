#ifndef __NOTEBOOK_H
#define __NOTEBOOK_H

#define BIFINCL_WINDOW
#define BIFINCL_CONTROL
#define BIFINCL_COMMONHANDLERS
#include <bif.h>

struct notebook_page;
class notebook_notify_handler;

class FNoteBook : public FControl,
                  public FSizeHandler
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
        
protected:
        Bool PumpEvent(FEvent &ev);
        Bool SizeChanged(FSizeEvent &ev);
        
private:
        notebook_page *aPage;
        int pages;
        PageId pageid_generator;
        int pageid2aidx(PageId page);
        Bool loadpage(PageId page);
        Bool showpage(PageId page, int setfocus);
        
        FHandler *notify_handler;
        Bool HandleWM_NOTIFY(FEvent &ev);
        friend class notebook_notify_handler;
        
        FWnd *wnd_text;
        void createText();
        void updatetext();
};

#endif

