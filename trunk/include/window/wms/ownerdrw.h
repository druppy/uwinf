#ifndef __OWNERDRW_H_INCLUDED
#define __OWNERDRW_H_INCLUDED

#define OIS_DISABLED    0x0001
#define OIS_SELECTED    0x0002
#define OIS_FOCUSED     0x0004

struct OWNERITEM {
        uint16  controlID;      //id of control window
        WmsHWND controlHwnd;    //hwnd of control window
        uint32  itemID;         //id/index of item
        FRect   itemRect;       //rectangle of item
        WmsHDC  hdc;            //HDC to draw in
        uint32  itemState;      //state of item (combination of OIS_xxx)
        uint32  itemData1;      //item data 1 (usually text)
        uint32  itemData2;      //item data 2 (application data)
};

class BIFCLASS FMeasureItemEvent : public FEvent {
public:
        Bool IsFromControl();
        Bool IsFromMenu();
        int GetControlID() { return UINT161FROMMP(GetMP1()); }

        //WMS only:
        int GetItemWidth() const { return UINT162FROMMR(GetResult()); }
        void SetItemWidth(int w) { SetResult(MRFROM2UINT16(GetItemHeight(),w)); }
        int GetItemHeight() const { return UINT161FROMMR(GetResult()); }
        void SetItemHeight(int h) { SetResult(MRFROM2UINT16(h,GetItemWidth())); }
};

class BIFCLASS FDrawItemEvent : public FEvent {
public:
        Bool IsFromControl();
        Bool IsFromMenu();
        int GetControlID() { return UINT161FROMMP(GetMP1()); }

        //WMS only:
        OWNERITEM *GetOwnerItem() { return (OWNERITEM*)PFROMMP(GetMP2()); }
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
