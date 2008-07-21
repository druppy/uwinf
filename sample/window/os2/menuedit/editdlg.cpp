#define BIFINCL_APPLICATION
#define BIFINCL_STDFILEDIALOG
#include "editdlg.hpp"
#include "menuedit.h"
#include "savemenu.hpp"
#include "loadmenu.hpp"
#include "testmenu.hpp"
#include <string.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include "menuedit.idh"
#include "menuedit.mid"
#include "prodinfo.hpp"

//standard items to insert---------------------------------------------------
static const MenuItem default_StandardItem= {
        MenuItem::et_item,
        "Item",                   //text
        "9999",                   //id
        MenuItem::it_text,        //type
        0,                        //not static
        0,                        //not help
        0,                        //not syscommand
        MenuItem::ib_nobreak,        //break
        0,                        //not checked
        0,                        //not disabled
        0,                        //not framed
        0                         //dismiss
};

static const MenuItem default_SeparatorItem= {
        MenuItem::et_item,
        "",                       //text
        "",                       //id
        MenuItem::it_separator,   //type
        0,                        //not static
        0,                        //not help
        0,                        //not syscommand
        MenuItem::ib_nobreak,     //break
        0,                        //not checked
        0,                        //not disabled
        0,                        //not framed
        0                         //dismiss
};

static const MenuItem default_WholeMenuItem= {
        MenuItem::et_submenu,
        "",                       //no text
        "9999",                   //id
        MenuItem::it_text,        //type
        0,                        //not static
        0,                        //not help
        0,                        //not syscommand
        MenuItem::ib_nobreak,     //break
        0,                        //not checked
        0,                        //not disabled
        0,                        //not framed
        0                         //dismiss
};

static const MenuItem default_EndSubmenuItem= {
        MenuItem::et_endsubmenu,        //the rest does not apply
        "",                       //text
        "",                       //id
        MenuItem::it_text,        //type
        0,                        //not static
        0,                        //not help
        0,                        //not syscommand
        MenuItem::ib_nobreak,     //break
        0,                        //not checked
        0,                        //not disabled
        0,                        //not framed
        0                         //dismiss
};



//MenuEditDialog implementation----------------------------------------------
MenuEditDialog::MenuEditDialog()
  : FOwnerNotifyHandler(this),
    itemList(),
    currentItem(-1),
    ignoreSelections(0)
{
        itemList.insert(0,default_WholeMenuItem);
        itemList.insert(1,default_EndSubmenuItem);
        changed=False;
        menuname[0]='\0';
}


MenuEditDialog::~MenuEditDialog()
{
}


Bool MenuEditDialog::Create(FWnd *pOwner) {
        if(!FDialog::Create(pOwner,DIALOG_MENUEDIT))
                return False;
        //since dialogs don't normally have icons we must change that
        FIcon *picon=new FIcon(ICON_MENUEDIT);
        if(picon)
                SetIcon(picon);
        //under os/2 dialogs in resourcen can't have menus, so we have to load the men ourseleves
        FMenu *pmenu=new FMenu(GetHwnd(),MENU_MENUEDIT);
        if(pmenu)
                SetMenu(pmenu);
        return True;
}

Bool MenuEditDialog::CreateControls() {
        int b=1;
        b=b&&c_MenuItemText.Create(this,ID_MENUITEMTEXT);
        b=b&&c_MenuItemID.Create(this,ID_MENUITEMID);
        b=b&&c_Text.Create(this,ID_TEXT);
        b=b&&c_Bitmap.Create(this,ID_BITMAP);
        b=b&&c_Separator.Create(this,ID_SEPARATOR);
        b=b&&c_OwnerDraw.Create(this,ID_OWNERDRAW);
        b=b&&c_Static.Create(this,ID_STATIC);
        b=b&&c_Help.Create(this,ID_HELP);
        b=b&&c_SysCommand.Create(this,ID_SYSCOMMAND);
        b=b&&c_NoBreak.Create(this,ID_NOBREAK);
        b=b&&c_Break.Create(this,ID_BREAK);
        b=b&&c_BreakSeparator.Create(this,ID_BREAKSEPARATOR);
        b=b&&c_ButtonSeparator.Create(this,ID_BUTTONSEPARATOR);
        b=b&&c_Checked.Create(this,ID_CHECKED);
        b=b&&c_Disabled.Create(this,ID_DISABLED);
        b=b&&c_Framed.Create(this,ID_FRAMED);
        b=b&&c_NoDismiss.Create(this,ID_NODISMISS);
        b=b&&c_MenuItems.Create(this,ID_MENUITEMS);
        b=b&&c_MoveUp.Create(this,ID_MOVEUP);
        b=b&&c_MoveDown.Create(this,ID_MOVEDOWN);
        b=b&&c_InsertItem.Create(this,ID_INSERTITEM);
        b=b&&c_InsertSubmenu.Create(this,ID_INSERTSUBMENU);
        b=b&&c_InsertSeparator.Create(this,ID_INSERTSEPARATOR);
        b=b&&c_DeleteItem.Create(this,ID_DELETEITEM);
        b=b&&c_TestMenu.Create(this,ID_TESTMENU);
        b=b&&c_TestPopupMenu.Create(this,ID_TESTPOPUP);
        b=b&&RefillListBox();
        SelectItem(1);
        return (Bool)b;
}



Bool MenuEditDialog::Command(FCommandEvent &ev) {
        switch(ev.GetItem()) {
                case ID_MOVEUP:
                        MoveUp();
                        break;
                case ID_MOVEDOWN:
                        MoveDown();
                        break;
                case ID_INSERTITEM:
                        InsertItem();
                        break;
                case ID_INSERTSUBMENU:
                        InsertSubmenu();
                        break;
                case ID_INSERTSEPARATOR:
                        InsertSeparator();
                        break;
                case ID_DELETEITEM:
                        DeleteItem();
                        break;
                case ID_TESTMENU:
                        TestMenu();
                        break;
                case ID_TESTPOPUP:
                        TestPopupMenu();
                        break;
                case ID_NEWFILE:
                        NewMenu();
                        break;
                case ID_OPENFILE:
                        LoadMenu();
                        break;
                case ID_SAVEFILE:
                        SaveMenu();
                        break;
                case ID_SAVEFILEAS:
                        SaveAsMenu();
                        break;
                case ID_CHECKDUPLICATES:
                        CheckDuplicates();
                        break;
                case ID_STANDARDFILE:
                        InsertStandardFileMenu();
                        break;
                case ID_STANDARDEDIT:
                        InsertStandardEditMenu();
                        break;
                case ID_STANDARDHELP:
                        InsertStandardHelpMenu();
                        break;
                case ID_HELP_INDEX:
                        //
                        break;
                case ID_GENERAL_HELP:
                        //
                        break;
                case ID_USING_HELP:
                        //
                        break;
                case ID_HELP_KEYS:
                        //
                        break;
                case ID_HELPPRODINFO: {
                        ProductInformationDialog dlg;
                        dlg.DoModal(this);
                        break;
                }
                default:
                        return False;        //not understood
        }
        return True;
}


Bool MenuEditDialog::Notify(FOwnerNotifyEvent &ev) {
        if(ev.GetItem()==ID_MENUITEMS) {
                //it's from the item listbox
                if(ev.GetNotifyCode()==LN_SELECT) {
                        //an item has been selected
                        if(ignoreSelections==0) {
                                ++ignoreSelections;
                                  if(ExportCurrent())
                                        UpdateItem(currentItem);
                                  currentItem=c_MenuItems.GetCurrentSelection();
                                  ImportCurrent();
                                  UpdatePushButtons();
                                --ignoreSelections;
                        }
                }
        }
        return False;
}


Bool MenuEditDialog::Close(FEvent&) {
        if(changed) {
                switch(WannaSave()) {
                        case mbid_yes:
                                if(SaveMenu())
                                        break;
                                else
                                        return True;
                        case mbid_no:
                                break;
                        case mbid_cancel:
                                return True;
                }
        }
        GetCurrentApp()->TerminateApp();
        return True;
}

Bool MenuEditDialog::CloseUnconditionally(FEvent &) {
        if(changed) {
                if(menuname[0]=='\0') {
                        //The user looses the changes - not our fault :-)
                } else {
                        ::SaveMenu(menuname,itemList);
                }
        }
        return False;   
}

Bool MenuEditDialog::SaveMenu() {
        if(menuname[0]=='\0')
                return SaveAsMenu();
        else {
                if(::SaveMenu(menuname,itemList)!=0) {
                        MessageBox(this,"An error ocurred while saving the menu",0,mb_ok,mb_error);
                        return False;
                } else {
                        changed=False;
                        return True;
                }
        }
}

Bool MenuEditDialog::NewMenu() {
        if(changed) {
                switch(WannaSave()) {
                        case mbid_yes:
                                if(!SaveMenu())
                                        return False;
                                else
                                        break;
                        case mbid_no:
                                break;
                        case mbid_cancel:
                        default:
                                return False;
                }
        }
        menuname[0]='\0';
        SetSubtitle("Untitled");
        itemList.clear();
        itemList.insert(0,default_WholeMenuItem);
        itemList.insert(1,default_EndSubmenuItem);
        changed=False;
        RefillListBox();
        SelectItem(0);
        return True;
}

Bool MenuEditDialog::SaveAsMenu() {
        char newfilename[256]="*.mnu";
        FStdSaveFileDialog dlg;
        Bool b=dlg.DoModal(this,newfilename);
        if(!b) 
                return False;
        else {
                strcpy(menuname,newfilename);
                SetSubtitle(menuname);
                return SaveMenu();
        }
}

Bool MenuEditDialog::LoadMenu() {
        if(changed) {
                switch(WannaSave()) {
                        case mbid_yes:
                                if(!SaveMenu())
                                        return False;
                                else
                                        break;
                        case mbid_no:
                                break;
                        case mbid_cancel:
                                return False;
                }
        }

        char filename[256];
        if(menuname[0]) {
                char drive[_MAX_DRIVE],dir[_MAX_DIR],name[_MAX_FNAME],ext[_MAX_EXT];
                _splitpath(menuname,drive,dir,name,ext);
                _makepath(filename,drive,dir,"*",".mnu");
        } else {
                strcpy(filename,"*.mnu");
        }
        FStdOpenFileDialog dlg;
        Bool b=dlg.DoModal(this,filename);
        if(!b) return False;

        return LoadMenu(filename);
}

mbid MenuEditDialog::WannaSave() {
        return MessageBox(this,"The menu has been modified. Do you want to save it?","Save",mb_yesnocancel);
}

Bool MenuEditDialog::LoadMenu(const char *filename) {
        MenuItemList mil;
        if(::LoadMenu(filename,mil)!=0) {
                MessageBox(this,"An error ocurred while loading the menu",0,mb_ok,mb_error);
                return False;
        } else {
                strcpy(menuname,filename);
                SetSubtitle(menuname);
                itemList=mil;
                RefillListBox();
                ImportCurrent();
                SelectItem(0);
                changed=False;
                return True;
        }
}

//actions---
void MenuEditDialog::MoveUp() {
        if(currentItem==-1)
                return;        //no item to move up
        if(currentItem==0)
                return;        //cant move the whole menu up
        if(currentItem==1)
                return;        //cant move above the menu
        switch(itemList[currentItem].EditType) {
                case MenuItem::et_item: {
                        //Move the normal item up
                        MenuItem tmp=itemList[currentItem-1];
                        itemList[currentItem-1]=itemList[currentItem];
                        itemList[currentItem]=tmp;
                        ++ignoreSelections;
                          UpdateItem(currentItem-1);
                          UpdateItem(currentItem);
                          SelectItem(currentItem-1);
                        --ignoreSelections;
                        break;
                }

                case MenuItem::et_submenu: {
                        //Move the whole submenu up
                        //find the end of the submenu
                        for(int endIndex=currentItem, level=0;
                            ;
                            endIndex++)
                        {
                                switch(itemList[endIndex].EditType) {
                                        case MenuItem::et_submenu:
                                                level++; break;
                                        case MenuItem::et_endsubmenu:
                                                level--; break;
                                }
                                if(level==0) break;
                        }
                        //move the items
                        MenuItem first=itemList[currentItem-1];
                        for(int i=currentItem-1; i<endIndex; i++)
                                itemList[i]=itemList[i+1];
                        itemList[endIndex]=first;
                        //update the listbox
                        ++ignoreSelections;
                          RefillListBox();
                          SelectItem(currentItem-1);
                        --ignoreSelections;
                        break;

                }

                case MenuItem::et_endsubmenu:
                        return;        //cant be done
        }
        changed=True;
}

void MenuEditDialog::MoveDown() {
        if(currentItem==-1)
                return;        //no item to move down
        if(currentItem==0)
                return;        //cant move the whole menu down
        if(currentItem==itemList.count()-2)
                return;        //cant move below the menu
        switch(itemList[currentItem].EditType) {
                case MenuItem::et_item: {
                        //Move the normal item down
                        MenuItem tmp=itemList[currentItem];
                        itemList[currentItem]=itemList[currentItem+1];
                        itemList[currentItem+1]=tmp;
                        ++ignoreSelections;
                          UpdateItem(currentItem);
                          UpdateItem(currentItem+1);
                          SelectItem(currentItem+1);
                        --ignoreSelections;
                        break;
                }

                case MenuItem::et_submenu: {
                        //Move the whole submenu down
                        //find the end of the submenu
                        for(int endIndex=currentItem, level=0;
                            ;
                            endIndex++)
                        {
                                switch(itemList[endIndex].EditType) {
                                        case MenuItem::et_submenu:
                                                level++; break;
                                        case MenuItem::et_endsubmenu:
                                                level--; break;
                                }
                                if(level==0) break;
                        }
                        if(endIndex==itemList.count()-2)
                                return; //cant move down
                        //move the items
                        MenuItem last=itemList[endIndex+1];
                        for(int i=endIndex+1; i>currentItem; i--)
                                itemList[i]=itemList[i-1];
                        itemList[currentItem]=last;
                        //update the listbox
                        ++ignoreSelections;
                          RefillListBox();
                          SelectItem(currentItem+1);
                        --ignoreSelections;
                        break;
                }

                case MenuItem::et_endsubmenu:
                        return;        //cant be done
        }
        changed=True;
}


void MenuEditDialog::InsertItem() {
        //insert normal item above the current item
        if(currentItem==-1 ||        //no current item
           currentItem==0)        //no item above the menu
                return;
        itemList.insert(currentItem,default_StandardItem);
        ++ignoreSelections;
          RefillListBox();        //we could do better, but not this time
          SelectItem(currentItem+1);
        --ignoreSelections;
        changed=True;
}

void MenuEditDialog::InsertSubmenu() {
        //insert submenu above the current item
        MenuItem mi[3] = {
           { MenuItem::et_submenu, "sub", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "Item", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_endsubmenu, "", "", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 }
        };
        insertStandardSubmenu(mi,3);
}

void MenuEditDialog::InsertSeparator() {
        //insert separator above the current item
        if(currentItem==-1 ||        //no current item
           currentItem==0)        //no item above the menu
                return;
        itemList.insert(currentItem,default_SeparatorItem);
        ++ignoreSelections;
          RefillListBox();        //we could do better, but not this time
          SelectItem(currentItem+1);
        --ignoreSelections;
        changed=True;
}

void MenuEditDialog::DeleteItem() {
        if(currentItem==-1)
                return;
        switch(itemList[currentItem].EditType) {
                case MenuItem::et_item: {
                        itemList.remove(currentItem);
                        ++ignoreSelections;
                          RefillListBox();
                          SelectItem(currentItem);
                        --ignoreSelections;
                        break;
                }
                case MenuItem::et_submenu: {
                        int level=0;
                        do {
                                if(itemList[currentItem].EditType==MenuItem::et_submenu)
                                        level++;
                                if(itemList[currentItem].EditType==MenuItem::et_endsubmenu)
                                        level--;
                                itemList.remove(currentItem);
                        } while(level>0);
                        ++ignoreSelections;
                          RefillListBox();
                          SelectItem(currentItem);
                        --ignoreSelections;
                        break;
                }
                case MenuItem::et_endsubmenu:
                        return;
        }
        changed=True;
}

void MenuEditDialog::insertStandardSubmenu(const MenuItem *mi, int items) {
        //insert submenu above the current item
        if(currentItem==-1 ||        //no current item
           currentItem==0)        //no item above the menu
                return;
        for(int i=0; i<items; i++)
                itemList.insert(currentItem+i,mi[i]);
        ++ignoreSelections;
          RefillListBox();
          SelectItem(currentItem+items);
        --ignoreSelections;
        changed=True;
}

void MenuEditDialog::InsertStandardFileMenu() {
        MenuItem mi[6] = {
           { MenuItem::et_submenu, "~File", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~New", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~Open...", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~Save", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "Save ~as...", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_endsubmenu, "", "", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 }
        };
        insertStandardSubmenu(mi,6);
}
void MenuEditDialog::InsertStandardEditMenu() {
        MenuItem mi[9] = {
           { MenuItem::et_submenu, "~Edit", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~Undo", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~Redo", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "", "", MenuItem::it_separator, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "Cu~t", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~Copy", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~Paste", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "C~lear", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_endsubmenu, "", "", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 }
        };
        insertStandardSubmenu(mi,9);
}
void MenuEditDialog::InsertStandardHelpMenu() {
        MenuItem mi[8] = {
           { MenuItem::et_submenu, "~Help", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "Help ~index", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~General help", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "Using help", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~Keys help", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "", "", MenuItem::it_separator, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_item, "~Product information", "9999", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 },
           { MenuItem::et_endsubmenu, "", "", MenuItem::it_text, 0,0,0, MenuItem::ib_nobreak, 0,0,0,0 }
        };
        insertStandardSubmenu(mi,8);
}

static Bool CanHaveID(const MenuItem &mi) {
        if(mi.EditType==MenuItem::et_endsubmenu)
                return False;
        if(mi.ItemType==MenuItem::it_separator)
                return False;
        return True;
}

void MenuEditDialog::CheckDuplicates() {
        //we could do better, but not this time...
        ExportCurrent();
        for(int i=1; i<itemList.count(); i++) {
                if(CanHaveID(itemList[i])) {
                        for(int j=i+1; j<itemList.count(); j++) {
                                if(CanHaveID(itemList[j])) {
                                        if(strcmp(itemList[i].ItemID,itemList[j].ItemID)==0) {
                                                char msg[256];
                                                sprintf(msg,
                                                        "Item \"%s\" clashes with item \"%s\" (ID: %s)\nEdit the item?",
                                                        itemList[i].ItemText,
                                                        itemList[j].ItemText,
                                                        itemList[i].ItemID);
                                                SelectItem(j);
                                                if(MessageBox(this,msg,"Duplicate found",mb_yesno)==mbid_yes) {
                                                         return;
                                                }
                                        }
                                }
                        }
                }
        }
}

void MenuEditDialog::TestMenu() {
        MenuTestWindow dlg;
        dlg.DoModal(this,itemList);
}

void MenuEditDialog::TestPopupMenu() {
        PopupMenuTestWindow dlg;
        dlg.DoModal(this,itemList);
}


//helpers---
void MenuEditDialog::ImportCurrent() {
        if(currentItem==-1 ||
           itemList[currentItem].EditType==MenuItem::et_endsubmenu)
        {
                //when no item is selected or it's our pseudo endsubmenu items
                //it can't be edited
                //Disable all items and empty the entryfields
                c_MenuItemText.SetText("");
                c_MenuItemText.Disable();
                c_MenuItemID.SetText("");
                c_MenuItemID.Disable();
                c_Text.Disable();
                c_Bitmap.Disable();
                c_Separator.Disable();
                c_OwnerDraw.Disable();
                c_Static.Disable();
                c_Help.Disable();
                c_SysCommand.Disable();
                c_NoBreak.Disable();
                c_Break.Disable();
                c_BreakSeparator.Disable();
                c_ButtonSeparator.Disable();
                c_Checked.Disable();
                c_Disabled.Disable();
                c_Framed.Disable();
                c_NoDismiss.Disable();
                return;
        }

        MenuItem &mi=itemList[currentItem];
        //enable all controls
        c_MenuItemText.Enable();
        c_MenuItemID.Enable();
        c_Text.Enable();
        c_Bitmap.Enable();
        c_Separator.Enable();
        c_OwnerDraw.Enable();
        c_Static.Enable();
        c_Help.Enable();
        c_SysCommand.Enable();
        c_NoBreak.Enable();
        c_Break.Enable();
        c_BreakSeparator.Enable();
        c_ButtonSeparator.Enable();
        c_Checked.Enable();
        c_Disabled.Enable();
        c_Framed.Enable();
        c_NoDismiss.Enable();

        if(currentItem==0) {
                //the whole menu is a bit special
                c_MenuItemText.SetText("");
                c_MenuItemText.Disable();
        } else
                c_MenuItemText.SetText(mi.ItemText);
        c_MenuItemID.SetText(mi.ItemID);
        switch(mi.ItemType) {
                case MenuItem::it_text:
                        c_Text.Check();
                        break;
                case MenuItem::it_bitmap:
                        c_Bitmap.Check();
                        break;
                case MenuItem::it_separator:
                        c_Separator.Check();
                        break;
                case MenuItem::it_ownerdraw:
                        c_OwnerDraw.Check();
                        break;
        }
        c_Static.SetCheck(mi.Static);
        c_Help.SetCheck(mi.Help);
        c_SysCommand.SetCheck(mi.SysCommand);

        switch(mi.ItemBreak) {
                case MenuItem::ib_nobreak:
                        c_NoBreak.Check();
                        break;
                case MenuItem::ib_break:
                        c_Break.Check();
                        break;
                case MenuItem::ib_breakseparator:
                        c_BreakSeparator.Check();
                        break;
                case MenuItem::ib_buttonseparator:
                        c_ButtonSeparator.Check();
                        break;
        }
        c_Checked.SetCheck(mi.Checked);
        c_Disabled.SetCheck(mi.Disabled);
        c_Framed.SetCheck(mi.Framed);
        c_NoDismiss.SetCheck(mi.NoDismiss);
}

Bool MenuEditDialog::ExportCurrent() {
        if(currentItem==-1 ||
           itemList[currentItem].EditType==MenuItem::et_endsubmenu)
                return False;
        MenuItem mi=itemList[currentItem];
        c_MenuItemText.GetText(mi.ItemText,MAXITEMTEXT);
        c_MenuItemID.GetText(mi.ItemID,MAXITEMID);
        if(c_Text.IsChecked())
                mi.ItemType=MenuItem::it_text;
        else if(c_Bitmap.IsChecked())
                mi.ItemType=MenuItem::it_bitmap;
        else if(c_Separator.IsChecked())
                mi.ItemType=MenuItem::it_separator;
        else
                mi.ItemType=MenuItem::it_ownerdraw;
        mi.Static=c_Static.GetCheck();
        mi.Help=c_Help.GetCheck();
        mi.SysCommand=c_SysCommand.GetCheck();
        if(c_NoBreak.IsChecked())
                mi.ItemBreak=MenuItem::ib_nobreak;
        else if(c_Break.IsChecked())
                mi.ItemBreak=MenuItem::ib_break;
        else if(c_BreakSeparator.IsChecked())
                mi.ItemBreak=MenuItem::ib_breakseparator;
        else
                mi.ItemBreak=MenuItem::ib_buttonseparator;
        mi.Checked=c_Checked.GetCheck();
        mi.Disabled=c_Disabled.GetCheck();
        mi.Framed=c_Framed.GetCheck();
        mi.NoDismiss=c_NoDismiss.GetCheck();
        if(memcmp(&mi,&itemList[currentItem],sizeof(mi))!=0) {
                itemList[currentItem]=mi;
                changed=True;
                return True;
        } else
                return False;
}


void MenuEditDialog::UpdatePushButtons() {
        if(currentItem==-1 || currentItem==0) {
                //no item or top of menu
                c_MoveUp.Disable();
                c_MoveDown.Disable();
                c_InsertItem.Disable();
                c_InsertSubmenu.Disable();
                c_InsertSeparator.Disable();
                c_DeleteItem.Disable();
        } else if(currentItem==itemList.count()-1) {
                //end of menu
                c_MoveDown.Disable();
                c_MoveUp.Disable();
                c_InsertItem.Enable();
                c_InsertSubmenu.Enable();
                c_InsertSeparator.Enable();
                c_DeleteItem.Disable();
        } else if(itemList[currentItem].EditType==MenuItem::et_endsubmenu) {
                //endmenu pseudo item
                c_MoveDown.Disable();
                c_MoveUp.Disable();
                c_InsertItem.Enable();
                c_InsertSubmenu.Enable();
                c_InsertSeparator.Enable();
                c_DeleteItem.Disable();
        } else {
                //normal item
                c_MoveDown.Enable();
                c_MoveUp.Enable();
                c_InsertItem.Enable();
                c_InsertSubmenu.Enable();
                c_InsertSeparator.Enable();
                c_DeleteItem.Enable();
        }
        if(itemList.count()>2) {
                //it's possible to test the menu
                c_TestMenu.Enable();
                c_TestPopupMenu.Enable();
        } else {
                c_TestMenu.Disable();
                c_TestPopupMenu.Disable();
        }
}


void MenuEditDialog::SelectItem(int index) {
        c_MenuItems.SetCurrentSelection(index);
        currentItem=index;
        ImportCurrent();
        UpdatePushButtons();
}

void MenuEditDialog::UpdateItem(int index) {
        if(index==-1) return;
        //calculate indentation
        int level=0;
        for(int i=0; i<index; i++) {
                if(itemList[i].EditType==MenuItem::et_submenu)
                        level++;
                if(itemList[i].EditType==MenuItem::et_endsubmenu)
                        level--;
        }
        if(itemList[index].EditType==MenuItem::et_endsubmenu)
                level--;

        //build string
        char s[MAXITEMTEXT+128];        //should be enough
        s[0]='\0';
        while(level--)
                strcat(s,"  ");
        switch(itemList[index].EditType) {
                case MenuItem::et_submenu:
                        if(index==0) {
                                strcat(s,"Menu ");
                        } else {
                                strcat(s,"Submenu ");
                                strcat(s,itemList[index].ItemText);
                                strcat(s,",");
                        }
                        strcat(s,itemList[index].ItemID);
                        break;
                case MenuItem::et_endsubmenu:
                        strcat(s,"End");
                        break;
                case MenuItem::et_item:
                        if(itemList[index].ItemType==MenuItem::it_separator)
                                strcat(s,"-separator-");
                        else {
                                strcat(s,"Item ");
                                strcat(s,itemList[index].ItemText);
                                strcat(s,",");
                                strcat(s,itemList[index].ItemID);
                        }
                        break;
        }

        //replace the item
        ++ignoreSelections;
          c_MenuItems.RemoveItem(index);
          c_MenuItems.InsertItem(index,s);
          if(ignoreSelections==1) 
                c_MenuItems.SetCurrentSelection(currentItem);
        --ignoreSelections;

}

Bool MenuEditDialog::RefillListBox() {
        //hide listbox while adding the items
        c_MenuItems.DisableRedraw();

        Bool b=True;
        c_MenuItems.Clear();
        for(int i=0; i<itemList.count() && b; i++) {
                b=c_MenuItems.AddItem("");
                if(b) UpdateItem(i);
        }

        c_MenuItems.EnableRedraw();
        return b;
}

