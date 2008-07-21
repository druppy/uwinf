#ifndef __COMBOBOX_H_INCLUDED
#define __COMBOBOX_H_INCLUDED

#ifndef __KEYBOARD_H_INCLUDED
#  include <window\wms\keyboard.h>
#endif
#ifndef __COMHNDL_H_INCLUDED
#  include <window\wms\comhndl.h>
#endif

//generic combox
class BIFCLASS FComboBoxHandler : public FHandler {
public:
        FComboBoxHandler(FWnd *pwnd)
          : FHandler(pwnd)
          { SetDispatch(dispatchFunc_t(FComboBoxHandler::Dispatch)); }
protected:
        Bool Dispatch(FEvent &ev);

        //combobox msgs
        virtual Bool cbm_islistshowing() =0;
        virtual Bool cbm_showlist(Bool show) =0;
        virtual Bool wm_adjustwindowpos(FEvent &ev) =0;
        virtual Bool wm_focuschanging(WmsHWND otherhwnd, Bool f, uint16 fl) =0;
        virtual Bool lm_(FEvent &ev) =0;
        virtual Bool wm_querytextlen(FEvent &ev) =0;
        virtual Bool wm_querytext(FEvent &ev) =0;
        virtual Bool wm_settext(FEvent &ev) =0;
        virtual Bool em_limittext(FEvent &ev) =0;
};

class DropButton;

#define CBID_LIST       0x029a
#define CBID_EDIT       0x029b
#define CBID_DROP       0x029c
#define CBS_SIMPLE              0x0001
#define CBS_DROPDOWN            0x0002
#define CBS_DROPDOWNLIST        0x0004

class BIFCLASS FComboBox : public FControl,
                           protected FComboBoxHandler,
                           public FKeyboardHandler,
                           public FFocusHandler,
                           public FOwnerNotifyHandler,
                           public FCreateDestroyHandler
{
protected:
        Bool selfResizing;
        DropButton *dropbutton;
        FWnd *listboxthing;
        FWnd *editthing;
        int droppedDownHeight;
public:
        int GetCount();
        void Clear();

        Bool RemoveItem(int index);
        Bool SetCurrentSelection(int index);
protected:
        FComboBox();
        ~FComboBox();

        Bool Create(FWnd *pParent, int resID)
          { return FControl::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID,
                    const char *pszControlText,
                    uint32 flStyle, FRect *rect
                   );
protected:
        unsigned QueryDlgCode() { return DLGC_LISTBOX; }
        Bool Paint();

        virtual Bool cbm_islistshowing();
        virtual Bool cbm_showlist(Bool show);
        virtual Bool wm_adjustwindowpos(FEvent &ev);
        virtual Bool wm_focuschanging(WmsHWND otherhwnd, Bool f, uint16 fl);

        Bool KeyPressed(FKeyboardEvent &ev);

        void firstItem();
        void prevPage();
        void prevItem();
        void nextItem();
        void nextPage();
        void lastItem();

        virtual int findCurrentItem() =0;
        virtual void selectItem(int idx) =0;

        Bool Notify(FOwnerNotifyEvent &ev);
};


//A little mixin-class so we dont have to repeat ourselves too much :-)
class BIFCLASS FComboTextItem_mixin
{
        FComboBox *combo;
protected:
        FComboTextItem_mixin(FComboBox *c)
          : combo(c)
          {}
public:
        Bool InsertItem(int index, const char *pszItem);
        Bool AddItem(const char *pszItem);
        Bool GetItemText(int index, char *buffer, int buflen);
        int GetItemTextLen(int index);
};


//a combobox where the user can enter his own text if he wants
class BIFCLASS FEditComboBox : public FComboBox,
                               public FComboTextItem_mixin
{
public:
        FEditComboBox()
          : FComboBox(),
            FComboTextItem_mixin(this)
          {}

        Bool SetText(const char *pszText);
        Bool GetText(char *buffer, int buflen);
        int GetTextLength();
        Bool LimitText(int limit);
protected:
        int findCurrentItem();
        void selectItem(int idx);

        virtual Bool lm_(FEvent &ev);
        virtual Bool wm_querytextlen(FEvent &ev);
        virtual Bool wm_querytext(FEvent &ev);
        virtual Bool wm_settext(FEvent &ev);
        virtual Bool em_limittext(FEvent &ev);
};


//a combobox where the listbox is always visible
class BIFCLASS FSimpleComboBox : public FEditComboBox 
{
public:
        Bool Create(FWnd *pParent, int resID);
        Bool Create(FWnd *pParent, int ID,
                    const char *initialtext,
                    FRect *rect
                   );
private:
        Bool CreateThings();
protected:
        virtual Bool cbm_islistshowing();
        virtual Bool cbm_showlist(Bool show);
        virtual Bool wm_adjustwindowpos(FEvent &ev);

        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};


//a combobox where the listbox only is visible when the user has requested it
class BIFCLASS FDropDownComboBox : public FEditComboBox 
{
public:
        FDropDownComboBox() : FEditComboBox() {}
        Bool Create(FWnd *pParent, int resID);
        Bool Create(FWnd *pParent, int ID,
                    const char *initialtext,
                    FRect *rect
                   );
private:
        Bool CreateThings();
protected:

        virtual Bool wm_adjustwindowpos(FEvent &ev);
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};


//a combobox that doesn't allow the user to enter his own item
class BIFCLASS FSelectionComboBox : public FComboBox 
{
public:
        FSelectionComboBox() : FComboBox() {}

        Bool Create(FWnd *pParent, int resID);
        Bool Create(FWnd *pParent, int ID,
                    uint32 flStyle, FRect *rect
                   );

        int GetCurrentSelection();
protected:
        virtual Bool wm_adjustwindowpos(FEvent &ev);
        virtual Bool lm_(FEvent &ev);
        virtual Bool wm_querytextlen(FEvent &ev);
        virtual Bool wm_querytext(FEvent &ev);
        virtual Bool wm_settext(FEvent &ev);
        virtual Bool em_limittext(FEvent &ev);

        int findCurrentItem();
        void selectItem(int idx);
};


class BIFCLASS FTextSelectionComboBox : public FSelectionComboBox,
                                        public FComboTextItem_mixin
{
public:
        FTextSelectionComboBox()
          : FSelectionComboBox(),
            FComboTextItem_mixin(this)
          {}
          
        Bool Create(FWnd *pParent, int resID);
        Bool Create(FWnd *pParent, int ID, FRect *rect);

protected:
        virtual Bool lm_(FEvent &ev);
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
        void selectItem(int idx);
private:
        Bool CreateThings();
};


class BIFCLASS FODSelectionComboBox : public FSelectionComboBox,
                                      public FOwnerDrawOwnee
{
        FOwnerDrawOwner *o;
public:
        FODSelectionComboBox(FOwnerDrawOwner *powner);
        ~FODSelectionComboBox();

        Bool Create(FWnd *pParent, int resID)
          { return FSelectionComboBox::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect);

        Bool AddItem(long itemData);
        Bool InsertItem(int index, long itemData);
        Bool GetItemData(int index, long *itemData);

protected:
        virtual Bool OwneeMeasure(FMeasureItemEvent&);
        virtual int GetItemHeight() =0;

        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
        void selectItem(int idx);
private:
        Bool CreateThings();
};

#endif
