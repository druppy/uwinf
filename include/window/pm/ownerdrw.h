#ifndef __OWNERDRW_H_INCLUDED
#define __OWNERDRW_H_INCLUDED

class BIFCLASS FMeasureItemEvent : public FEvent {
public:
        Bool IsFromControl();
        Bool IsFromMenu();
        int GetControlID() { return SHORT1FROMMP(GetMP1()); }

        //os/2 only:
        int GetItemID() const { return SHORT1FROMMP(GetMP2()); }
        int GetItemWidth() const { return SHORT2FROMMR(GetResult()); }
        void SetItemWidth(int w) { SetResult(MRFROM2SHORT(GetItemHeight(),w)); }
        int GetItemHeight() const { return SHORT1FROMMR(GetResult()); }
        void SetItemHeight(int h) { SetResult(MRFROM2SHORT(h,GetItemWidth())); }
};

class BIFCLASS FDrawItemEvent : public FEvent {
public:
        Bool IsFromControl();
        Bool IsFromMenu();
        int GetControlID() { return SHORT1FROMMP(GetMP1()); }

        //os/2 only:
        ULONG GetControlSpec() { return LONGFROMMP(GetMP2()); }
        POWNERITEM GetOwnerItem() { return (POWNERITEM)PVOIDFROMMP(GetMP2()); }
};

class FOwnerDrawOwnee;
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
