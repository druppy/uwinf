#ifndef __LISTBOX_H_INCLUDED
#define __LISTBOX_H_INCLUDED

class BIFCLASS FListBox : public FControl {
        enum {
                lbs_nointegralheight
        };
protected:
        Bool Create(FWnd *pParent, int ID) {
                return FControl::Create(pParent,ID);
        }
        Bool Create(FWnd *pParent, int ID, FRect *rect, ULONG flStyle);
public:

        int GetCount();
        void Clear();

        int GetTopIndex();
        Bool SetTopIndex(int index);

        void DisableRedraw();
        void EnableRedraw();
};

class BIFCLASS FTextListBox : virtual public FListBox {
protected:
        Bool sortItems;
public:
        Bool Create(FWnd *pParent, int ID, Bool sorted=False) {
                sortItems=sorted;
                return FListBox::Create(pParent,ID);
        }
        Bool Create(FWnd *pParent, int ID, FRect *rect, Bool sorted=False);

        Bool AddItem(const char *pszText);
        Bool InsertItem(int index, const char *pszText);
        Bool GetItemText(int index, char *buffer, int maxbuf);
        int GetItemTextLen(int index);
        Bool RemoveItem(int index);
        Bool ReplaceItem(int index, const char *pszText);

        int FindItem(const char *pszText, int startIndex=-1);
};

class BIFCLASS FSingleSelectionListBox : virtual public FListBox {
protected:
        FSingleSelectionListBox() : FListBox() {}
public:

        int GetCurrentSelection();
        Bool SetCurrentSelection(int index);
};

class BIFCLASS FMultipleSelectionListBox : virtual public FListBox {
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
};


//text listboxes
class BIFCLASS FSingleSelectionTextListBox : virtual public FListBox,
                                             public FTextListBox,
                                             public FSingleSelectionListBox
{
public:
        FSingleSelectionTextListBox();

        Bool Create(FWnd *pParent, int ID);

        Bool Create(FWnd *pParent, int ID, FRect *rect, Bool sorted=False);
};

class BIFCLASS FMultipleSelectionTextListBox : virtual public FListBox,
                                               public FTextListBox,
                                               public FMultipleSelectionListBox
{
public:
        FMultipleSelectionTextListBox();

        Bool Create(FWnd *pParent, int ID);

        Bool Create(FWnd *pParent, int ID, FRect *rect,
                    lbs_multiselectstyle msstyle=lbs_me_selection,
                    Bool sorted=False);
};


//ownerdraw listboxes
class BIFCLASS FODListBox : virtual public FListBox, 
                            public FOwnerDrawOwnee 
{
public:
        FODListBox(FOwnerDrawOwner *owner)
          : FListBox(), FOwnerDrawOwnee(this,owner)
          {}

        Bool Create(FWnd *pParent, int ID) {
                return FListBox::Create(pParent,ID);
        }
        Bool Create(FWnd *pParent, int ID, FRect *rect);

        Bool AddItem(long itemData);
        Bool InsertItem(int index, long itemData);
        Bool GetItemData(int index, long *itemData);
        Bool RemoveItem(int index);
        Bool ReplaceItem(int index, long itemData);

protected:
        virtual Bool OwneeMeasure(FMeasureItemEvent&);

        virtual int GetItemHeight() =0;
};


class BIFCLASS FSingleSelectionODListBox : virtual public FListBox,
                                           public FODListBox,
                                           public FSingleSelectionListBox
{
public:
        FSingleSelectionODListBox(FOwnerDrawOwner *owner);

        Bool Create(FWnd *pParent, int ID);

        Bool Create(FWnd *pParent, int ID, FRect *rect);
};

class BIFCLASS FMultipleSelectionODListBox : virtual public FListBox,
                                             public FODListBox,
                                             public FMultipleSelectionListBox
{
public:
        FMultipleSelectionODListBox(FOwnerDrawOwner *owner);

        Bool Create(FWnd *pParent, int ID);

        Bool Create(FWnd *pParent, int ID, FRect *rect,
                    lbs_multiselectstyle msstyle=lbs_me_selection);
};

#endif
