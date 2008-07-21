#include "testmenu.hpp"
#include "genmenu.hpp"
#include "testmenu.h"

int MenuTestWindow::DoModal(FWnd *pOwner, const MenuItemList &itemList) {
        mil=&itemList;
        return FModalDialog::DoModal(pOwner,DIALOG_TESTMENU);
}

Bool MenuTestWindow::CreateControls() {
        HWND hwndMenu=GenerateMenu(*mil, GetHwnd(), 0);
        if(hwndMenu) {
                //tell the frame that we have added a menu
                WinSendMsg(GetHwnd(),WM_UPDATEFRAME,MPFROMLONG(FCF_MENU),0);
                return True;
        } else
                return False;
}

Bool MenuTestWindow::Command(FCommandEvent &) {
        EndDialog(0);
        return True;
}



Bool MenuTestWindow::OwnerDraw(FDrawItemEvent&) {
        return False;   //not yet
}

Bool MenuTestWindow::OwnerMeasure(FMeasureItemEvent&) {
        return False;   //not yet
}


Bool PopupMenuTestWindow::CreateControls() {
        //Bypass the inherited method
        return True;
}


Bool PopupMenuTestWindow::MouseContextMenu(FMouseEvent&) {
        HWND hwndMenu=GenerateMenu(*mil, GetHwnd(), 1);

        POINTL p;
        WinQueryPointerPos(HWND_DESKTOP,&p);
        WinMapWindowPoints(HWND_DESKTOP,GetHwnd(),&p,1);
        WinPopupMenu(GetHwnd(),
                     GetHwnd(), 
                     hwndMenu, 
                     p.x,p.y,
                     0, 
                     PU_HCONSTRAIN|PU_VCONSTRAIN|PU_NONE|PU_KEYBOARD|PU_MOUSEBUTTON1|PU_MOUSEBUTTON2
                    );
        return True;
}

