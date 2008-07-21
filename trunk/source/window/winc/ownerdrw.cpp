/***
Filename: ownerdrw.cpp
Description:
  Implementation of ownerdrawing
Host:
  BC40, WC10
History:
  ISJ 94-11-?? Creation
  ISJ 95-03-03 Changed to avoid SC61 "internal compiler error"
**/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_OWNERDRAW
#include <bif.h>

//FMeasureItemEvent------------------------------------------------------------
Bool FMeasureItemEvent::IsFromControl() {
        return Bool(GetControlID()!=0);
}

Bool FMeasureItemEvent::IsFromMenu() {
        return Bool(GetControlID()==0);
}

int FMeasureItemEvent::GetControlID() {
        return GetWParam();
}


LPMEASUREITEMSTRUCT FMeasureItemEvent::GetMis() {
        return (LPMEASUREITEMSTRUCT)GetLParam();
}

int FMeasureItemEvent::GetControlType() {
        return GetMis()->CtlType;
}

int FMeasureItemEvent::GetItemID() {
        return GetMis()->itemID;
}

int FMeasureItemEvent::GetItemWidth() {
        return GetMis()->itemWidth;
}

void FMeasureItemEvent::SetItemWidth(int w) {
        GetMis()->itemWidth=w;
}

int FMeasureItemEvent::GetItemHeight() {
        return GetMis()->itemHeight;
}

void FMeasureItemEvent::SetItemHeight(int h) {
        GetMis()->itemHeight=h;
}

LPARAM FMeasureItemEvent::GetItemData() {
        return GetMis()->itemData;
}


//FDrawItemEvent---------------------------------------------------------------
Bool FDrawItemEvent::IsFromControl() {
        return Bool(GetControlID()!=0);
}

Bool FDrawItemEvent::IsFromMenu() {
        return Bool(GetControlID()==0);
}
int FDrawItemEvent::GetControlID() {
        return GetWParam();
}

const LPDRAWITEMSTRUCT FDrawItemEvent::GetDis() {
        return (const LPDRAWITEMSTRUCT)GetLParam();
}

int FDrawItemEvent::GetControlType() {
        return GetDis()->CtlType;
}

int FDrawItemEvent::GetItemID() {
        return GetDis()->itemID;
}

int FDrawItemEvent::GetAction() {
        return GetDis()->itemAction;
}

int FDrawItemEvent::GetItemState() {
        return GetDis()->itemState;
}

HWND FDrawItemEvent::GetControlHwnd() {
        if(IsFromControl())
                return GetDis()->hwndItem;
        else
                return 0;
}

HMENU FDrawItemEvent::GetHmenu() {
        if(IsFromMenu())
                return (HMENU)(GetDis()->hwndItem);
        else
                return 0;
}

HDC FDrawItemEvent::GetHdc() {
        return GetDis()->hDC;
}

FRect FDrawItemEvent::GetItemRect() {
        return GetDis()->rcItem;
}

LPARAM FDrawItemEvent::GetItemData() {
        return GetDis()->itemData;
}



//FOwnerDrawOwner -------------------------------------------------------------
//The ownerdrawowner keeps a list of ownerdraw-controls (ownerdraw-ownees), so
//it kan forward events to them. This way the controls can control themselves

void FOwnerDrawOwner::AddOwnee(FOwnerDrawOwnee *ownee) {
        ownee->next=firstOwnee;
        firstOwnee=ownee;
}

FOwnerDrawOwnee *FOwnerDrawOwner::findOwnee(int ID) {
        FOwnerDrawOwnee *p=firstOwnee;
        while(p) {
                if(p->GetOwneeID()==ID)
                        return p;
                p=p->next;
        }
        return 0;
}

FOwnerDrawOwner::FOwnerDrawOwner(FWnd *pwnd)
  : FHandler(pwnd)
{
        firstOwnee=0;
        SetDispatch(dispatchFunc_t(&FOwnerDrawOwner::Dispatch));
}

Bool FOwnerDrawOwner::Dispatch(FEvent& ev) {
        if(ev.GetID()==WM_DRAWITEM)
                return OwnerDraw((FDrawItemEvent&)ev);
        else if(ev.GetID()==WM_MEASUREITEM)
                return OwnerMeasure((FMeasureItemEvent&)ev);
        else
                return False;
}

Bool FOwnerDrawOwner::OwnerDraw(FDrawItemEvent& ev) {
        if(ev.IsFromControl()) {
                FOwnerDrawOwnee *ownee=findOwnee(ev.GetControlID());
                if(ownee)
                        return ownee->OwneeDraw(ev);
        }
        return False;
}

Bool FOwnerDrawOwner::OwnerMeasure(FMeasureItemEvent& ev) {
        if(ev.IsFromControl()) {
                FOwnerDrawOwnee *ownee=findOwnee(ev.GetControlID());
                if(ownee)
                        return ownee->OwneeMeasure(ev);
        }
        return False;
}



//FOwnerDrawOwnee -------------------------------------------------------------
int FOwnerDrawOwnee::GetOwneeID() {
        if(GetWnd()->IsValid()) {
                HWND hwnd=GetWnd()->GetHwnd();
                return GetDlgCtrlID(hwnd);
        } else {
                //not Create()'ed yet
                return ID;
        }                            
}

FOwnerDrawOwnee::FOwnerDrawOwnee(FWnd *pwnd, FOwnerDrawOwner *owner)
  : FHandler(pwnd)
{
        next=0;
        owner->AddOwnee(this);
}

FOwnerDrawOwnee::~FOwnerDrawOwnee() {
        //nothing. It is assumed that the ownerdrawowner is going to be
        //destroyed shortly after
}


