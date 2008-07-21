/***
Filename: combobox.cpp
Description:
  Implementation of FCombobox and subclasses
Host:
  Watcom 10.0a
History:
  ISJ 95-06-15 Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_COMBOBOX
#include <bif.h>
//#include <mem.h>
//#include <stdlib.h>
//#include <string.h>


//FComboBox-------------------------------------------------------------------

/***
Visibility: public
Description:
        Return the number of items in the listbox-part of the combobox
***/
int FComboBox::GetCount() {
        return (int)SendMessage(GetHwnd(), CB_GETCOUNT, 0,0);
}

/***
Visibility: public
Description:
        Remove all items from the listbox-part of the combobox
***/
void FComboBox::Clear() {
        SendMessage(GetHwnd(), CB_RESETCONTENT, 0,0);
}


/***
Visibility: public
Parameter:
        index    index of item to remove
Description:
        Remove one item from the listbox
Return:
        True on success
***/
Bool FComboBox::RemoveItem(int index) {
        if(SendMessage(GetHwnd(), CB_DELETESTRING, index, 0)==CB_ERR)
                return False;
        else
                return True;
}

/***
Visibility: public
Parameters:
        index   item to select. If -1 the combobox will not have a selected item
Return:
        True on success
Description:
        This function selects the specified item. The entryfield/static is 
        changed to reflect the new item.
Note:
        This function may cause the dropdown-listbox to drop down
***/
Bool FComboBox::SetCurrentSelection(int index) {
        if(index==-1) {
                SendMessage(GetHwnd(), CB_SETCURSEL, (WPARAM)-1, 0);
                return True;
        } else {
                if(SendMessage(GetHwnd(), CB_SETCURSEL, index,0)==CB_ERR)
                        return False;
                else
                        return True;
        }
}


/***
Visibility: protected (this class is abstract)
Description:
        This function creates the FCombobox interface object to a combobox 
        created from resources
Parameters:
        pParent    parent window
        ID         ID of the combobox in the resources
Return:
        True on success
***/
Bool FComboBox::Create(FWnd *pParent, int ID) {
        return FControl::Create(pParent,ID);
}

/***
Visibility: protected (this class is abstract)
Parameters:
        pParent         parent window
        ID              ID of the combobox
        pszControlText  initial text in entryfield (ignored for certain styles)
        flStyle         combobox style
        rect            combobox rectangle. For dropdown-list comboboxes this 
                        is the total size of the combobox when the list is 
                        dropped down
Return:
        True on success
***/
Bool FComboBox::Create(FWnd *pParent, int ID,
                       const char *pszControlText,
                       DWORD flStyle, FRect *rect
                      )
{
        return FControl::Create(pParent,ID,
                                "COMBOBOX",pszControlText,
                                flStyle,rect
                               );
}



//FComboTextItem_mixin-------------------------------------------------------

Bool FComboTextItem_mixin::InsertItem(int index, const char *pszItem) {
        LRESULT sIndexInserted=SendMessage(combo->GetHwnd(), CB_INSERTSTRING, index, (LPARAM)(LPSTR)pszItem);
        if(sIndexInserted==CB_ERR ||
           sIndexInserted==CB_ERRSPACE)
                return False;
        else
                return True;
}

Bool FComboTextItem_mixin::AddItem(const char *pszItem) {
        LRESULT sIndexInserted=SendMessage(combo->GetHwnd(), CB_ADDSTRING, 0, (LPARAM)(LPSTR)pszItem);
        if(sIndexInserted==CB_ERR ||
           sIndexInserted==CB_ERRSPACE)
                return False;
        else
                return True;
}

Bool FComboTextItem_mixin::GetItemText(int index, char *buffer, int buflen) {
        LRESULT s=SendMessage(combo->GetHwnd(), CB_GETLBTEXTLEN, index,0);
        if(s==CB_ERR || s>buflen)
                return False;
        s=SendMessage(combo->GetHwnd(), CB_GETLBTEXT, index, (LPARAM)(LPSTR)buffer);
        if(s==CB_ERR)
                return False;
        else
                return True;
}

int FComboTextItem_mixin::GetItemTextLen(int index) {
        return (int)SendMessage(combo->GetHwnd(), CB_GETLBTEXTLEN, index,0);
}



//FEditComboBox---------------------------------------------------------------

/***
Visibility: public
Parameters:
        pszText  new text of the entryfield-part of the combobox
Return:
        True on success
***/
Bool FEditComboBox::SetText(const char *pszText) {
        SetWindowText(GetHwnd(), pszText);
        return True;
}

/***
Visibility: public
Parameters:
        buffer  (output) current text in the entryfield-part of the combobox
        bflen   size of <buffer>
Return:
        True on success
***/
Bool FEditComboBox::GetText(char *buffer, int buflen) {
        if(GetWindowText(GetHwnd(), buffer, buflen)==0)
                return False;
        else
                return True;
}

/***
Visibility: public
Return:
        the length of the current text in the entryfield-part of the 
        combobox, including the null terminator
***/
int FEditComboBox::GetTextLength() {
        return GetWindowTextLength(GetHwnd());
}

/***
Visibility: public
Parameters:
        limit   new text limit
Return:
        True on success
Description:
        This function limits the number of characters the user can type 
        in the entryfield-part of the combobox.
***/
Bool FEditComboBox::LimitText(int limit) {
        if(SendMessage(GetHwnd(), CB_LIMITTEXT, limit, 0)!=1)
                return False;
        else
                return True;
}




//FSimpleComboBox------------------------------------------------------------
Bool FSimpleComboBox::Create(FWnd *pParent, int ID,
                             const char *initialtext,
                             FRect *rect
                            )
{
        return FComboBox::Create(pParent,ID,
                                 initialtext,
                                 CBS_SIMPLE, rect
                                );
}


//FSimpleComboBox------------------------------------------------------------
Bool FDropDownComboBox::Create(FWnd *pParent, int ID,
                               const char *initialtext,
                               FRect *rect
                              )
{
        return FComboBox::Create(pParent,ID,
                                 initialtext,
                                 CBS_DROPDOWN, rect
                                );
}



//FSelectionComboBox---------------------------------------------------------
Bool FSelectionComboBox::Create(FWnd *pParent, int ID,
                                DWORD flStyle, FRect *rect
                               )
{
        return FComboBox::Create(pParent,ID,
                                 "",
                                 CBS_DROPDOWNLIST|flStyle, rect);
}

/***
Visibility: public
Return:
        The currently selected item, -1 if none
***/
int FSelectionComboBox::GetCurrentSelection() {
        LRESULT i=SendMessage(GetHwnd(), CB_GETCURSEL, 0, 0);
        if(i==CB_ERR)
                return -1;
        else
                return (int)i;
}




//FODSelectionComboBox--------------------------------------------------------
FODSelectionComboBox::FODSelectionComboBox(FOwnerDrawOwner *powner)
  : FSelectionComboBox(),
    FOwnerDrawOwnee(this,powner)
{
}

FODSelectionComboBox::~FODSelectionComboBox() {
}


Bool FODSelectionComboBox::Create(FWnd *pParent, int ID,
                                  FRect *rect
                                 )
{
        return FSelectionComboBox::Create(pParent,ID,CBS_OWNERDRAWFIXED,rect);
}


Bool FODSelectionComboBox::AddItem(long itemData) {
        LRESULT l=SendMessage(GetHwnd(), CB_ADDSTRING, 0, (LPARAM)itemData);
        if(l==CB_ERR ||
           l==CB_ERRSPACE)
                return False;
        return True;
}

Bool FODSelectionComboBox::InsertItem(int index, long itemData) {
        LRESULT l=SendMessage(GetHwnd(), CB_INSERTSTRING, index, (LPARAM)itemData);
        if(l==CB_ERR ||
           l==CB_ERRSPACE)
                return False;
        return True;
}

Bool FODSelectionComboBox::GetItemData(int index, long *itemData) {
        LRESULT l=SendMessage(GetHwnd(), CB_GETLBTEXT, index, (LPARAM)(LPSTR)itemData);
        if(l==CB_ERR ||
           l==CB_ERRSPACE)
                return False;
        return True;
}


Bool FODSelectionComboBox::OwneeMeasure(FMeasureItemEvent &ev) {
        ev.SetItemHeight(GetItemHeight());
        return True;
}

