#ifndef __MENUWIN_H_INCLUDED
#define __MENUWIN_H_INCLUDED

#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_KEYBOARDHANDLERS
#define BIFINCL_MOUSEHANDLERS
#include <bif.h>

//menu window styles
#define MS_ACTIONBAR    0x00000001

//general type
#define MIS_SUBMENU     0x0001
#define MIS_SEPARATOR   0x0002
//display type
#define MIS_TEXT        0x0000
#define MIS_OWNERDRAW   0x0020
//command type
#define MIS_COMMAND     0x0000
#define MIS_SYSCOMMAND  0x0100
#define MIS_HELP        0x0200
//misc
#define MIS_BREAK       0x1000  /*not implemented*/
#define MIS_STATIC      0x2000


//item attributes
#define MIA_HILITE      0x0001
#define MIA_CHECKED     0x0002
#define MIA_DISABLED    0x0004
#define MIA_NODISMISS   0x0008


struct MENUITEM {
        sint16  pos;
        uint16  style;
        uint16  attr;
        uint16  id;
        WmsHWND submenu;
        uint32  handle;
};

class FMenuWindow_Handler : public FHandler {
public:
        FMenuWindow_Handler(FWnd *pwnd)
          : FHandler(pwnd)
          { SetDispatch(dispatchFunc_t(FMenuWindow_Handler::Dispatch)); }
protected:
        Bool Dispatch(FEvent &ev);
        virtual Bool    mm_deleteitem(uint16 id, Bool includeSubMenus) =0;
        virtual void    mm_endmenumode() =0;
        virtual sint16  mm_insertitem(const MENUITEM *i, const char *t) =0;
        virtual Bool    mm_isitemvalid(uint16 id, Bool includeSubMenus) =0;
        virtual sint16  mm_itemidfromposition(uint16 index) =0;
        virtual sint16  mm_itempositionfromid(uint16 id) =0;
        virtual Bool    mm_queryitem(uint16 id, Bool includeSubMenus, MENUITEM *i) =0;
        virtual uint16  mm_queryitemattr(uint16 id, Bool includeSubMenus) =0;
        virtual uint16  mm_queryitemcount() =0;
        virtual Bool    mm_queryitemrect(uint16 id, Bool includeSubMenus, FRect *r) =0;
        virtual Bool    mm_queryitemtext(uint16 id, uint16 maxbuf, char *buf) =0;
        virtual uint16  mm_queryitemtextlength(uint16 id) =0;
        virtual uint16  mm_queryselitemid(Bool includeSubMenus) =0;
        virtual Bool    mm_removeitem(uint16 id, Bool includeSubMenus) =0;
        virtual Bool    mm_selectitem(uint16 id, Bool includeSubMenus, Bool dismiss) =0;
        virtual Bool    mm_setitem(Bool includeSubMenus, const MENUITEM *i) =0;
        virtual Bool    mm_setitemattr(uint16 id, Bool includeSubMenus, uint16 mask, uint16 bits) =0;
        virtual Bool    mm_setitemhandle(uint16 id, Bool includeSubMenus, uint32 h) =0;
        virtual Bool    mm_setitemtext(uint16 id, const char *text) =0;
        virtual Bool    mm_startmenumode(Bool showSubMenu, Bool resumeMenu) =0;
        virtual Bool    wm_adjustwindowpos(WmsSWP *pswp, uint16 flags) =0;
        virtual Bool    wm_focuschanging(WmsHWND otherHwnd, Bool get, uint16 flags) =0;
};



class FMenuWindow : public FControl,
                    public FMenuWindow_Handler,
                    public FMouseHandler,
                    public FKeyboardHandler
{
        struct MenuItem {
                char *text;
                uint16 style;
                uint16 attr;
                int height;
                int width;
                FRect r;
                WmsHWND hwndSubMenu;
                Bool deleteWnd;
                uint16 id;
                uint32 handle;
        };
        MenuItem *item;
        unsigned items;
public:
        FMenuWindow()
          : FControl(),
            FMenuWindow_Handler(this),
            FMouseHandler(this),
            FKeyboardHandler(this),
            item(0), items(0)
          {}
        ~FMenuWindow();

        Bool Create(FWnd *pwndOwner, int ID);
protected:
        unsigned QueryDlgCode() { return DLGC_MENU; }
        Bool MatchMnemonic(char c);

        Bool    mm_deleteitem(uint16 id, Bool includeSubMenus);
        void    mm_endmenumode();
        sint16  mm_insertitem(const MENUITEM *i, const char *t);
        Bool    mm_isitemvalid(uint16 id, Bool includeSubMenus);
        sint16  mm_itemidfromposition(uint16 index);
        sint16  mm_itempositionfromid(uint16 id);
        Bool    mm_queryitem(uint16 id, Bool includeSubMenus, MENUITEM *i);
        uint16  mm_queryitemattr(uint16 id, Bool includeSubMenus);
        uint16  mm_queryitemcount();
        Bool    mm_queryitemrect(uint16 id, Bool includeSubMenus, FRect *r);
        Bool    mm_queryitemtext(uint16 id, uint16 maxbuf, char *buf);
        uint16  mm_queryitemtextlength(uint16 id);
        uint16  mm_queryselitemid(Bool includeSubMenus);
        Bool    mm_removeitem(uint16 id, Bool includeSubMenus);
        Bool    mm_selectitem(uint16 id, Bool includeSubMenus, Bool dismiss);
        Bool    mm_setitem(Bool includeSubMenus, const MENUITEM *i);
        Bool    mm_setitemattr(uint16 id, Bool includeSubMenus, uint16 mask, uint16 bits);
        Bool    mm_setitemhandle(uint16 id, Bool includeSubMenus, uint32 h);
        Bool    mm_setitemtext(uint16 id, const char *text);
        Bool    mm_startmenumode(Bool showSubMenu, Bool resumeMenu);
        Bool    wm_adjustwindowpos(WmsSWP *pswp, uint16 flags);
        Bool    wm_focuschanging(WmsHWND otherHwnd, Bool get, uint16 flags);

        virtual Bool Paint();

        Bool KeyPressed(FKeyboardEvent &ev);
        Bool CharInput(FKeyboardEvent &ev);

        Bool ButtonDown(FButtonDownEvent&);
        Bool ButtonUp(FButtonUpEvent&);
        Bool MouseMove(FMouseMoveEvent&);

private:
        Bool processMnemonic(char c);
        int  point2Item(FPoint p);
        void adjustItemRects();
        void adjustItemRects(int w);
        void measureItemParts(int *maxLeft, int *maxAccel, int *maxOwnerdraw);
        void rollupSubMenus();
        void dropdownSubMenu(int i, Bool resume);
        int findId(int id);
        int findSelectedItem();
        int findPrevSelectableItem(int i);
        int findNextSelectableItem(int i);
        void selectItem(int i);
        void chooseItem(int i, Bool pointerDidIt);
        Bool isSelectableItem(int i);
        Bool recurseBool(WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2);
        uint16 recurseNN(WmsMSG msg, WmsMPARAM mp1, WmsMPARAM mp2);
};


#endif  /*__MENUWIN_H_INCLUDED*/
