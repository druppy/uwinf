/***
Filename: listbox.cpp
Description:
  Implementation FListBox and subclasses
Host:
  BC40, WC10, SC61
History:
  ISJ 94-11-?? Creation
  ISJ 95-01-10 Added Disable/EnableRedraw()
  ISJ 95-03-03 Made FODListBox constructor non-inline to avoid SC61
               compiler and linker pulling in too much code.
  ISJ 95-03-04 Added ReplaceItem()
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_LISTBOX
#include <bif.h>


//listbox----------------------------------------------------------------------
Bool FListBox::Create(FWnd *pParent, int ID, FRect *rect, long style)
{
        return FControl::Create(pParent,ID,
                                "ListBox","",
                                style|LBS_NOTIFY|WS_VSCROLL,rect);
}

/***
Visibility: public
Description:
Return:
        Number of items in the listbox
***/
int FListBox::GetCount() {
        return (int)SendMessage(GetHwnd(),LB_GETCOUNT,0,0);
}

/***
Visibility: public
Description:
        Remove all items from the listbox
***/
void FListBox::Clear() {
        SendMessage(GetHwnd(),LB_RESETCONTENT,0,0);
}


/***
Visibility: public
Return:
        index of item at the top of the listbox
***/
int FListBox::GetTopIndex() {
        return (int)SendMessage(GetHwnd(),LB_GETTOPINDEX,0,0);
}

/***
Visibility: public
Description:
        Scroll the listbox so that the item <index> is at the top of the listbox
Return:
        success
***/
Bool FListBox::SetTopIndex(int index) {
        LRESULT l=SendMessage(GetHwnd(),LB_SETTOPINDEX,index,0);
        if(l==LB_ERR)
                return False;
        else
                return True;
}

/***
Visibility: public
Description:
        Disable redrawing the listbox each time an item is added/changed/deleted
        This is very useful when adding a lot of items to a listbox
        The redrawing is enabled againg with EnableRedraw()
***/
void FListBox::DisableRedraw() {
        SendMessage(GetHwnd(), WM_SETREDRAW, 0, 0);
}

/***
Visibility: public
Description:
        Enable redrawing of the listbox.
***/
void FListBox::EnableRedraw() {
        SendMessage(GetHwnd(), WM_SETREDRAW, 1, 0);
        InvalidateRect(GetHwnd(), NULL, TRUE);
}

//text listbox
Bool FTextListBox::Create(FWnd *pParent, int ID, FRect *rect, Bool sorted)
{
        long style=0;
        if(sorted) style|=LBS_SORT;
        return FListBox::Create(pParent,ID,rect,style);
}

Bool FTextListBox::AddItem(const char *pszText) {
        LRESULT l=SendMessage(GetHwnd(),LB_ADDSTRING,0,(LPARAM)(LPSTR)pszText);
        if(l<0) return False;
        else return True;
}

Bool FTextListBox::InsertItem(int index, const char *pszText) {
        LRESULT l=SendMessage(GetHwnd(),LB_INSERTSTRING,index,(LPARAM)(LPSTR)pszText);
        if(l<0) return False;
        else return True;
}

Bool FTextListBox::GetItemText(int index, char *buffer, int maxbuf) {
        if(GetItemTextLen(index)>maxbuf)
                return False;
        LRESULT l=SendMessage(GetHwnd(),LB_GETTEXT,index,(LPARAM)(LPSTR)buffer);
        if(l<0) return False;
        else return True;
}

int FTextListBox::GetItemTextLen(int index) {
        LRESULT l=SendMessage(GetHwnd(),LB_GETTEXTLEN,index,0);
        if(l<0) return -1;
        else return (int)l;

}

Bool FTextListBox::RemoveItem(int index) {
        LRESULT l=SendMessage(GetHwnd(),LB_DELETESTRING,index,0);
        if(l<0) return False;
        else return True;
}

Bool FTextListBox::ReplaceItem(int index, const char *pszText) {
        LRESULT l;
        l=SendMessage(GetHwnd(),LB_DELETESTRING,index,0);
        if(l==LB_ERR) return False;
        l=SendMessage(GetHwnd(),LB_INSERTSTRING,index,(LPARAM)(LPSTR)pszText);
        if(l<0) return False;
        else return True;
}

int FTextListBox::FindItem(const char *pszText, int startIndex) {
        LRESULT l=SendMessage(GetHwnd(),LB_FINDSTRING,startIndex,(LPARAM)(LPSTR)pszText);
        if(l>=0) return (int)l;
        else return -1;
}


//single selection listbox ----------------------------------------------------
int FSingleSelectionListBox::GetCurrentSelection() {
        LRESULT l=SendMessage(GetHwnd(),LB_GETCURSEL,0,0);
        if(l==LB_ERR) return -1;
        else return (int)l;
}

/***
Visibility: public
Description:
        Select the item <index>. If <index> is -1 no item is selected
***/
Bool FSingleSelectionListBox::SetCurrentSelection(int index) {
        LRESULT l=SendMessage(GetHwnd(),LB_SETCURSEL,index,0);
        if(l==LB_ERR && index!=-1) return False;
        else return True;
}



//multiple selection listbox---------------------------------------------------
Bool FMultipleSelectionListBox::SetSelState(int index, Bool selected) {
        LRESULT l=SendMessage(GetHwnd(),LB_SETSEL,selected,MAKELPARAM(index,0));
        if(l==LB_ERR) return False;
        else return True;
}
Bool FMultipleSelectionListBox::SetSelStateAll(Bool selected) {
        LRESULT l=SendMessage(GetHwnd(),LB_SETSEL,selected,MAKELPARAM(-1,0));
        if(l==LB_ERR) return False;
        else return True;
}

int FMultipleSelectionListBox::GetSelCount() {
        return (int)SendMessage(GetHwnd(),LB_GETSELCOUNT,0,0);
}

Bool FMultipleSelectionListBox::GetSelItems(int index[], int maxitem) {
        LRESULT l=SendMessage(GetHwnd(),LB_GETSELITEMS,maxitem,(LPARAM)(int FAR*)index);
        if(l==LB_ERR) return False;
        else return True;
}




//singelsel text listbox-------------------------------------------------------

FSingleSelectionTextListBox::FSingleSelectionTextListBox()
  : FListBox(), FTextListBox(), FSingleSelectionListBox()
{
}

Bool FSingleSelectionTextListBox::Create(FWnd *pParent, int ID) {
        return FTextListBox::Create(pParent,ID);
}

Bool FSingleSelectionTextListBox::Create(FWnd *pParent, int ID, FRect *rect, Bool sorted) {
        return FListBox::Create(pParent,ID,rect,sorted);
}

//multisel text listbox
FMultipleSelectionTextListBox::FMultipleSelectionTextListBox()
  : FListBox(), FTextListBox(), FMultipleSelectionListBox()
{
}

Bool FMultipleSelectionTextListBox::Create(FWnd *pParent, int ID) {
        return FListBox::Create(pParent,ID);
}


Bool FMultipleSelectionTextListBox::Create(FWnd *pParent, int ID,
                                           FRect *rect,
                                           lbs_multiselectstyle msstyle,
                                           Bool sorted)
{
        long style=0;
        switch(msstyle) {
                case lbs_me_selection:
                        style|= LBS_EXTENDEDSEL;        //this may be configurable in the future
                        break;
                case lbs_multipleselection:
                        style|= LBS_MULTIPLESEL;
                        break;
                case lbs_extendedselection:
                        style|= LBS_EXTENDEDSEL;
                        break;
        }

        if(sorted) style|=LBS_SORT;

        return FListBox::Create(pParent,ID,
                                rect,
                                style
                               );
}


//FODListBox ------------------------------------------------------------------
FODListBox::FODListBox(FOwnerDrawOwner *owner)
  : FListBox(),
    FOwnerDrawOwnee(this,owner)
{}

Bool FODListBox::Create(FWnd *pParent, int ID, FRect *rect) {
        return FListBox::Create(pParent,ID,rect,LBS_OWNERDRAWFIXED);
}


Bool FODListBox::AddItem(long itemData) {
        long i=SendMessage(GetHwnd(),LB_ADDSTRING,0,itemData);
        if(i<0) return False;
        else return True;
}

Bool FODListBox::InsertItem(int index, long itemData) {
        long i=SendMessage(GetHwnd(),LB_INSERTSTRING,index,itemData);
        if(i<0) return False;
        else return True;
}

Bool FODListBox::GetItemData(int index, long *itemData) {
        LRESULT l=SendMessage(GetHwnd(),LB_GETTEXT,index,(LPARAM)(LPSTR)itemData);
        if(l==LB_ERR) return False;
        else return True;
}

Bool FODListBox::RemoveItem(int index) {
        long i=SendMessage(GetHwnd(),LB_DELETESTRING,index,0);
        if(i<0) return False;
        else return True;
}

Bool FODListBox::ReplaceItem(int index, long itemData) {
        LRESULT l=SendMessage(GetHwnd(),LB_DELETESTRING,index,0);
        if(l==LB_ERR) return False;
        SendMessage(GetHwnd(),LB_INSERTSTRING,index,(LPARAM)itemData);
        return True;
}

// FSingleSelectionODListBox --
FSingleSelectionODListBox::FSingleSelectionODListBox(FOwnerDrawOwner *owner)
  : FListBox(), FODListBox(owner), FSingleSelectionListBox()
{
}


Bool FSingleSelectionODListBox::Create(FWnd *pParent, int ID) {
        return FListBox::Create(pParent,ID);
}


Bool FSingleSelectionODListBox::Create(FWnd *pParent, int ID, FRect *rect)
{
        return FListBox::Create(pParent,ID,rect,LBS_OWNERDRAWFIXED);
}

//FMultipleSelectionODListBox
FMultipleSelectionODListBox::FMultipleSelectionODListBox(FOwnerDrawOwner *owner)
  : FListBox(), FODListBox(owner), FMultipleSelectionListBox()
{
}


Bool FMultipleSelectionODListBox::Create(FWnd *pParent, int ID) {
        return FListBox::Create(pParent,ID);
}


Bool FMultipleSelectionODListBox::Create(FWnd *pParent,
                                         int ID,
                                         FRect *rect,
                                         lbs_multiselectstyle msstyle)
{
        long style=LBS_OWNERDRAWFIXED;
        switch(msstyle) {
                case lbs_me_selection:      style|= LBS_EXTENDEDSEL; break;
                case lbs_multipleselection: style|= LBS_MULTIPLESEL; break;
                case lbs_extendedselection: style|= LBS_EXTENDEDSEL; break;
        }

        return FListBox::Create(pParent,ID,
                                rect,
                                style
                               );
}

