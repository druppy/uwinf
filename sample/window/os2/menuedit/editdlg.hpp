#ifndef __EDITDLG_HPP
#define __EDITDLG_HPP

#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_CONTROLS
#define BIFINCL_SLE
#define BIFINCL_CHECKBOX
#define BIFINCL_RADIOBUTTON
#define BIFINCL_PUSHBUTTON
#define BIFINCL_LISTBOX
#define BIFINCL_MESSAGEBOX
#include <bif.h>
#include "menuitem.hpp"

class MenuEditDialog : public FDialog,
                       public FOwnerNotifyHandler
{
        FSLE                         c_MenuItemText;
        FSLE                         c_MenuItemID;
        FRadioButton                 c_Text;
        FRadioButton                 c_Bitmap;
        FRadioButton                 c_Separator;
        FRadioButton                 c_OwnerDraw;
        FCheckBox                    c_Static;
        FCheckBox                    c_Help;
        FCheckBox                    c_SysCommand;
        FRadioButton                 c_NoBreak;
        FRadioButton                 c_Break;
        FRadioButton                 c_BreakSeparator;
        FRadioButton                 c_ButtonSeparator;
        FCheckBox                    c_Checked;
        FCheckBox                    c_Disabled;
        FCheckBox                    c_Framed;
        FCheckBox                    c_NoDismiss;
        FSingleSelectionTextListBox  c_MenuItems;
        FPushButton                  c_MoveUp;
        FPushButton                  c_MoveDown;
        FPushButton                  c_InsertItem;
        FPushButton                  c_InsertSubmenu;
        FPushButton                  c_InsertSeparator;
        FPushButton                  c_DeleteItem;
        FPushButton                  c_TestMenu;
        FPushButton                  c_TestPopupMenu;

public:
        MenuEditDialog();
        ~MenuEditDialog();

        Bool Create(FWnd *pOwner);

        Bool LoadMenu(const char *filename);

protected:
        //overrides
        Bool CreateControls();

        Bool Command(FCommandEvent&);
        Bool Notify(FOwnerNotifyEvent&);
        Bool Close(FEvent&);
        Bool CloseUnconditionally(FEvent&);

private:
        //actions
        void MoveUp();
        void MoveDown();
        void InsertItem();
        void InsertSubmenu();
        void InsertSeparator();
        void DeleteItem();
        void TestMenu();
        void TestPopupMenu();

        void insertStandardSubmenu(const MenuItem *, int items);
        void InsertStandardFileMenu();
        void InsertStandardEditMenu();
        void InsertStandardHelpMenu();

        void CheckDuplicates();

        Bool NewMenu();
        Bool LoadMenu();
        Bool SaveMenu();
        Bool SaveAsMenu();
        mbid WannaSave();

        int ignoreSelections;           //if non-zero ignore selection notification from listbox

        //members
        MenuItemList itemList;          //menu
        int currentItem;                //currently selected item in listbox
        Bool changed;                   //has menu been modified?
        char menuname[256];

        //helpers
        void ImportCurrent();           //move from list to controls
        Bool ExportCurrent();           //move from controls to list
        void UpdatePushButtons();       //enable/disable
        void SelectItem(int index);     //select a item in the listbox
        void UpdateItem(int index);     //update menu item in listbox
        Bool RefillListBox();           //build listbox from ground up

};

#endif
