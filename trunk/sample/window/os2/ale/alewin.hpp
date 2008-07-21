#ifndef __ALEWIN_HPP
#define __ALEWIN_HPP

#define BIFINCL_WINDOW
#define BIFINCL_APPWND
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_CLIENTHANDLERS
#define BIFINCL_MOUSEHANDLERS
#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_SIZERESTRICTIONHANDLERS
#define BIFINCL_APPLICATION
#include <bif.h>

#include "lnote.hpp"
#include "linka.hpp"

class ALEClientWindow : public FClientWindow,
                        public FCommandHandler,
                        public FCloseHandler,
                        public FTranslatedMouseHandler,
                        public FPaintHandler,
                        public FSizeRestrictionClientHandler,
                        public LinkChangeNotifier
{
        HDC hdcBitmap;
        HPS hpsBitmap;
        HBITMAP hbm;
        int bitmapWidth,bitmapHeight;

        LinkArray la;
        int currentLink;
        char bitmapfilename[256];
        char linkfilename[256];
        Bool changed;

        void invalidateLink(int i);
        int linkFromPoint(const FPoint &p);
        void setCurrentLink(int i);
        FPoint createStart,createEnd;
        Bool creationInProgress;
public:
        ALEClientWindow(FSizeRestrictionFrameHandler *srfh);
        ~ALEClientWindow();

        Bool Create(FAppWindow *pParent);

        void ChangeLink(const Link&);

        Bool LoadBitmap(const char *filename);
        Bool LoadFile();
        Bool SaveFile();
protected:
        Bool Command(FCommandEvent &);
        Bool Close(FEvent&);
        Bool CloseUnconditionally(FEvent&);
        Bool Paint(FPaintEvent &);

        Bool MouseSingleSelect(FMouseEvent&);
        Bool MouseOpen(FMouseEvent&);
        Bool MouseSelectStart(FMouseEvent&);
        Bool MouseSelectEnd(FMouseEvent&);
        Bool MouseMove(FMouseMoveEvent&);

        Bool GetMinimumWidth(int &);
        Bool GetMinimumHeight(int &);
        Bool GetMaximumWidth(int &);
        Bool GetMaximumHeight(int &);

private:
        Bool editCurrentLink();
};

#endif

