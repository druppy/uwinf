#ifndef __TESTMENU_HPP
#define __TESTMENU_HPP

#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_CONTROLS
#define BIFINCL_OWNERDRAW
#define BIFINCL_MOUSEHANDLERS
#include <bif.h>

#include "menuitem.hpp"

class MenuTestWindow : public FModalDialog, public FOwnerDrawOwner {
protected:
        HWND hwndMenu;  //the menu to test
        const MenuItemList *mil;
public:
        MenuTestWindow() : FModalDialog(), FOwnerDrawOwner(this) {}

        int DoModal(FWnd *pOwner, const MenuItemList &menuItemList);
        Bool CreateControls();

protected:
        Bool Command(FCommandEvent &);
        Bool OwnerDraw(FDrawItemEvent&);
        Bool OwnerMeasure(FMeasureItemEvent&);
};

class PopupMenuTestWindow : public MenuTestWindow, public FTranslatedMouseHandler {
public:
        PopupMenuTestWindow() : MenuTestWindow(), FTranslatedMouseHandler(this) {}

        Bool CreateControls();

protected:
        Bool MouseContextMenu(FMouseEvent&);
};


#endif

