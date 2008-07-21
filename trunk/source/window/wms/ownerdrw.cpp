/***
Filename: ownerdrw.cpp
Description:
  Implementation of ownerdrawing
Host:
  Watcom 10.0a-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-08-27 Copied from PM sources to WMS
**/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_OWNERDRAW
#define BIFINCL_FRAME
#define WMSINCL_MSGS
#include <bif.h>

//Note: IsFromXxxx does not works for popup-menus, or controls having an Id of FID_MENU

//FMeasureItemEvent------------------------------------------------------------
Bool FMeasureItemEvent::IsFromControl() {
        return Bool(GetControlID()!=FID_MENU);
}

Bool FMeasureItemEvent::IsFromMenu() {
        return Bool(GetControlID()==FID_MENU);
}


//FDrawItemEvent---------------------------------------------------------------
Bool FDrawItemEvent::IsFromControl() {
        return Bool(GetControlID()!=FID_MENU);
}

Bool FDrawItemEvent::IsFromMenu() {
        return Bool(GetControlID()==FID_MENU);
}




//FOwnerDrawOwner -------------------------------------------------------------
//The ownerdrawowner keeps a list of ownerdraw-controls (ownerdraw-ownees), so
//it can forward events to them. This way the controls can control themselves

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
        WmsHWND hwnd=GetWnd()->GetHwnd();
        if(hwnd) {
                return WmsQueryWindowID(hwnd);
        } else {
                //The control is sending some ownerdraw-related messages
                //during wm_create
                // - use the ID variable
                return ID;
        }
}

FOwnerDrawOwnee::FOwnerDrawOwnee(FWnd *pwnd, FOwnerDrawOwner *owner)
  : FHandler(pwnd)
{
        next=0;
        ID=-2;
        owner->AddOwnee(this);
}

FOwnerDrawOwnee::~FOwnerDrawOwnee() {
        //nothing. It is assumed that the ownerdrawowner is going to be
        //destroyed shortly after
}

