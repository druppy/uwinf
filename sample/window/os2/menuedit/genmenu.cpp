#define INCL_WIN
#include <os2.h>
#include "genmenu.hpp"


static void GenerateItem(MENUITEM &os2_mi, const MenuItem edit_mi) {
        //translate style and attribute in os/2-stuff
        os2_mi.afStyle=0;
        os2_mi.afAttribute=0;
        switch(edit_mi.ItemType) {
                case MenuItem::it_text: 
                        os2_mi.afStyle|=MIS_TEXT; break;
                case MenuItem::it_separator: 
                        os2_mi.afStyle|=MIS_SEPARATOR; break;
                case MenuItem::it_bitmap: 
                        os2_mi.afStyle|=MIS_BITMAP; break;
                case MenuItem::it_ownerdraw: 
                        os2_mi.afStyle|=MIS_OWNERDRAW; break;
        }
        if(edit_mi.Static)
                os2_mi.afStyle|=MIS_STATIC;
        if(edit_mi.Help)
                os2_mi.afStyle|=MIS_HELP;
        if(edit_mi.SysCommand)
                os2_mi.afStyle|=MIS_SYSCOMMAND;
        switch(edit_mi.ItemBreak) {
                case MenuItem::ib_nobreak:
                        break;
                case MenuItem::ib_break:
                        os2_mi.afStyle|=MIS_BREAK;
                case MenuItem::ib_breakseparator:
                        os2_mi.afStyle|=MIS_BREAKSEPARATOR;
                case MenuItem::ib_buttonseparator:
                        os2_mi.afStyle|=MIS_BUTTONSEPARATOR;
        }
        if(edit_mi.EditType==MenuItem::et_submenu)
                os2_mi.afStyle|=MIS_SUBMENU;

        if(edit_mi.Checked)
                os2_mi.afAttribute|=MIA_CHECKED;
        if(edit_mi.Disabled)
                os2_mi.afAttribute|=MIA_DISABLED;
        if(edit_mi.Framed)
                os2_mi.afAttribute|=MIA_FRAMED;
        if(edit_mi.NoDismiss)
                os2_mi.afAttribute|=MIA_NODISMISS;
}


static HWND CreateSubmenu(const MenuItemList &mil, int *index, ULONG style, HWND hwndParent, HWND hwndOwner) {
        HWND hwndMenu=WinCreateWindow(hwndParent,       //parent
                                      WC_MENU,
                                      "",               //no text
                                      style|WS_VISIBLE,
                                      0,0,              //x,y
                                      0,0,              //cx,cy
                                      hwndOwner,        //owner
                                      HWND_TOP,
                                      *index+1000,      //id
                                      NULL,             //pCtlData
                                      NULL              //pPresParam
                                     );
        if(!hwndMenu) return NULL;

        for(;;) {
                const MenuItem &mi=mil.Get(*index);
                ++*index;
                if(mi.EditType==MenuItem::et_endsubmenu)
                        break;  //finished

                MENUITEM os2_mi;
                const char *pszItemText=mi.ItemText;
                GenerateItem(os2_mi,mi);
                os2_mi.iPosition=MIT_END;
                os2_mi.id=(USHORT)(*index+1000);
                if(mi.EditType==MenuItem::et_submenu) 
                        os2_mi.hwndSubMenu=CreateSubmenu(mil,index,0,hwndParent,hwndOwner);
                else
                        os2_mi.hwndSubMenu=NULL;
                os2_mi.hItem=NULL;

                WinSendMsg(hwndMenu, MM_INSERTITEM, MPFROMP(&os2_mi), MPFROMP(pszItemText));
        } 
        return hwndMenu;
}

HWND GenerateMenu(const MenuItemList &mil, HWND hwndFrame, int popup) {
        int index=1;
        HWND hwndMenu;
        if(popup)
                hwndMenu=CreateSubmenu(mil,&index,0,            HWND_OBJECT, hwndFrame);
        else
                hwndMenu=CreateSubmenu(mil,&index,MS_ACTIONBAR, HWND_OBJECT, hwndFrame);
        if(hwndMenu) {
                if(!popup) {
                        WinSetWindowUShort(hwndMenu,QWS_ID,FID_MENU);
                        WinSetParent(hwndMenu,hwndFrame,FALSE);
                }
        }
        return hwndMenu;
}

