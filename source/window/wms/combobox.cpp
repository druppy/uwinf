/***
Filename: combobox.cpp
Description:
        Implementation of the combobox classes under WMS
History:
        ISJ 96-09-08 Created
***/
#define BIFINCL_WINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_COMBOBOX
#define BIFINCL_LISTBOX
#define BIFINCL_SLE
#define WMSINCL_MSGS
#define WMSINCL_CPI
#define WMSINCL_FOCUS
#include <bif.h>
#include "presparm.h"
#include <string.h>

Bool FComboBoxHandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case CBM_SHOWLIST:
                        ev.SetResult((WmsMRESULT)cbm_showlist((Bool)UINT161FROMMP(ev.mp1)));
                        return True;
                case CBM_ISLISTSHOWING:
                        ev.SetResult((WmsMRESULT)cbm_islistshowing());
                        return True;
                case WM_QUERYTEXT:
                        return wm_querytext(ev);
                case WM_QUERYTEXTLENGTH:
                        return wm_querytextlen(ev);
                case WM_SETTEXT:
                        return wm_settext(ev);
                case LM_DELETEALL:
                case LM_DELETEITEM:
                case LM_INSERTITEM:
                case LM_QUERYITEMCOUNT:
                case LM_QUERYITEMDATA:
                case LM_QUERYITEMTEXT:
                case LM_QUERYITEMTEXTLEN:
                case LM_QUERYSELECTION:
                case LM_QUERYTOPINDEX:
                case LM_SEARCHSTRING:
                case LM_SELECTITEM:
                case LM_SETITEMDATA:
                case LM_SETITEMTEXT:
                case LM_SETTOPINDEX:
                        return lm_(ev);
                case WM_FOCUSCHANGING:
                        return wm_focuschanging(HWNDFROMMP(ev.mp1), (Bool)UINT161FROMMP(ev.mp2), UINT162FROMMP(ev.mp2));
                default:
                        return False;
        }
}



class DropButton : public FControl,
                   public FMouseHandler
{
public:
        DropButton()
          : FControl(), FMouseHandler(this), rollup(False)
          {}
        Bool rollup;
        Bool Create(FWnd *pParent, int ID, FRect *r) {
                return FControl::Create(pParent,ID,"",0,r);
        }

        unsigned QueryDlgCode() { return DLGC_CHECKBOX; }
        Bool Paint();
        Bool ButtonDown(FButtonDownEvent&);
};

Bool DropButton::Paint() {
        WmsHDC hdc=WmsBeginPaint(GetHwnd());
        if(hdc) {
                color fg_clr,bg_clr;
                WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR,0,0,&fg_clr,1);
                WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR,PP_DIALOGBACKGROUNDCOLOR,0,&bg_clr,1);
                FRect r;
                WmsQueryWindowRect(GetHwnd(),&r);
                CpiEraseRect(hdc,r,fg_clr|bg_clr);

                char buttontext[128];
                if(rollup)
                        WmsQueryPresParam(GetHwnd(), PP_CBROLLUPCHARS,0,0, buttontext,128);
                else
                        WmsQueryPresParam(GetHwnd(), PP_CBDROPDOWNCHARS,0,0, buttontext,128);
                CpiSetBkMode(hdc, bm_transparent);
                CpiSetTextColor(hdc, fg_clr);
                CpiOuttext(hdc, 0,0, buttontext);

                WmsEndPaint(hdc);
                return True;
        } else
                return False;
}

Bool DropButton::ButtonDown(FButtonDownEvent&) {
        if(rollup)
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), CBM_SHOWLIST, MPFROMUINT16(False), 0);
        else
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), CBM_SHOWLIST, MPFROMUINT16(True), 0);
        return True; //handled
}

FComboBox::FComboBox()
  : FControl(),
    FComboBoxHandler(this),
    FFocusHandler(this),
    FKeyboardHandler(this),
    FOwnerNotifyHandler(this),
    FCreateDestroyHandler(this),
    listboxthing(0),
    editthing(0),
    dropbutton(0),
    selfResizing(False)
{
}

FComboBox::~FComboBox() {
        delete listboxthing;
        delete editthing;
        delete dropbutton;
}

int FComboBox::GetCount() {
        return (int)WmsSendMsg(GetHwnd(), LM_QUERYITEMCOUNT, (WmsMPARAM)0, (WmsMPARAM)0);
}

void FComboBox::Clear() {
        WmsSendMsg(GetHwnd(), LM_DELETEALL, (WmsMPARAM)0, (WmsMPARAM)0);
}


Bool FComboBox::RemoveItem(int index) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_DELETEITEM, MPFROMUINT16(index), (WmsMPARAM)0);
}

Bool FComboBox::SetCurrentSelection(int index) {
        return (Bool)WmsSendMsg(GetHwnd(), LM_SELECTITEM, MPFROMUINT16(index), MPFROMUINT16(True));
}


Bool FComboBox::Create(FWnd *pParent, int ID,
                       const char *pszControlText,
                       uint32 flStyle, FRect *rect
                      )
{
        return FControl::Create(pParent,ID,pszControlText,flStyle,rect);
}



Bool FComboBox::wm_focuschanging(WmsHWND otherhwnd, Bool f, uint16) {
        if(f) {
                //gaining
                WmsHWND hwndNewFocus=WmsQueryFocus(GetDesktop()->GetHwnd());
                if(hwndNewFocus==GetHwnd()) {
                        ForwardFocus(editthing);
                        return False;
                }
                if(GetHwnd()==otherhwnd ||
                   WmsIsChild(GetHwnd(), otherhwnd)) {
                        //internal focus chaning
                        //-ignore
                        return True;
                } else {
                        //forward focus to edit-thing
                        ForwardFocus(editthing);
                        return False;
                }
        } else {
                //losing
                if(GetHwnd()==otherhwnd ||
                   WmsIsChild(GetHwnd(), otherhwnd)) {
                        //internal focus changing
                        //-ignore
                        return True;
                } else {
                        //losing focus to window outside family - roll listbox up
                        WmsPostMsg(GetHwnd(), CBM_SHOWLIST, MPFROMUINT16(False), 0);
                        return False;
                }
        }
}


Bool FComboBox::Paint() {
        WmsHDC hdc = WmsBeginPaint(GetHwnd());
        if(hdc) {
                color fg_clr,bg_clr;
                WmsQueryPresParam(GetHwnd(), PP_DEFAULTFOREGROUNDCOLOR,0,0,&fg_clr,1);
                WmsQueryPresParam(GetHwnd(), PP_DEFAULTBACKGROUNDCOLOR,PP_DIALOGBACKGROUNDCOLOR,0,&bg_clr,1);
                FRect r;
                WmsQueryWindowRect(GetHwnd(),&r);
                CpiEraseRect(hdc,r,fg_clr|bg_clr);

                WmsEndPaint(hdc);
                return True;
        } else
                return False;
}


Bool FComboBox::KeyPressed(FKeyboardEvent &ev) {
        if(ev.IsAltDown() && ev.GetLogicalKey()==lk_down) {
                if(cbm_islistshowing())
                        cbm_showlist(False);
                else
                        cbm_showlist(True);
                return True;
        }
        if(ev.IsAltDown() || ev.IsCtrlDown() || ev.IsShiftDown())
                return False;
        switch(ev.GetLogicalKey()) {
                case lk_home:
                        firstItem();
                        break;
                case lk_pageup:
                        prevPage();
                        break;
                case lk_up:
                        prevItem();
                        break;
                case lk_down:
                        nextItem();
                        break;
                case lk_pagedown:
                        nextPage();
                        break;
                case lk_end:
                        lastItem();
                        break;
                default:
                        return False;
        }
        return True;
}

void FComboBox::firstItem() {
        selectItem(0);
}

void FComboBox::prevPage() {
        if(GetCount()==0) return;
        int idx=findCurrentItem();
        if(idx==-1)
                firstItem();
        else {
                if(!listboxthing) return;
                FRect r;
                WmsQueryWindowRect(listboxthing->GetHwnd(),&r);
                idx -= r.GetHeight();
                if(idx<0) idx=0;
                selectItem(idx);
        }
}
void FComboBox::prevItem() {
        int idx=findCurrentItem();
        if(idx==-1)
                firstItem();
        else if(idx>0)
                selectItem(idx-1);
}
void FComboBox::nextItem() {
        int idx=findCurrentItem();
        if(idx==-1)
                firstItem();
        else if(idx<GetCount()-1)
                selectItem(idx+1);
}

void FComboBox::nextPage() {
        if(GetCount()==0) return;
        int idx=findCurrentItem();
        if(idx==-1)
                lastItem();
        else {
                if(!listboxthing) return;
                FRect r;
                WmsQueryWindowRect(listboxthing->GetHwnd(),&r);
                idx += r.GetHeight();
                if(idx>=GetCount()) idx=GetCount()-1;
                selectItem(idx);
        }
}
void FComboBox::lastItem() {
        if(GetCount()>0)
                selectItem(GetCount()-1);
}

Bool FComboBox::Notify(FOwnerNotifyEvent &ev) {
        if(ev.GetItem()==CBID_LIST &&
           ev.GetNotifyCode()==LN_SELECT &&
           listboxthing) {
                int idx=(int)WmsSendMsg(listboxthing->GetHwnd(), LM_QUERYSELECTION, MPFROMUINT16(-1), (WmsMPARAM)0);
                selectItem(idx);
        }
        return False; //pass-up
}


Bool FComboBox::cbm_islistshowing() {
        if(WmsQueryWindowStyle(listboxthing->GetHwnd())&WS_VISIBLE)
                return True;
        else
                return False;
}

Bool FComboBox::cbm_showlist(Bool show) {
        if(!cbm_islistshowing()) {
                //list is hidden
                if(!show)
                        return True; //already hidden
                //show list
                dropbutton->rollup=True;
                WmsInvalidateRect(dropbutton->GetHwnd(),0,False);

                WmsSetWindowPos(listboxthing->GetHwnd(), HWND_TOP, 0,0,0,0, SWP_ZORDER|SWP_SHOW);

                WmsHWND hwndFocus = WmsQueryFocus(GetDesktop()->GetHwnd());
                if(hwndFocus==GetHwnd() || WmsIsChild(GetHwnd(), hwndFocus))
                        WmsSetFocus(GetDesktop()->GetHwnd(),listboxthing->GetHwnd());
                //restore saved height
                FRect r;
                WmsQueryWindowRect(GetHwnd(),&r);
                selfResizing=True;
                WmsSetWindowPos(GetHwnd(),0, 0,0, r.GetWidth(),droppedDownHeight, SWP_SIZE);
                selfResizing=False;

                return True;
        } else {
                //list is visible
                if(show)
                        return True; //already visible
                //hide list
                dropbutton->rollup=False;
                WmsInvalidateRect(dropbutton->GetHwnd(),0,False);
                if(WmsQueryFocus(GetDesktop()->GetHwnd())==listboxthing->GetHwnd())
                        WmsSetFocus(GetDesktop()->GetHwnd(),editthing->GetHwnd());
                WmsSetWindowPos(listboxthing->GetHwnd(),0, 0,0, 0,0, SWP_HIDE);
                //reduce height to 1
                FRect r;
                WmsQueryWindowRect(GetHwnd(),&r);
                selfResizing=True;
                WmsSetWindowPos(GetHwnd(),0, 0,0, r.GetWidth(),1, SWP_SIZE);
                selfResizing=False;

                return True;
        }
}

Bool FComboBox::wm_adjustwindowpos(FEvent &ev) {
        uint16 flags = UINT161FROMMP(ev.mp2);
        if(flags&SWP_SIZE && listboxthing && listboxthing->IsValid() && !selfResizing) {
                //adjust window height to 1
                WmsSWP *pswp = (WmsSWP*)PFROMMP(ev.mp1);
                droppedDownHeight = (int)pswp->cy;
                pswp->cy=1;
        }
        return False;
}

//FComboTextItem_mixin---------------------------------------------------------
Bool FComboTextItem_mixin::InsertItem(int index, const char *pszItem) {
        return (Bool)WmsSendMsg(combo->GetHwnd(), LM_INSERTITEM, MPFROMUINT16(index), MPFROMP(pszItem));
}

Bool FComboTextItem_mixin::AddItem(const char *pszItem) {
        return (Bool)WmsSendMsg(combo->GetHwnd(), LM_INSERTITEM, MPFROMUINT16(-1), MPFROMP(pszItem));
}

Bool FComboTextItem_mixin::GetItemText(int index, char *buffer, int buflen) {
        return (Bool)WmsSendMsg(combo->GetHwnd(), LM_QUERYITEMTEXT, MPFROM2UINT16(index,buflen), MPFROMP(buffer));
}

int FComboTextItem_mixin::GetItemTextLen(int index) {
        return (int)WmsSendMsg(combo->GetHwnd(), LM_QUERYITEMTEXTLEN, MPFROMUINT16(index), (WmsMPARAM)0);
}




//FEditComboBox----------------------------------------------------------------

static Bool forwardEvent(FEvent &ev, FWnd *pwnd) {
        if(!pwnd) return False;
        WmsMRESULT mr=WmsSendMsg(pwnd->GetHwnd(), ev.GetID(), ev.mp1, ev.mp2);
        ev.SetResult(mr);
        return True;
}

Bool FEditComboBox::SetText(const char *pszText) {
        return (Bool)WmsSendMsg(GetHwnd(), WM_SETTEXT, MPFROMP(pszText), 0);
}

Bool FEditComboBox::GetText(char *buffer, int buflen) {
        return (Bool)WmsSendMsg(GetHwnd(), WM_QUERYTEXT, MPFROMP(buffer), MPFROMUINT16(buflen));
}

int FEditComboBox::GetTextLength() {
        return (Bool)WmsSendMsg(GetHwnd(), WM_QUERYTEXTLENGTH, 0, 0);
}

Bool FEditComboBox::LimitText(int limit) {
        return (Bool)WmsSendMsg(GetHwnd(), EM_SETTEXTLIMIT, MPFROMUINT16(limit), 0);
}

int FEditComboBox::findCurrentItem() {
        char buf[256];
        GetText(buf,256);
        WmsMRESULT mr=WmsSendMsg(GetHwnd(), LM_SEARCHSTRING, MPFROM2UINT16(-1,LSS_PREFIX|LSS_CASESENSITIVE), MPFROMP(buf));
        int idx=(sint16)UINT161FROMMR(mr);
        if(idx<0) return 1;
        else return idx;
}

void FEditComboBox::selectItem(int idx) {
        char buf[256];
        if(GetItemText(idx,buf,256)) {
                SetText(buf);
                WmsSendMsg(listboxthing->GetHwnd(), LM_SELECTITEM, MPFROMUINT16(idx), MPFROMUINT16(True));
        }
}


Bool FEditComboBox::lm_(FEvent &ev) {
        return forwardEvent(ev,listboxthing);
}

Bool FEditComboBox::wm_querytextlen(FEvent &ev) {
        return forwardEvent(ev,editthing);
}

Bool FEditComboBox::wm_querytext(FEvent &ev) {
        return forwardEvent(ev,editthing);
}

Bool FEditComboBox::wm_settext(FEvent &ev) {
        return forwardEvent(ev,editthing);
}

Bool FEditComboBox::em_limittext(FEvent &ev) {
        return forwardEvent(ev,editthing);
}


//FSimpleComboBox--------------------------------------------------------------
Bool FSimpleComboBox::Create(FWnd *pParent, int resID) {
        if(!FComboBox::Create(pParent,resID))
                return False;
        return True;
}

Bool FSimpleComboBox::Create(FWnd *pParent, int ID, const char *initialtext, FRect *rect)
{
        if(!FComboBox::Create(pParent,ID,"",CBS_SIMPLE,rect))
                return False;
        if(!CreateThings()) {
                Destroy();
                return False;
        }
        SetText(initialtext);
        return True;
}

Bool FSimpleComboBox::CreateThings() {
        //create sle and listbox
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        //remember full height and reduce height to 1 row
        droppedDownHeight = (int)r.GetHeight();
        WmsSetWindowPos(GetHwnd(), 0, 0,0, r.GetWidth(),1, SWP_SIZE);

        FSLE *sle = new FSLE;
        if(!sle) return False;
        FRect slerect=r;
        slerect.SetBottom(slerect.GetTop()+1);
        if(!sle->Create(this, CBID_EDIT, &slerect)) {
                delete sle; sle=0;
                return False;
        }
        FComboBox::editthing = sle;

        FSingleSelectionTextListBox *listbox = new FSingleSelectionTextListBox;
        if(!listbox) {
                delete sle; sle=0;
                return False;
        }
        FRect lbrect=r;
        lbrect.SetTop(lbrect.GetTop()+1);
        if(lbrect.GetHeight()<0) lbrect.SetBottom(lbrect.GetTop());
        if(!listbox->Create(this, CBID_LIST, &lbrect)) {
                delete listbox; listbox=0;
                delete sle; sle=0;
                return False;
        }

        FComboBox::listboxthing = listbox; //superclass needs access to fsinglesellistbox
        return True;
}


Bool FSimpleComboBox::cbm_islistshowing() {
        return True;
}
Bool FSimpleComboBox::cbm_showlist(Bool show) {
        return show;
}
Bool FSimpleComboBox::wm_adjustwindowpos(FEvent &ev) {
        uint16 flags = UINT161FROMMP(ev.mp2);
        if(flags&SWP_SIZE && listboxthing && listboxthing->IsValid() && !selfResizing) {
                WmsSWP *pswp = (WmsSWP*)PFROMMP(ev.mp1);
                WmsSetWindowPos(editthing->GetHwnd(),0, 0,0, pswp->cx,1, SWP_MOVE|SWP_SIZE);
                WmsSetWindowPos(listboxthing->GetHwnd(),0, 0,1, pswp->cx,pswp->cy-1, SWP_MOVE|SWP_SIZE);
                return False;
        }
        return FComboBox::wm_adjustwindowpos(ev);
}

Bool FSimpleComboBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        SetStyle(0xFFFF0000L, CBS_SIMPLE);
        SetStyle(~WS_CLIPCHILDREN,WS_CLIPCHILDREN);
        if(!CreateThings())
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"text")==0 && value[p]) {
                        SetText(value[p]);
                }
        }
        return True;
}



//-----------------------------------------------------------------------------
Bool FDropDownComboBox::Create(FWnd *pParent, int resID) {
        if(!FComboBox::Create(pParent,resID))
                return False;
        return True;
}

Bool FDropDownComboBox::Create(FWnd *pParent, int ID,
                               const char *initialtext,
                               FRect *rect
                               )
{
        if(!FComboBox::Create(pParent,ID,"",CBS_DROPDOWN,rect))
                return False;
        if(!CreateThings()) {
                Destroy();
                return False;
        }
        SetText(initialtext);
        return True;
}

Bool FDropDownComboBox::CreateThings() {
        //create sle and listbox
        FRect r;
        WmsQueryWindowRect(GetHwnd(),&r);
        //remember full height and reduce height to 1 row
        droppedDownHeight = (int)r.GetHeight();
        WmsSetWindowPos(GetHwnd(), 0, 0,0, r.GetWidth(),1, SWP_SIZE);

        FSLE *sle = new FSLE;
        if(!sle) return False;
        FRect slerect=r;
        slerect.SetBottom(slerect.GetTop()+1);
        slerect.SetRight(slerect.GetRight()-2);
        if(!sle->Create(this, CBID_EDIT, &slerect)) {
                delete sle; sle=0;
                return False;
        }

        FSingleSelectionTextListBox *listbox = new FSingleSelectionTextListBox;
        if(!listbox) {
                delete sle; sle=0;
                return False;
        }
        FRect lbrect=r;
        lbrect.SetTop(lbrect.GetTop()+1);
        if(lbrect.GetHeight()<0) lbrect.SetBottom(lbrect.GetTop());
        if(!listbox->Create(this, CBID_LIST, &lbrect)) {
                delete listbox; listbox=0;
                delete sle; sle=0;
                return False;
        }
        listbox->Hide();

        dropbutton=new DropButton;
        if(!dropbutton) {
                delete listbox;
                delete sle; sle=0;
                return False;
        }
        FRect dbrect=r;
        dbrect.SetLeft(r.GetRight()-1);
        dbrect.SetBottom(dbrect.GetTop()+1);
        if(!dropbutton->Create(this,CBID_DROP,&dbrect)) {
                delete dropbutton; dropbutton=0;
                delete listbox;
                delete sle; sle=0;
                return False;
        }

        FComboBox::editthing = sle;
        FComboBox::listboxthing = listbox;

        return True;
}


Bool FDropDownComboBox::wm_adjustwindowpos(FEvent &ev) {
        uint16 flags = UINT161FROMMP(ev.mp2);
        if(flags&SWP_SIZE && listboxthing && listboxthing->IsValid() && !selfResizing) {
                WmsSWP *pswp = (WmsSWP*)PFROMMP(ev.mp1);
                WmsSetWindowPos(editthing->GetHwnd(),0, 0,0, pswp->cx-2,1, SWP_MOVE|SWP_SIZE);
                WmsSetWindowPos(dropbutton->GetHwnd(),0, pswp->cx-1,0, 1,1, SWP_MOVE|SWP_SIZE);
                WmsSetWindowPos(listboxthing->GetHwnd(),0, 0,0, pswp->cx,pswp->cy-1, SWP_MOVE|SWP_SIZE);
        }
        return FComboBox::wm_adjustwindowpos(ev);
}


Bool FDropDownComboBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        SetStyle(0xFFFF0000L, CBS_DROPDOWN);
        SetStyle(~WS_CLIPCHILDREN,WS_CLIPCHILDREN);
        SetStyle(~WS_CLIPSIBLINGS,WS_CLIPSIBLINGS);
        if(!CreateThings())
                return False;
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"text")==0 && value[p]) {
                        SetText(value[p]);
                }
        }
        return True;
}


//FSelectionComboBox----------------------------------------------------------
Bool FSelectionComboBox::Create(FWnd *pParent, int resID) {
        return FComboBox::Create(pParent,resID);
}

Bool FSelectionComboBox::Create(FWnd *pParent, int ID, uint32 flStyle, FRect *rect) {
        return FComboBox::Create(pParent,ID,"",flStyle,rect);
}

int FSelectionComboBox::GetCurrentSelection() {
        return (int)WmsSendMsg(GetHwnd(), LM_QUERYSELECTION, MPFROMUINT16(-1), (WmsMPARAM)0);
}

Bool FSelectionComboBox::wm_adjustwindowpos(FEvent &ev) {
        uint16 flags = UINT161FROMMP(ev.mp2);
        if(flags&SWP_SIZE && listboxthing && listboxthing->IsValid() && !selfResizing) {
                WmsSWP *pswp = (WmsSWP*)PFROMMP(ev.mp1);
                WmsSetWindowPos(editthing->GetHwnd(),0, 0,0, pswp->cx-1,1, SWP_MOVE|SWP_SIZE);
                WmsSetWindowPos(dropbutton->GetHwnd(),0, pswp->cx-1,0, 1,1, SWP_MOVE|SWP_SIZE);
                WmsSetWindowPos(listboxthing->GetHwnd(),0, 0,0, pswp->cx,pswp->cy-1, SWP_MOVE|SWP_SIZE);
        }
        return FComboBox::wm_adjustwindowpos(ev);
}

Bool FSelectionComboBox::lm_(FEvent &ev) {
        return forwardEvent(ev,listboxthing);
}
Bool FSelectionComboBox::wm_querytextlen(FEvent &ev) {
        return forwardEvent(ev,editthing);
}
Bool FSelectionComboBox::wm_querytext(FEvent &ev) {
        return forwardEvent(ev,editthing);
}
Bool FSelectionComboBox::wm_settext(FEvent &ev) {
        ev.SetResult((WmsMRESULT)(False));
        return True;
}
Bool FSelectionComboBox::em_limittext(FEvent &ev) {
        ev.SetResult((WmsMRESULT)(False));
        return True;
}
int FSelectionComboBox::findCurrentItem() {
        int idx=(int)WmsSendMsg(listboxthing->GetHwnd(), LM_QUERYSELECTION, MPFROMUINT16(-1), (WmsMPARAM)0);
        if(idx<0) idx=-1;
        return idx;
}
void FSelectionComboBox::selectItem(int idx) {
        if(idx>=0) {
                WmsSendMsg(listboxthing->GetHwnd(), LM_SELECTITEM, MPFROMUINT16(idx), MPFROMUINT16(True));
        } else {         
                WmsSendMsg(listboxthing->GetHwnd(), LM_SELECTITEM, MPFROMUINT16(-1), MPFROMUINT16(False));
        }
        WmsInvalidateRect(dropbutton->GetHwnd(),0,False);
}


//FTextSelectionComboBox-----------------------------------------------------
Bool FTextSelectionComboBox::Create(FWnd *pParent, int resID) {
        return FSelectionComboBox::Create(pParent,resID);
}

Bool FTextSelectionComboBox::Create(FWnd *pParent, int ID, FRect *rect) {
        if(!FSelectionComboBox::Create(pParent,ID,CBS_DROPDOWNLIST,rect))
                return False;
        if(!CreateThings()) {
                Destroy();
                return False;
        } else
                return True;
}

Bool FTextSelectionComboBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        SetStyle(0xFFFF0000L, CBS_DROPDOWNLIST);
        SetStyle(~WS_CLIPCHILDREN,WS_CLIPCHILDREN);
        SetStyle(~WS_CLIPSIBLINGS,WS_CLIPSIBLINGS);
        if(!CreateThings())
                return False;
        return True;
}

Bool FTextSelectionComboBox::CreateThings() {
        FRect r;
        WmsQueryWindowRect(GetHwnd(), &r);
        //remember full height and reduce height to 1 row
        droppedDownHeight = (int)r.GetHeight();
        WmsSetWindowPos(GetHwnd(), 0, 0,0, r.GetWidth(),1, SWP_SIZE);

        //create editthing
        FSLE *sle = new FSLE;
        if(!sle)
                return False;
        FRect slerect=r;
        slerect.SetBottom(slerect.GetTop()+1);
        slerect.SetRight(slerect.GetRight()-1);
        if(!sle->Create(this, CBID_EDIT, &slerect, FSLE::sle_readonly)) {
                delete sle;
                return False;
        }

        dropbutton = new DropButton;
        if(!dropbutton) {
                delete sle;
                return False;
        }
        FRect ddrect=r;
        ddrect.SetBottom(ddrect.GetTop()+1);
        ddrect.SetLeft(ddrect.GetRight()-1);
        if(!dropbutton->Create(this, CBID_DROP, &ddrect)) {
                delete dropbutton; dropbutton=0;
                delete sle;
                return False;
        }

        FSingleSelectionTextListBox *listbox = new FSingleSelectionTextListBox;
        if(!listbox) {
                delete sle;
                delete dropbutton; dropbutton=0;
                return False;
        }
        FRect lbrect=r;
        lbrect.SetTop(lbrect.GetTop()+1);
        if(!listbox->Create(this, CBID_LIST, &lbrect)) {
                delete listbox; listbox=0;
                delete sle; sle=0;
                return False;
        }
        listbox->Hide();

        editthing = sle;
        listboxthing = listbox;

        return True;
}

Bool FTextSelectionComboBox::lm_(FEvent &ev) {
        if(!FSelectionComboBox::lm_(ev)) return False;
        if(ev.GetID()==LM_SETITEMTEXT) {
                //if the item is the current item then we have update the entryfield too
                int idx=(sint16)UINT161FROMMP(ev.mp1);
                if(idx==findCurrentItem()) {
                        char buf[256];
                        WmsSendMsg(listboxthing->GetHwnd(), LM_QUERYITEMTEXT, MPFROM2UINT16(idx,256), MPFROMP(buf));
                        WmsSendMsg(editthing->GetHwnd(), WM_SETTEXT, MPFROMP(buf), 0);
                }
        }
        return True;
}

void FTextSelectionComboBox::selectItem(int idx) {
        //we need to set the text of the entryfield too
        FSelectionComboBox::selectItem(idx);
        char buf[256];
        if(idx>=0) {
                WmsSendMsg(listboxthing->GetHwnd(), LM_QUERYITEMTEXT, MPFROM2UINT16(idx,256), MPFROMP(buf));
        } else {
                buf[0]='\0';
        }
        WmsSendMsg(editthing->GetHwnd(), WM_SETTEXT, MPFROMP(buf), 0);
}



//
class OD_Edit : public FControl, public FFocusHandler {
        FODSelectionComboBox *combo;
public:
        OD_Edit(FODSelectionComboBox *c)
          : FControl(), FFocusHandler(this), combo(c)
          {}
        unsigned QueryDlgCode() { return DLGC_LISTBOX; }
        Bool Paint();
        Bool GotFocus(FSetFocusEvent&) {
                WmsInvalidateRect(GetHwnd(),0,False);
                return False;
        }
        Bool LostFocus(FKillFocusEvent&) {
                WmsInvalidateRect(GetHwnd(),0,False);
                return False;
        }
};

Bool OD_Edit::Paint() {
        WmsHDC hdc=WmsBeginPaint(GetHwnd());
        if(hdc) {
                int idx=combo->GetCurrentSelection();
                OWNERITEM oi;
                oi.controlID = WmsQueryWindowID(WmsQueryWindowParent(GetHwnd()));
                oi.controlHwnd = GetHwnd();
                WmsQueryWindowRect(GetHwnd(), &oi.itemRect);
                oi.hdc = hdc;

                if(idx>=0) {
                        oi.itemID = (uint32)idx;
                        oi.itemState = OIS_SELECTED;
                        char buf[256];
                        WmsSendMsg(combo->GetHwnd(), LM_QUERYITEMTEXT, MPFROM2UINT16(idx,256), MPFROMP(buf));
                        oi.itemData1 = (uint32)buf;
                        combo->GetItemData(idx, (long*)&oi.itemData2);
                } else {
                        oi.itemID = (uint32)-1;
                        oi.itemState = OIS_FOCUSED;
                        oi.itemData1 = 0;
                        oi.itemData2 = 0;
                }
                WmsSendMsg(WmsQueryWindowOwner(GetHwnd()), WM_DRAWITEM, MPFROMUINT16(oi.controlID), MPFROMP(&oi));

                WmsEndPaint(hdc);
                return True;
        } else
                return False;
}


class Combo_ODListbox : public FSingleSelectionODListBox {
public:
        Combo_ODListbox(FOwnerDrawOwner *powner)
          : FSingleSelectionODListBox(powner)
          {}
        int GetItemHeight() { return 1; }
        Bool OwneeDraw(FDrawItemEvent&) { return False; }
};

//
FODSelectionComboBox::FODSelectionComboBox(FOwnerDrawOwner *powner)
  : FSelectionComboBox(),
    FOwnerDrawOwnee(this,powner),
    o(powner)
{}

FODSelectionComboBox::~FODSelectionComboBox() {
}

Bool FODSelectionComboBox::Create(FWnd *pParent, int ID, FRect *rect) {
        if(!FSelectionComboBox::Create(pParent,ID,CBS_DROPDOWNLIST,rect))
                return False;
        if(!CreateThings()) {
                Destroy();
                return False;
        } else
                return True;
}

Bool FODSelectionComboBox::AddItem(long itemData) {
        return InsertItem(-1,itemData);
}

Bool FODSelectionComboBox::InsertItem(int index, long itemData) {
        char buf = '\0';
        WmsMRESULT mr=WmsSendMsg(GetHwnd(), LM_INSERTITEM, MPFROMUINT16(index), MPFROMP(&buf));
        int idx=(sint16)UINT161FROMMR(mr);
        if(idx>=0) {
                WmsSendMsg(GetHwnd(), LM_SETITEMDATA, MPFROMUINT16(idx), MPFROMUINT32(itemData));
                return True;
        } else
                return False;
}

Bool FODSelectionComboBox::GetItemData(int index, long *itemData) {
        *itemData = (uint32)WmsSendMsg(GetHwnd(), LM_QUERYITEMDATA, MPFROMUINT16(index), 0);
        return True;
}

Bool FODSelectionComboBox::OwneeMeasure(FMeasureItemEvent &ev) {
        ev.SetItemHeight(GetItemHeight());
        return True;
}

Bool FODSelectionComboBox::ProcessResourceSetup(int props, char *name[], char *value[]) {
        if(!FControl::ProcessResourceSetup(props,name,value))
                return False;
        SetStyle(0xFFFF0000L, CBS_DROPDOWNLIST);
        SetStyle(~WS_CLIPCHILDREN,WS_CLIPCHILDREN);
        SetStyle(~WS_CLIPSIBLINGS,WS_CLIPSIBLINGS);
        if(!CreateThings())
                return False;
        return True;
}

void FODSelectionComboBox::selectItem(int idx) {
        FSelectionComboBox::selectItem(idx);
        WmsInvalidateRect(editthing->GetHwnd(),0,False);
}

Bool FODSelectionComboBox::CreateThings() {
        FRect r;
        WmsQueryWindowRect(GetHwnd(), &r);
        //remember full height and reduce height to 1 row
        droppedDownHeight = (int)r.GetHeight();
        WmsSetWindowPos(GetHwnd(), 0, 0,0, r.GetWidth(),1, SWP_SIZE);

        //create editthing
        OD_Edit *edit = new OD_Edit(this);
        if(!edit)
                return False;
        FRect editrect=r;
        editrect.SetBottom(editrect.GetTop()+1);
        editrect.SetRight(editrect.GetRight()-1);
        if(!edit->Create(this, CBID_EDIT, "", 0, &editrect)) {
                delete edit;
                return False;
        }

        dropbutton = new DropButton;
        if(!dropbutton) {
                delete edit;
                return False;
        }
        FRect ddrect=r;
        ddrect.SetBottom(ddrect.GetTop()+1);
        ddrect.SetLeft(ddrect.GetRight()-1);
        if(!dropbutton->Create(this, CBID_DROP, &ddrect)) {
                delete dropbutton; dropbutton=0;
                delete edit;
                return False;
        }

        Combo_ODListbox *listbox = new Combo_ODListbox(o);
        if(!listbox) {
                delete edit;
                delete dropbutton; dropbutton=0;
                return False;
        }
        FRect lbrect=r;
        lbrect.SetTop(lbrect.GetTop()+1);
        if(!listbox->Create(this, CBID_LIST, &lbrect)) {
                delete listbox; listbox=0;
                delete edit; edit=0;
                return False;
        }
        listbox->Hide();

        editthing = edit;
        listboxthing = listbox;

        return True;
}

