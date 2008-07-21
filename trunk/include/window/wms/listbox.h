#ifndef __LISTBOX_H_INCLUDED
#define __LISTBOX_H_INCLUDED

#ifndef __KEYBOARD_H_INCLUDED
#  include <window\wms\keyboard.h>
#endif
#ifndef __MOUSE_H_INCLUDED
#  include <window\wms\mouse.h>
#endif
#ifndef __COMHNDL_H_INCLUDED
#  include <window\wms\comhndl.h>
#endif

//styles
#define LS_MULTIPLESEL  0x0001
#define LS_HORZSCROLL   0x0008
#define LS_EXTENDEDSEL  0x0010
//notifications
#define LN_SELECT       1
#define LN_SETFOCUS     2
#define LN_KILLFOCUS    3
#define LN_ENTER        5
//search options
#define LSS_CASESENSITIVE       0x0001
#define LSS_PREFIX              0x0002
#define LSS_SUBSTRING           0x0004

class BIFCLASS FListBox_Handler : public FHandler {
public:
        FListBox_Handler(FWnd *pWnd)
          : FHandler(pWnd)
        { SetDispatch(dispatchFunc_t(FListBox_Handler::Dispatch)); }
        Bool Dispatch(FEvent&);
protected:
        virtual void    lm_deleteall() =0;
        virtual Bool    lm_deleteitem(int pos) =0;
        virtual int     lm_insertitem(const char *s, int pos) =0;
        virtual int     lm_queryitemcount() =0;
        virtual uint32  lm_queryitemdata(int pos) =0;
        virtual int     lm_queryitemtext(char *buf, int buflen, int pos) =0;
        virtual int     lm_queryitemtextlen(int pos) =0;
        virtual int     lm_queryselection(int start) =0;
        virtual int     lm_querytopindex() =0;
        virtual int     lm_searchstring(const char *s, int start, uint16 options) =0;
        virtual Bool    lm_selectitem(int pos, Bool s) =0;
        virtual Bool    lm_setitemdata(uint32 data, int pos) =0;
        virtual Bool    lm_setitemtext(const char *s, int pos) =0;
        virtual Bool    lm_settopindex(int pos) =0;

        virtual void    wm_paint() =0;
};

class BIFCLASS FListBox : public FControl,
                          protected FListBox_Handler,
                          public FKeyboardHandler,
                          public FFocusHandler,
                          public FTranslatedMouseHandler
{
public:
        enum {
                lbs_nointegralheight,
                lbs_horizontalscroll
        };
protected:
        FListBox();
        ~FListBox();

        Bool Create(FWnd *pParent, int resID)
          { return FControl::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, uint32 flStyle);

        //these structures are meant to be accessed by the subclasses
        struct ListBoxItem {
                char  *itemData1;       //text
                uint32 itemData2;       //app. data
                uint32 itemState;       //OIS_xxx
        };
        ListBoxItem *item;
        int items;
        int allocatedItems;
        int topItem;
        int focusedItem;
        int insertItem(const ListBoxItem &lbi, int pos);
        Bool removeItem(int pos);
        Bool focusItem(int pos);
        Bool invalidateItem(int pos);
        void fixFocus();
        void notifyOwner(uint16 code);
        int mapPoint2Item(const FPoint& p);
public:

        int GetCount();
        void Clear();

        int GetTopIndex();
        Bool SetTopIndex(int index);

        void DisableRedraw();
        void EnableRedraw();

        Bool Paint() { return False; }
protected:
        unsigned QueryDlgCode() { return DLGC_LISTBOX; }
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);

protected:
        void    lm_deleteall();
        Bool    lm_deleteitem(int pos);
        int     lm_insertitem(const char *s, int pos);
        int     lm_queryitemcount();
        uint32  lm_queryitemdata(int pos);
        int     lm_queryitemtext(char *buf, int buflen, int pos);
        int     lm_queryitemtextlen(int pos);
        int     lm_queryselection(int start);
        int     lm_querytopindex();
        int     lm_searchstring(const char *s, int start, uint16 options);
        Bool    lm_selectitem(int pos, Bool s);
        Bool    lm_setitemdata(uint32 data, int pos);
        Bool    lm_setitemtext(const char *s, int pos);
        Bool    lm_settopindex(int pos);

        Bool LostFocus(FKillFocusEvent&);
        Bool GotFocus(FSetFocusEvent&);

        Bool KeyPressed(FKeyboardEvent&);

        Bool MouseSelectStart(FMouseEvent&) =0;
        Bool MouseMove(FMouseMoveEvent&) =0;
        Bool MouseSelectEnd(FMouseEvent&) =0;
        Bool MouseSingleSelect(FMouseEvent&) =0;
        Bool MouseOpen(FMouseEvent&);

};

class BIFCLASS FTextListBox : virtual public FListBox {
protected:
        Bool sortItems;
        FTextListBox() : FListBox() {}
public:
        Bool Create(FWnd *pParent, int resID, Bool sorted=False) {
                sortItems=sorted;
                return FListBox::Create(pParent,resID);
        }
        Bool Create(FWnd *pParent, int ID, FRect *rect, Bool sorted=False);

        Bool AddItem(const char *pszText);
        Bool InsertItem(int index, const char *pszText);
        Bool GetItemText(int index, char *buffer, int maxbuf);
        int GetItemTextLen(int index);
        Bool RemoveItem(int index);
        Bool ReplaceItem(int index, const char *pszText);

        int FindItem(const char *pszText, int startIndex=-1);
protected:
        int lm_insertitem(const char *s, int pos);
        void wm_paint();
        Bool CharInput(FKeyboardEvent&);
        Bool _ProcessResourceSetup(int props, char *name[], char *value[]);
};

class BIFCLASS FSingleSelectionListBox : virtual public FListBox {
protected:
        FSingleSelectionListBox() : FListBox() {}
public:

        int GetCurrentSelection();
        Bool SetCurrentSelection(int index);
protected:
        Bool lm_selectitem(int pos, Bool s);

        Bool KeyPressed(FKeyboardEvent&);
        Bool CharInput(FKeyboardEvent&);

        Bool MouseSelectStart(FMouseEvent&);
        Bool MouseMove(FMouseMoveEvent&);
        Bool MouseSelectEnd(FMouseEvent&);
        Bool MouseSingleSelect(FMouseEvent&);
};

class BIFCLASS FMultipleSelectionListBox : virtual public FListBox {
        void extendSelection(int prevfocus);
protected:
        FMultipleSelectionListBox() : FListBox() {}
public:

        enum lbs_multiselectstyle {
                lbs_me_selection,
                lbs_multipleselection,
                lbs_extendedselection
        };
        Bool SetSelState(int index, Bool selected);
        Bool SetSelStateAll(Bool selected);
        Bool SelectItem(int index)
          { return SetSelState(index,True); }
        Bool UnSelectItem(int index)
          { return SetSelState(index,False); }

        int GetSelCount();
        Bool GetSelItems(int index[], int maxitem);
protected:
        Bool KeyPressed(FKeyboardEvent&);
        Bool CharInput(FKeyboardEvent&);

        Bool isMultiple();
        Bool isExtended();

        Bool MouseSelectStart(FMouseEvent&);
        Bool MouseMove(FMouseMoveEvent&);
        Bool MouseSelectEnd(FMouseEvent&);
        Bool MouseSingleSelect(FMouseEvent&);
        Bool _ProcessResourceSetup(int props, char *name[], char *value[]);
};


//text listboxes
class BIFCLASS FSingleSelectionTextListBox : virtual public FListBox,
                                             public FTextListBox,
                                             public FSingleSelectionListBox
{
public:
        FSingleSelectionTextListBox();

        Bool Create(FWnd *pParent, int resID)
          { return FListBox::Create(pParent,resID); }

        Bool Create(FWnd *pParent, int ID, FRect *rect, Bool sorted=False);
protected:
        Bool CharInput(FKeyboardEvent&);
        //this function circumvents a bug in WC:
        int lm_insertitem(const char *s, int pos)
          { return FTextListBox::lm_insertitem(s,pos); }
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};

class BIFCLASS FMultipleSelectionTextListBox : virtual public FListBox,
                                               public FTextListBox,
                                               public FMultipleSelectionListBox
{
public:
        FMultipleSelectionTextListBox();

        Bool Create(FWnd *pParent, int resID, Bool sorted=False)
          { return FTextListBox::Create(pParent,resID,sorted); }
        Bool Create(FWnd *pParent, int ID, FRect *rect,
                    lbs_multiselectstyle msstyle=lbs_me_selection,
                    Bool sorted=False);
protected:
        Bool KeyPressed(FKeyboardEvent&);
        Bool CharInput(FKeyboardEvent&);
        //this function circumvents a bug in WC:
        int lm_insertitem(const char *s, int pos)
          { return FTextListBox::lm_insertitem(s,pos); }
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};



//ownerdraw listboxes
class BIFCLASS FODListBox : virtual public FListBox, 
                            public FOwnerDrawOwnee 
{
public:
        FODListBox(FOwnerDrawOwner *owner)
          : FListBox(), FOwnerDrawOwnee(this,owner)
          {}
        
        Bool Create(FWnd *pParent, int resID)
          { return FListBox::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect);

        Bool AddItem(long itemData);
        Bool InsertItem(int index, long itemData);
        Bool GetItemData(int index, long *itemData);
        Bool RemoveItem(int index);
        Bool ReplaceItem(int index, long itemData);

protected:
        virtual Bool OwneeMeasure(FMeasureItemEvent&);

        virtual int GetItemHeight() =0;
        virtual void wm_paint();
};


class BIFCLASS FSingleSelectionODListBox : virtual public FListBox,
                                           public FODListBox,
                                           public FSingleSelectionListBox
{
public:
        FSingleSelectionODListBox(FOwnerDrawOwner *owner);

        Bool Create(FWnd *pParent, int resID)
          { return FODListBox::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect);
protected:
        Bool ProcessResourceSetup(int props, char *name[], char *value[])
          { return FSingleSelectionListBox::ProcessResourceSetup(props,name,value); }
};

class BIFCLASS FMultipleSelectionODListBox : virtual public FListBox,
                                             public FODListBox,
                                             public FMultipleSelectionListBox
{
public:
        FMultipleSelectionODListBox(FOwnerDrawOwner *owner);

        Bool Create(FWnd *pParent, int resID)
          { return FODListBox::Create(pParent,resID); }

        Bool Create(FWnd *pParent, int ID, FRect *rect,
                    lbs_multiselectstyle msstyle=lbs_me_selection);
protected:
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};


#endif
