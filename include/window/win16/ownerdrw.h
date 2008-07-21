#ifndef __OWNERDRW_H_INCLUDED
#define __OWNERDRW_H_INCLUDED

class BIFCLASS FMeasureItemEvent : public FEvent {
public:
        Bool IsFromControl();
        Bool IsFromMenu();
        int GetControlID();

        //windows only:
        LPMEASUREITEMSTRUCT GetMis();
        int GetControlType();
        int GetItemID();
        int GetItemWidth();
        void SetItemWidth(int w);
        int GetItemHeight();
        void SetItemHeight(int h);
        LPARAM GetItemData();
};

class BIFCLASS FDrawItemEvent : public FEvent {
public:
        Bool IsFromControl();
        Bool IsFromMenu();
        int GetControlID();

        //Windows only:
        const LPDRAWITEMSTRUCT GetDis();

        int GetControlType();

	int GetItemID();

        int GetAction();
        Bool DrawEntire() { return Bool(GetAction()&ODA_DRAWENTIRE); }
        Bool DrawFocus() { return Bool(GetAction()&ODA_FOCUS); }
        Bool DrawSelect() { return Bool(GetAction()&ODA_SELECT); }

        int GetItemState();
        Bool IsItemChecked() { return Bool(GetItemState()&ODS_CHECKED); }
        Bool IsItemDisabled() { return Bool(GetItemState()&ODS_DISABLED); }
        Bool IsItemFocused() { return Bool(GetItemState()&ODS_FOCUS); }
        Bool IsItemGrayed() { return Bool(GetItemState()&ODS_GRAYED); }
        Bool IsItemSelected() { return Bool(GetItemState()&ODS_SELECTED); }

        HWND GetControlHwnd();
        HMENU GetHmenu();

        HDC GetHdc();
        FRect GetItemRect();

        LPARAM GetItemData();
};

class BIFCLASS FOwnerDrawOwnee;

class BIFCLASS FOwnerDrawOwner : public FHandler {
        friend class FOwnerDrawOwnee;
        void AddOwnee(FOwnerDrawOwnee *ownee);
        FOwnerDrawOwnee *firstOwnee;
        FOwnerDrawOwnee *findOwnee(int ID);
public:
        FOwnerDrawOwner(FWnd *pwnd);
protected:
        Bool Dispatch(FEvent&);

        virtual Bool OwnerDraw(FDrawItemEvent&);
        virtual Bool OwnerMeasure(FMeasureItemEvent&);
};

class BIFCLASS FOwnerDrawOwnee : public FHandler {
        friend class FOwnerDrawOwner;
        int GetOwneeID();
        FOwnerDrawOwnee *next;
        int ID;
public:
        FOwnerDrawOwnee(FWnd *pwnd, FOwnerDrawOwner *owner);
        ~FOwnerDrawOwnee();
protected:
        void SetID(int id) { ID=id; }
        virtual Bool OwneeDraw(FDrawItemEvent&) =0;
        virtual Bool OwneeMeasure(FMeasureItemEvent&) =0;
};

#endif
