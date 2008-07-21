/***
Filename: combobox.cpp
Description:
  Implementation of FCombobox and subclasses
Host:
  Watcom 10.0a-11.0, VAC 3.0
History:
  ISJ 95-01-25 Creation
  ISJ 95-06-15 Changed GetTextLength to GetTextLen
  ISJ 97-06-28 Removed <mem.h> reference - memset() is in string.h
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_COMBOBOX
#define INCL_GPIPRIMITIVES
#include <bif.h>
#include <stdlib.h>
#include <string.h>


//FComboBox-------------------------------------------------------------------

/***
Visibility: public
Description:
        Return the number of items in the listbox-part of the combobox
***/
int FComboBox::GetCount() {
        return SHORT1FROMMR(WinSendMsg(GetHwnd(), LM_QUERYITEMCOUNT, 0 ,0));
}

/***
Visibility: public
Description:
        Remove all items from the listbox-part of the combobox
***/
void FComboBox::Clear() {
        WinSendMsg(GetHwnd(), LM_DELETEALL, 0, 0);
}


/***
Visibility: public
Return:
        -1     no items in the listbox-part
        other  index of the first visibel item in the listbox
***/
int FComboBox::GetTopIndex() {
        MRESULT mr=WinSendMsg(GetHwnd(), LM_QUERYTOPINDEX, 0, 0);
        if((SHORT)SHORT1FROMMR(mr)==LIT_NONE)
                return -1;
        else
                return SHORT1FROMMR(mr);
}

/***
Visibility: public
Parameters:
        index    index of the new top-item
Return:
        True on success
Note:
        The listbox-part is scrolled as necessary
***/
Bool FComboBox::SetTopIndex(int i) {
        return (Bool)WinSendMsg(GetHwnd(), LM_SETTOPINDEX, MPFROMSHORT(i), 0);
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
        WinSendMsg(GetHwnd(), LM_DELETEITEM, MPFROMSHORT(index), 0);
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
        if(index==-1)
                return (Bool)WinSendMsg(GetHwnd(), LM_SELECTITEM, MPFROMSHORT(LIT_NONE), 0);
        else
                return (Bool)WinSendMsg(GetHwnd(), LM_SELECTITEM, MPFROMSHORT(index), MPFROMSHORT(TRUE));
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
                       ULONG flStyle, FRect *rect
                      )
{
        return FControl::Create(pParent,ID,
                                WC_COMBOBOX,pszControlText,
                                flStyle,rect
                               );
}



//FComboTextItem_mixin-------------------------------------------------------

Bool FComboTextItem_mixin::InsertItem(int index, const char *pszItem) {
        SHORT sIndexInserted=(SHORT)SHORT1FROMMR(WinSendMsg(combo->GetHwnd(), LM_INSERTITEM, MPFROMSHORT(index), MPFROMP(pszItem)));
        if(sIndexInserted==LIT_MEMERROR ||
           sIndexInserted==LIT_ERROR)
                return False;
        else
                return True;
}

Bool FComboTextItem_mixin::AddItem(const char *pszItem) {
        SHORT sIndexInserted=(SHORT)SHORT1FROMMR(WinSendMsg(combo->GetHwnd(), LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(pszItem)));
        if(sIndexInserted==LIT_MEMERROR ||
           sIndexInserted==LIT_ERROR)
                return False;
        else
                return True;
}

Bool FComboTextItem_mixin::GetItemText(int index, char *buffer, int buflen) {
        SHORT s=(SHORT)SHORT1FROMMR(WinSendMsg(combo->GetHwnd(), LM_QUERYITEMTEXT, MPFROM2SHORT(index,buflen), MPFROMP(buffer)));
        if(s<=0)
                return False;
        else
                return True;
}

int FComboTextItem_mixin::GetItemTextLen(int index) {
        return SHORT1FROMMR(WinSendMsg(combo->GetHwnd(), LM_QUERYITEMTEXTLENGTH, MPFROMSHORT(index), 0));
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
        return (Bool)WinSetWindowText(GetHwnd(), pszText);
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
        WinQueryWindowText(GetHwnd(), buflen, buffer);
        return True;
}

/***
Visibility: public
Return:
        the length of the current text in the entryfield-part of the 
        combobox, including the null terminator
***/
int FEditComboBox::GetTextLen() {
        return WinQueryWindowTextLength(GetHwnd());
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
        return (Bool)WinSendMsg(GetHwnd(), EM_SETTEXTLIMIT, MPFROMSHORT(limit), 0);
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
                                FRect *rect
                               )
{
        return FComboBox::Create(pParent,ID,
                                 "",
                                 CBS_DROPDOWNLIST, rect);
}

/***
Visibility: public
Return:
        The currently selected item, -1 if none
***/
int FSelectionComboBox::GetCurrentSelection() {
        SHORT i=(SHORT)SHORT1FROMMR(WinSendMsg(GetHwnd(), LM_QUERYSELECTION, 0, 0));
        if(i==LIT_NONE)
                return -1;
        else
                return i;
}




//FODSelectionComboBox--------------------------------------------------------
class FODComboBox_ef_handler : public FHandler {
public:
        FODComboBox_ef_handler(FWnd *pwnd)
          : FHandler(pwnd)
          { SetDispatch(dispatchFunc_t(&FODComboBox_ef_handler::Dispatch)); }
        Bool Dispatch(FEvent&);
        virtual void DoDefaultAndRepaint(FEvent&)=0;
};

Bool FODComboBox_ef_handler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_SETWINDOWPARAMS:
                case EM_SETFIRSTCHAR:
                case EM_SETSEL:
                case WM_SETFOCUS: 
                case WM_PAINT:
                        //the entryfield repaints immidiatly on these messages
                        DoDefaultAndRepaint(ev);
                        return True;
                default:
                        return False;
        }
}


class FODComboBox_ef : public FControl, public FODComboBox_ef_handler
{
        FODSelectionComboBox *combo;
public:
        FODComboBox_ef(FODSelectionComboBox *c)
          : FControl(),
            FODComboBox_ef_handler(this)
          { combo=c; }
        void DoDefaultAndRepaint(FEvent &ev);
};

void FODComboBox_ef::DoDefaultAndRepaint(FEvent &ev) {
        Default(ev);

        OWNERITEM oi;
        memset(&oi,0,sizeof(oi));
        oi.hwnd = GetHwnd();
        oi.hps = WinGetPS(GetHwnd());
        oi.fsState=FALSE;
        oi.fsStateOld=FALSE;
        WinQueryWindowRect(GetHwnd(),&oi.rclItem);
        oi.idItem = (LONG)combo->GetCurrentSelection();
        combo->GetItemData(oi.idItem,(long*)&oi.hItem);

        WinSendMsg(WinQueryWindow(GetHwnd(),QW_OWNER), WM_DRAWITEM, MPFROMSHORT(CBID_EDIT), MPFROMP(&oi));
        if(oi.fsState!=oi.fsStateOld) {
                //invert item rect
                WinInvertRect(oi.hps,&oi.rclItem);
        }
        //draw focus rect if we have the focus
        if(WinQueryFocus(HWND_DESKTOP)==GetHwnd() && !(ev.GetID()==WM_SETFOCUS && ev.mp2==0)) {
                GpiSetMix(oi.hps,FM_XOR);
                GpiSetBackMix(oi.hps,FM_LEAVEALONE);
                GpiSetLineType(oi.hps,LINETYPE_ALTERNATE);
                GpiSetColor(oi.hps,CLR_WHITE);

                POINTL p[4];
                p[0].x=oi.rclItem.xLeft;
                p[0].y=oi.rclItem.yBottom;
                p[1].x=oi.rclItem.xLeft;
                p[1].y=oi.rclItem.yTop-1;
                p[2].x=oi.rclItem.xRight-1;
                p[2].y=oi.rclItem.yTop-1;
                p[3].x=oi.rclItem.xRight-1;
                p[3].y=oi.rclItem.yBottom;

                GpiMove(oi.hps,&p[3]);
                GpiPolyLine(oi.hps,4,p);
        }
        WinReleasePS(oi.hps);
}



FODComboBox_forwarder::FODComboBox_forwarder(FWnd *pwnd)
 : FHandler(pwnd)
{
        SetDispatch(dispatchFunc_t(&FODComboBox_forwarder::Dispatch));
}

Bool FODComboBox_forwarder::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_DRAWITEM) {
                FDrawItemEvent &die=(FDrawItemEvent&)ev;
                if(die.GetControlID()==CBID_LIST ||
                   die.GetControlID()==CBID_EDIT)
                {
                        //forward the event to the owner pretending it was sent
                        //from the combobox itself
                        HWND hwndCBOwner=WinQueryWindow(GetWnd()->GetHwnd(),QW_OWNER);
                        USHORT cbID=WinQueryWindowUShort(GetWnd()->GetHwnd(), QWS_ID);
                        MRESULT mr=WinSendMsg(hwndCBOwner, WM_DRAWITEM, MPFROMSHORT(cbID), ev.mp2);
                        ev.SetResult(mr);
                        return True;
                } else
                        return False;
        } else if(ev.GetID()==WM_MEASUREITEM) {
                FMeasureItemEvent &mie=(FMeasureItemEvent&)ev;
                if(mie.GetControlID()==CBID_LIST ||
                   mie.GetControlID()==CBID_EDIT)
                {
                        //forward the event to the owner pretending it was sent
                        //from the combobox itself
                        HWND hwndCBOwner=WinQueryWindow(GetWnd()->GetHwnd(),QW_OWNER);
                        USHORT cbID=WinQueryWindowUShort(GetWnd()->GetHwnd(), QWS_ID);
                        MRESULT mr=WinSendMsg(hwndCBOwner, WM_MEASUREITEM, MPFROMSHORT(cbID), ev.mp2);
                        ev.SetResult(mr);
                        return True;
                } else
                        return False;
        } else
                return False;
}




static Bool isComboBoxListBox(HWND hwndCombo, HWND hwndList) {
        //it <hwndList> the listbox of the combobox?
        if(WinQueryWindow(hwndList,QW_OWNER) != hwndCombo)
                return False;   //combobox must be owner
        if(WinQueryWindowUShort(hwndList, QWS_ID) != CBID_LIST)
                return False;   //
        return True;
}

static HWND findComboBoxListBox(HWND hwndCombo) {
        //At least under warp, window handles are sequential, so a good guess is hwndCombo+1
        if(isComboBoxListBox(hwndCombo,hwndCombo+1))
                return hwndCombo+1;
        
        //When the listbox is not dropped down it is an object window
        HENUM henumObject=WinBeginEnumWindows(HWND_OBJECT);
        for(HWND hwndObj=WinGetNextWindow(henumObject); hwndObj; hwndObj=WinGetNextWindow(henumObject)) {
                if(isComboBoxListBox(hwndCombo,hwndObj)) {
                        WinEndEnumWindows(henumObject);
                        return hwndObj;
                }
        }
        WinEndEnumWindows(henumObject);

        //When the listbox is dropped down its parent is the same as the combox' parent
        HENUM henumSiblings=WinBeginEnumWindows(WinQueryWindow(hwndCombo,QW_PARENT));
        for(HWND hwndSibling=WinGetNextWindow(henumSiblings); hwndSibling; hwndSibling=WinGetNextWindow(henumSiblings)) {
                if(isComboBoxListBox(hwndCombo,hwndSibling)) {
                        WinEndEnumWindows(henumSiblings);
                        return hwndObj;
                }
        }
        WinEndEnumWindows(henumSiblings);

        //Give up
        return NULLHANDLE;
}

Bool FODSelectionComboBox::MakeItOwnerDraw() {
        //Step 1: find the listbox in the combobox
        hwndListBox=findComboBoxListBox(GetHwnd());
        if(!hwndListBox) return False;

        //Step 2: smack a LS_OWNERDRAW style into the listbox
        WinSetWindowBits(hwndListBox, QWL_STYLE, LS_OWNERDRAW, LS_OWNERDRAW);

        //Step 3: subclass the editfield
        ef=new FODComboBox_ef(this);
        if(!ef->Create(this,CBID_EDIT))
                return False;

        //Step 4: ask the subclass what the height of the items should be
        int itemHeight=GetItemHeight();

        //Step 5: change font presparams for the combobox. This will cause the
        //        combobox to resize the entryfield, and the listbox to send 
        //        a wm_measureitem again

        //compute font
        HPS hps=WinGetScreenPS(HWND_DESKTOP);
        HDC hdc=GpiQueryDevice(hps);
        LONG vertical_res;
        DevQueryCaps(hdc,CAPS_VERTICAL_FONT_RES,1,&vertical_res);
        int fontsize = itemHeight*72/vertical_res;
        char font[20];
        itoa(fontsize,font,10);
        strcat(font,".Helv");
        //change presparams
        WinSetPresParam(GetHwnd(), PP_FONTNAMESIZE, strlen(font), (PVOID)font);

        return True;
}



FODSelectionComboBox::FODSelectionComboBox(FOwnerDrawOwner *powner)
  : FSelectionComboBox(),
    FOwnerDrawOwnee(this,powner),
    FODComboBox_forwarder(this)
{
        ef=0;
        hwndListBox=0;
}

FODSelectionComboBox::~FODSelectionComboBox() {
        delete ef;
}


Bool FODSelectionComboBox::Create(FWnd *pParent, int ID)
{
        if(!FSelectionComboBox::Create(pParent,ID))
                return False;   
        if(!MakeItOwnerDraw())
                return False;
        return True;
}

Bool FODSelectionComboBox::Create(FWnd *pParent, int ID,
                                  FRect *rect
                                 )
{
        if(!FSelectionComboBox::Create(pParent,ID,rect))
                return False;
        if(!MakeItOwnerDraw())
                return False;
        return True;
}


Bool FODSelectionComboBox::AddItem(long itemData) {
        char empty='\0';
        MRESULT mr=WinSendMsg(GetHwnd(), LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(&empty));
        SHORT i=SHORT1FROMMR(mr);
        if(i==LIT_MEMERROR ||
           i==LIT_ERROR)
                return False;
        WinSendMsg(hwndListBox, LM_SETITEMHANDLE, MPFROMSHORT(i), MPFROMLONG(itemData));
        return True;
}

Bool FODSelectionComboBox::InsertItem(int index, long itemData) {
        char empty='\0';
        MRESULT mr=WinSendMsg(GetHwnd(), LM_INSERTITEM, MPFROMSHORT(index), MPFROMP(&empty));
        SHORT i=SHORT1FROMMR(mr);
        if(i==LIT_MEMERROR ||
           i==LIT_ERROR)
                return False;
        WinSendMsg(hwndListBox, LM_SETITEMHANDLE, MPFROMSHORT(i), MPFROMLONG(itemData));
        return True;
}

Bool FODSelectionComboBox::GetItemData(int index, long *itemData) {
        MRESULT mr=WinSendMsg(hwndListBox, LM_QUERYITEMHANDLE, MPFROMSHORT(index), 0);
        *itemData = (long)mr;
        return True;
}


Bool FODSelectionComboBox::OwneeMeasure(FMeasureItemEvent &ev) {
        ev.SetItemHeight(GetItemHeight());
        return True;
}

