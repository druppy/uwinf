/***
Filename: listbox.cpp
Description:
  Implementation FListBox and subclasses
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_LISTBOX
#include <bif.h>


//listbox----------------------------------------------------------------------
Bool FListBox::Create(FWnd *pParent, int ID, FRect *rect, ULONG flStyle)
{
        return FControl::Create(pParent,ID,
                                WC_LISTBOX,"",
                                flStyle,rect);
}

/***
Visibility: public
Description:
        Return the number of items in the listbox
Return:
        number of items in the listbox
***/
int FListBox::GetCount() {
        return SHORT1FROMMR(WinSendMsg(GetHwnd(),LM_QUERYITEMCOUNT,0,0));
}

/***
Visibility: public
Description:
        Remove all items from the listbox
***/
void FListBox::Clear() {
        WinSendMsg(GetHwnd(),LM_DELETEALL,0,0);
}


/***
Visibility: public
Description:
        Return the item index of the first visible item (top item) in the 
        listbox.
Return:
        index of item at the top of the listbox
***/
int FListBox::GetTopIndex() {
        return SHORT1FROMMR(WinSendMsg(GetHwnd(),LM_QUERYTOPINDEX,0,0));
}

/***
Visibility: public
Description:
        Scroll the listbox so that the item <index> is at the top of the listbox
Return:
        success
***/
Bool FListBox::SetTopIndex(int index) {
        return (Bool)WinSendMsg(GetHwnd(),LM_SETTOPINDEX,MPFROMSHORT(index),0);
}


/***
Visibility: public
Description:
        Disable further redrawing of the listbox. This function should be 
        called before larger number of items is to be added to the listbox
***/
void FListBox::DisableRedraw() {
        WinEnableWindowUpdate(GetHwnd(),FALSE);
}

/***
Visibility: public
Description:
        Enable redrawing of the listbox, previously disabled by DisableRedraw. 
        Future changes to the listbox will be cause the listbox to repaint immidiatly.
***/
void FListBox::EnableRedraw() {
        WinShowWindow(GetHwnd(),TRUE);
}

//text listbox
Bool FTextListBox::Create(FWnd *pParent, int ID, FRect *rect, Bool sorted)
{
        sortItems=sorted;
        return FListBox::Create(pParent,ID,rect,0);
}

/***
Visibility: public
Description:
        append a string item to the listbox
Parameters:
        pszText   item text
Return:
        True on success
***/
Bool FTextListBox::AddItem(const char *pszText) {
        MRESULT rc;
        if(sortItems)
                rc=WinSendMsg(GetHwnd(),LM_INSERTITEM,MPFROMSHORT(LIT_SORTASCENDING),MPFROMP(pszText));
        else
                rc=WinSendMsg(GetHwnd(),LM_INSERTITEM,MPFROMSHORT(LIT_END),MPFROMP(pszText));

        if(SHORT1FROMMR(rc)==(USHORT)LIT_MEMERROR || SHORT1FROMMR(rc)==(USHORT)LIT_ERROR)
                return False;
        else 
                return True;
}

/***
Visibility: public
Description:
        Insert an item on a specified location. The item is inserted _before_ 
        the specified index.
Parameters:
        index    insert position
        pszText  item text
Return:
***/
Bool FTextListBox::InsertItem(int index, const char *pszText) {
        MRESULT rc;
        if(sortItems)
                rc=WinSendMsg(GetHwnd(),LM_INSERTITEM,MPFROMSHORT(LIT_SORTASCENDING),MPFROMP(pszText));
        else
                rc=WinSendMsg(GetHwnd(),LM_INSERTITEM,MPFROMSHORT(index),MPFROMP(pszText));

        if(SHORT1FROMMR(rc)==(USHORT)LIT_MEMERROR || SHORT1FROMMR(rc)==(USHORT)LIT_ERROR)
                return False;
        else 
                return True;
}

/***
Visibility: public
Description:
        Retrieve the text of an item
Parameters:
        index    item index
        buffer   (output) item text
        maxbuf   size of <buffer>
Return:
        True on success
***/
Bool FTextListBox::GetItemText(int index, char *buffer, int maxbuf) {
        if(GetItemTextLen(index)>maxbuf)
                return False;
        WinSendMsg(GetHwnd(),LM_QUERYITEMTEXT,MPFROM2SHORT(index,maxbuf),MPFROMP(buffer));
        return True;
}

/***
Visibility: public
Description:
        Retrieve length of the item text
Parameters:
        indexx  item index
Return:
        non-negative: item text length
        negative: error
***/
int FTextListBox::GetItemTextLen(int index) {
        MRESULT rc=WinSendMsg(GetHwnd(),LM_QUERYITEMTEXTLENGTH,MPFROMSHORT(index),0);
        if(SHORT1FROMMR(rc)==(USHORT)LIT_ERROR)
                return -1;
        else
                return SHORT1FROMMR(rc);
}

/***
Visibility: public
Description:
        Remove an item from the listbox
Parameters:
        index  item index
Return:
        True on success
***/
Bool FTextListBox::RemoveItem(int index) {
        WinSendMsg(GetHwnd(),LM_DELETEITEM,MPFROMSHORT(index),0);
        return True;
}


/***
Visibility: public
Description:
        Replace the text of an item
Parameters:
        index        item whose text is to be replaced
        pszText
Return:
        success
***/
Bool FTextListBox::ReplaceItem(int index, const char *pszText) {
        return (Bool)WinSendMsg(GetHwnd(), LM_SETITEMTEXT, MPFROMSHORT(index), MPFROMP(pszText));
}

/***
Visibility: public
Description:
        find an item with a specified
Parameters:
        pszText
        startIndex   where to start the search
Return:
        non-negative  index of found item
        negative      error
Note:
        Under os/2 the search is case-sensitive
***/
int FTextListBox::FindItem(const char *pszText, int startIndex) {
        MRESULT rc=WinSendMsg(GetHwnd(),LM_SEARCHSTRING,MPFROM2SHORT(LSS_CASESENSITIVE|LSS_PREFIX,startIndex),MPFROMP(pszText));
        SHORT i=SHORT1FROMMR(rc);
        if(i==LIT_ERROR || i==LIT_NONE)
                return -1;
        else
                return i;
}


//single selection listbox ----------------------------------------------------
int FSingleSelectionListBox::GetCurrentSelection() {
        SHORT i=SHORT1FROMMR(WinSendMsg(GetHwnd(),LM_QUERYSELECTION,0,0));
        if(i==LIT_NONE)
                return -1;
        else
                return i;
}

/***
Visibility: public
Description:
        Select the item <index>. If <index> is -1 no item is selected
***/
Bool FSingleSelectionListBox::SetCurrentSelection(int index) {
        if(index==-1)
                return (Bool)WinSendMsg(GetHwnd(),LM_SELECTITEM,MPFROMSHORT(LIT_NONE),0);
        else
                return (Bool)WinSendMsg(GetHwnd(),LM_SELECTITEM,MPFROMSHORT(index),MPFROMSHORT(TRUE));
}



//multiple selection listbox---------------------------------------------------
Bool FMultipleSelectionListBox::SetSelState(int index, Bool selected) {
        return (Bool)WinSendMsg(GetHwnd(),LM_SELECTITEM,MPFROMSHORT(index),MPFROMSHORT(selected));
}

Bool FMultipleSelectionListBox::SetSelStateAll(Bool selected) {
        if(!selected) {
                return (Bool)WinSendMsg(GetHwnd(),LM_SELECTITEM,MPFROMSHORT(LIT_NONE),0);
        } else {
                //no built-in method to select all items
                int count=GetCount();
                for (int i=0; i<count; i++) {
                        WinSendMsg(GetHwnd(),LM_SELECTITEM,MPFROMSHORT(i),MPFROMSHORT(TRUE));
                } 
                return True;
        }
}

int FMultipleSelectionListBox::GetSelCount() {
        int count=0;
        int i=LIT_FIRST;
        while(i!=LIT_NONE) {
                i=SHORT1FROMMR(WinSendMsg(GetHwnd(),LM_QUERYSELECTION,MPFROMSHORT(i),0));
                if(i!=LIT_NONE)
                        count++;
        }
        return count;
}

Bool FMultipleSelectionListBox::GetSelItems(int index[], int maxitem) {
        int count=0;
        int i=LIT_FIRST;
        while(i!=LIT_NONE && maxitem>0) {
                i=SHORT1FROMMR(WinSendMsg(GetHwnd(),LM_QUERYSELECTION,MPFROMSHORT(i),0));
                if(i!=LIT_NONE) {
                        *index=i;
                        index++;
                }
        }
        return True;
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
        ULONG flStyle=0;
        switch(msstyle) {
                case lbs_me_selection:      
                        flStyle|= LS_EXTENDEDSEL; 
                        break;
                case lbs_multipleselection: 
                        flStyle|= LS_MULTIPLESEL|LS_EXTENDEDSEL; 
                        break;
                case lbs_extendedselection: 
                        flStyle|= LS_EXTENDEDSEL; 
                        break;
        }

        sortItems=sorted;

        return FListBox::Create(pParent,ID,
                                rect,
                                flStyle
                               );
}


//FODListBox ------------------------------------------------------------------
Bool FODListBox::Create(FWnd *pParent, int ID, FRect *rect) {
        SetID(ID);
        return FListBox::Create(pParent,ID,rect,LS_OWNERDRAW);
}

static char empty[]="";

Bool FODListBox::AddItem(long itemData) {
        SHORT i=SHORT1FROMMR(WinSendMsg(GetHwnd(), LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(empty)));
        if(i<0) return False;
        WinSendMsg(GetHwnd(),LM_SETITEMHANDLE,MPFROMSHORT(i),MPFROMLONG(itemData));
        return True;
}

Bool FODListBox::InsertItem(int index, long itemData) {
        SHORT i=SHORT1FROMMR(WinSendMsg(GetHwnd(), LM_INSERTITEM, MPFROMSHORT(index), MPFROMP(empty)));
        if(i<0) return False;
        WinSendMsg(GetHwnd(),LM_SETITEMHANDLE,MPFROMSHORT(i),MPFROMLONG(itemData));
        return True;
}

Bool FODListBox::GetItemData(int index, long *itemData) {
        *itemData=(long)WinSendMsg(GetHwnd(), LM_QUERYITEMHANDLE, MPFROMSHORT(index), 0);
        return True;
}

Bool FODListBox::RemoveItem(int index) {
        WinSendMsg(GetHwnd(), LM_DELETEITEM, MPFROMSHORT(index), 0);
        return True;
}

Bool FODListBox::ReplaceItem(int index, long itemData) {
        return (Bool)WinSendMsg(GetHwnd(), LM_SETITEMHANDLE, MPFROMSHORT(index), MPFROMLONG(itemData));
}


Bool FODListBox::OwneeMeasure(FMeasureItemEvent &ev) {
        ev.SetItemHeight(GetItemHeight());
        return True;
}


// FSingleSelectionODListBox --
FSingleSelectionODListBox::FSingleSelectionODListBox(FOwnerDrawOwner *owner)
  : FListBox(), 
    FODListBox(owner), 
    FSingleSelectionListBox()
{
}


Bool FSingleSelectionODListBox::Create(FWnd *pParent, int ID) {
        return FListBox::Create(pParent,ID);
}


Bool FSingleSelectionODListBox::Create(FWnd *pParent, int ID, FRect *rect)
{
        SetID(ID);
        return FListBox::Create(pParent,ID,rect,LS_OWNERDRAW);
}

//FMultipleSelectionODListBox---
FMultipleSelectionODListBox::FMultipleSelectionODListBox(FOwnerDrawOwner *owner)
  : FListBox(), 
    FODListBox(owner), 
    FMultipleSelectionListBox()
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
        long style=LS_OWNERDRAW;
        switch(msstyle) {
                case lbs_me_selection:      
                        style|= LS_EXTENDEDSEL; 
                        break;
                case lbs_multipleselection: 
                        style|= LS_MULTIPLESEL|LS_EXTENDEDSEL; 
                        break;
                case lbs_extendedselection: 
                        style|= LS_EXTENDEDSEL; 
                        break;
        }

        SetID(ID);
        return FListBox::Create(pParent,ID,
                                rect,
                                style
                               );
}

