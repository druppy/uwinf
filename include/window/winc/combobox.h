#ifndef __COMBOBOX_H_INCLUDED
#define __COMBOBOX_H_INCLUDED

//generic combox
class BIFCLASS FComboBox : public FControl 
{
public:
        int GetCount();
        void Clear();

        Bool RemoveItem(int index);
        Bool SetCurrentSelection(int index);
protected:
        Bool Create(FWnd *pParent, int ID);
        Bool Create(FWnd *pParent, int ID,
                    const char *pszControlText,
                    DWORD flStyle, FRect *rect
                   );
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


//a combobox where the user change enter his own text if he wants
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
};


//a combobox where the listbox is always visible
class BIFCLASS FSimpleComboBox : public FEditComboBox 
{
public:
        Bool Create(FWnd *pParent, int ID)
          { return FComboBox::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID,
                    const char *initialtext,
                    FRect *rect
                   );
};


//a combobox where the listbox only is visible when the user has requested it
class BIFCLASS FDropDownComboBox : public FEditComboBox 
{
public:
        Bool Create(FWnd *pParent, int ID)
          { return FComboBox::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID,
                    const char *initialtext,
                    FRect *rect
                   );
};


//a combobox that doesn't allow the user to enter his own item
class BIFCLASS FSelectionComboBox : public FComboBox 
{
public:
        Bool Create(FWnd *pParent, int ID)
          { return FComboBox::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID,
                    DWORD flStyle, FRect *rect
                   );

        int GetCurrentSelection();
};


class BIFCLASS FTextSelectionComboBox : public FSelectionComboBox,
                                        public FComboTextItem_mixin
{
public:
        FTextSelectionComboBox()
          : FSelectionComboBox(),
            FComboTextItem_mixin(this)
          {}
          
        Bool Create(FWnd *pParent, int ID)
          { return FSelectionComboBox::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect)
          { return FSelectionComboBox::Create(pParent,ID,0,rect); }

};


class BIFCLASS FODSelectionComboBox : public FSelectionComboBox,
                                      public FOwnerDrawOwnee
{
public:
        FODSelectionComboBox(FOwnerDrawOwner *powner);
        ~FODSelectionComboBox();

        Bool Create(FWnd *pParent, int ID)
          { return FSelectionComboBox::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID,
                    FRect *rect
                   );

        Bool AddItem(long itemData);
        Bool InsertItem(int index, long itemData);
        Bool GetItemData(int index, long *itemData);

protected:
        virtual Bool OwneeMeasure(FMeasureItemEvent&);

        virtual int GetItemHeight() =0;
};

#endif
