/***
Filename: dialog.cpp
Description:
  Implementation of FDialog
Host:
  Watcom 10.0a-10.6
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 96-05-27 Creation (migrated from PM)
  ISJ 96-08-30 Implement creation from resource templates
  ISJ 96-09-18 Optimized a bit
***/
#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_APPLICATION
#define BIFINCL_CONTROLS
#define BIFINCL_DYNMSG
#define WMSINCL_FOCUS
#define WMSINCL_MSGS
#define WMSINCL_BW
#include <bif.h>
#include <dbglog.h>
#include <string.h>
#include <stdlib.h>


//FDialogKeyboardHandler--------------------------------------------------------
Bool FDialogKeyboardHandler::CharInput(FKeyboardEvent &ev) {
        char c=ev.GetChar();
        if(c<32) c += '@';
        if(findMnemonic(c))
                return True;
        else
                return False;
}

Bool FDialogKeyboardHandler::KeyPressed(FKeyboardEvent &ev) {
        switch(ev.GetLogicalKey()) {
                case lk_up:
                case lk_left:
                        return prevControl();
                case lk_down:
                case lk_right:
                        return nextControl();
                case lk_tab:
                        if((!ev.IsCtrlDown()) && (!ev.IsAltDown())) {
                                if(ev.IsShiftDown())
                                        return prevTab();
                                else
                                        return nextTab();
                        }
                        return False;
                case lk_enter:
                case lk_numenter:
                        return enter();
                case lk_esc:
                        return escape();
                default: 
                        if(ev.IsAltDown()) {
                                //possible mnemonic
                                if(ev.GetLogicalKey()>=lk_a && ev.GetLogicalKey()<=lk_z) {
                                        if(findMnemonic((char)(ev.GetLogicalKey()-lk_a+'A')))
                                                return True;
                                }
                                char c=0;
                                switch(ev.GetLogicalKey()) {
                                        case lk_0:
                                        case lk_num0: c='0'; break;
                                        case lk_1:
                                        case lk_num1: c='1'; break;
                                        case lk_2:
                                        case lk_num2: c='2'; break;
                                        case lk_3:
                                        case lk_num3: c='3'; break;
                                        case lk_4:
                                        case lk_num4: c='4'; break;
                                        case lk_5:
                                        case lk_num5: c='5'; break;
                                        case lk_6:
                                        case lk_num6: c='6'; break;
                                        case lk_7:
                                        case lk_num7: c='7'; break;
                                        case lk_8:
                                        case lk_num8: c='8'; break;
                                        case lk_9:
                                        case lk_num9: c='9'; break;
                                }
                                if(c!=0 && findMnemonic(c))
                                        return True;
                        }
                        return False;
        }
}

static Bool isFocusControl(WmsHWND hwnd, uint32 style) {
        //is a window a control which can have focus?
        if(style&WS_DISABLED)
                return False;   //window is disabled
        if((style&WS_VISIBLE)==0)
                return False;   //window is invisible
        uint32 dlgc = (uint32)WmsSendMsg(hwnd, WM_QUERYDLGCODE, 0, 0);
        if((dlgc&DLGC_CONTROL)==0)
                return False;   //window is not a control
        dlgc &= ~DLGC_CONTROL;
        if(dlgc==DLGC_STATIC)
                return False;
        return True;
}

static WmsHWND findGroupStart(WmsHWND h) {
        for(;;) {
                if(WmsQueryWindowStyle(h)&WS_GROUP)
                        return h;
                WmsHWND p=WmsQueryWindow(h,QW_PREVSIBLING);
                if(p)
                        h=p;
                else
                        return h;
        }
}
static WmsHWND findGroupEnd(WmsHWND h) {
        for(;;) {
                WmsHWND n=WmsQueryWindow(h,QW_NEXTSIBLING);
                if(!n) return h;
                if(WmsQueryWindowStyle(n)&WS_GROUP)
                        return h;
                h = n;
        }
}

static WmsHWND findFocusChild(WmsHWND hwndDlg) {
        WmsHWND hwndFocus=WmsQueryFocus(GetDesktop()->GetHwnd());
        while(hwndFocus && WmsQueryWindowParent(hwndFocus)!=hwndDlg) {
                hwndFocus = WmsQueryWindowParent(hwndFocus);
        }
        return hwndFocus;
}

Bool FDialogKeyboardHandler::nextControl() {
        //set focus to next control within group
        WmsHWND hwndFocus=findFocusChild(GetWnd()->GetHwnd());
        if(!hwndFocus)
                return False;
        WmsHWND groupStart=findGroupStart(hwndFocus);
        WmsHWND groupEnd=findGroupEnd(hwndFocus);
        if(groupStart==groupEnd) return True; //optimize single-item groups

        WmsHWND h;
        if(hwndFocus!=groupEnd) {
                h = WmsQueryWindow(hwndFocus, QW_NEXTSIBLING);
                while(h) {
                        uint32 s=WmsQueryWindowStyle(h);
                        if(isFocusControl(h,s))
                                break;
                        if(h==groupEnd) {h=0; break; }
                        h = WmsQueryWindow(h, QW_NEXTSIBLING);
                }
        } else
                h=0;
        if(!h) {
                h = groupStart;
                while(h) {
                        uint32 s=WmsQueryWindowStyle(h);
                        if(isFocusControl(h,s))
                                break;
                        if(h==hwndFocus) {h=0; break; }
                        h = WmsQueryWindow(h, QW_NEXTSIBLING);
                }
        }
        if(h) {
                WmsSetFocus(GetDesktop()->GetHwnd(), h);
                return True;
        } else
                return False;
}

Bool FDialogKeyboardHandler::prevControl() {
        //set focus to next control within group
        WmsHWND hwndFocus=findFocusChild(GetWnd()->GetHwnd());
        if(!hwndFocus)
                return False;
        WmsHWND groupStart=findGroupStart(hwndFocus);
        WmsHWND groupEnd=findGroupEnd(hwndFocus);
        if(groupStart==groupEnd) return True; //optimize single-item groups

        WmsHWND h;
        if(hwndFocus!=groupStart) {
                h = WmsQueryWindow(hwndFocus, QW_PREVSIBLING);
                while(h) {
                        uint32 s=WmsQueryWindowStyle(h);
                        if(isFocusControl(h,s))
                                break;
                        if(h==groupStart) {h=0; break; }
                        h = WmsQueryWindow(h, QW_PREVSIBLING);
                }
        } else
                h=0;
        if(!h) {
                h = groupEnd;
                while(h) {
                        uint32 s=WmsQueryWindowStyle(h);
                        if(isFocusControl(h,s))
                                break;
                        if(h==hwndFocus) {h=0; break; }
                        h = WmsQueryWindow(h, QW_PREVSIBLING);
                }
        }
        if(h) {
                WmsSetFocus(GetDesktop()->GetHwnd(), h);
                return True;
        } else
                return False;
}

Bool FDialogKeyboardHandler::nextTab() {
        //set focus to next control with WS_TABSTOP
        WmsHWND hwndFocus=findFocusChild(GetWnd()->GetHwnd());
        if(!hwndFocus)
                return False;
        WmsHWND h=WmsQueryWindow(hwndFocus, QW_NEXTSIBLING);
        while(h) {
                uint32 s=WmsQueryWindowStyle(h);
                if(isFocusControl(h,s) && s&WS_TABSTOP)
                        break;
                h = WmsQueryWindow(h, QW_NEXTSIBLING);
        }
        if(!h) {
                h = WmsQueryWindow(hwndFocus, QW_FIRSTSIBLING);
                while(h) {
                        if(h==hwndFocus) { h=0; break; }
                        uint32 s=WmsQueryWindowStyle(h);
                        if(isFocusControl(h,s) && s&WS_TABSTOP)
                                break;
                        h = WmsQueryWindow(h, QW_NEXTSIBLING);
                }
        }
        if(h) {
                WmsSetFocus(GetDesktop()->GetHwnd(), h);
                return True;
        } else
                return False;
}

Bool FDialogKeyboardHandler::prevTab() {
        //set focus to next control with WS_TABSTOP
        WmsHWND hwndFocus=findFocusChild(GetWnd()->GetHwnd());
        if(!hwndFocus)
                return False;
        WmsHWND h=WmsQueryWindow(hwndFocus, QW_PREVSIBLING);
        while(h) {
                uint32 s=WmsQueryWindowStyle(h);
                if(isFocusControl(h,s) && s&WS_TABSTOP)
                        break;
                h = WmsQueryWindow(h, QW_PREVSIBLING);
        }
        if(!h) {
                h = WmsQueryWindow(hwndFocus, QW_LASTSIBLING);
                while(h) {
                        if(h==hwndFocus) { h=0; break; }
                        uint32 s=WmsQueryWindowStyle(h);
                        if(isFocusControl(h,s) && s&WS_TABSTOP)
                                break;
                        h = WmsQueryWindow(h, QW_PREVSIBLING);
                }
        }
        if(h) {
                WmsSetFocus(GetDesktop()->GetHwnd(), h);
                return True;
        } else
                return False;
}

Bool FDialogKeyboardHandler::escape() {
        //act as if a cancel button was pressed (even if there is none)
        WmsPostMsg(GetWnd()->GetHwnd(), WM_COMMAND, MPFROMUINT16(DID_CANCEL), MPFROM2UINT16(CMDSRC_PUSHBUTTON,False));
        return True;
}

Bool FDialogKeyboardHandler::enter() {
        WmsHWND hwndFocus=findFocusChild(GetWnd()->GetHwnd());
        if(!hwndFocus)
                return False;
        //if a push button has the focus then click it
        uint32 dlgc = (uint32)WmsSendMsg(hwndFocus, WM_QUERYDLGCODE, 0,0);
        if(dlgc&DLGC_CONTROL && (dlgc&0x000f)==DLGC_PUSHBUTTON) {
                WmsSendMsg(hwndFocus, BM_CLICK, 0,0);
                return True;
        }

        //find a default pushbutton in the dialog
        WmsHWND h=WmsQueryWindow(GetWnd()->GetHwnd(), QW_FIRSTCHILD);
        while(h) {
                uint32 s=WmsQueryWindowStyle(h);
                if(isFocusControl(h,s)) {
                        dlgc = (uint32)WmsSendMsg(h, WM_QUERYDLGCODE, 0,0);
                        if(dlgc&DLGC_CONTROL && (dlgc&0x000f)==DLGC_PUSHBUTTON && (dlgc&DLGC_DEFAULT)) {
                                WmsSendMsg(h, BM_CLICK, 0,0);
                                return True;
                        }
                }
                h = WmsQueryWindow(h, QW_NEXTSIBLING);
        }
        return False;
}

Bool FDialogKeyboardHandler::findMnemonic(char c) {
        WmsHWND h = WmsQueryWindow(GetWnd()->GetHwnd(), QW_FIRSTCHILD);
        while(h) {
                if((Bool)WmsSendMsg(h, WM_MATCHMNEMONIC, MPFROMUINT16(c), 0)) {
                        uint32 dlgc=(uint32)WmsSendMsg(h, WM_QUERYDLGCODE, 0,0);
                        if(dlgc&DLGC_CONTROL) {
                                switch(dlgc&0x000f) {
                                        case DLGC_PUSHBUTTON: {
                                               //if it's a mnemonic for the focused button the button should be pushed
                                               if(h==WmsQueryFocus(GetDesktop()->GetHwnd()))
                                                      WmsSendMsg(h, BM_CLICK, 0,0);
                                               WmsSetFocus(GetDesktop()->GetHwnd(), h);
                                               break;
                                        }
                                        case DLGC_RADIOBUTTON:
                                        case DLGC_CHECKBOX: {
                                               //if it's a radiobutton or checkbox, click it
                                               WmsSetFocus(GetDesktop()->GetHwnd(), h);
                                               WmsSendMsg(h, BM_CLICK, 0,0);
                                               break;
                                        }
                                        case DLGC_STATIC: {
                                                //statics can't have focus, so find the next control that can
                                                while(h) {
                                                        uint32 s=WmsQueryWindowStyle(h);
                                                        if(isFocusControl(h,s)) {
                                                                WmsSetFocus(GetDesktop()->GetHwnd(),h);
                                                                break;
                                                        }
                                                        h=WmsQueryWindow(h,QW_NEXTSIBLING);
                                                }
                                                break;
                                        }
                                        default:
                                                //other control types just gets focus
                                                WmsSetFocus(GetDesktop()->GetHwnd(),h);
                                }
                                return True;
                        }
                }
                h = WmsQueryWindow(h, QW_NEXTSIBLING);
        }
        return False;
}


//DialogFocusHandler------------------------------------------------------------
static WmsMSG FDialogFocusHandler::wm_forwardfocus;

static WmsHWND findFirstTab(WmsHWND h) {
        h = WmsQueryWindow(h, QW_FIRSTCHILD);
        while(h) {
                uint32 s = WmsQueryWindowStyle(h);
                if(s&WS_TABSTOP && isFocusControl(h,s))
                        return h;
                h = WmsQueryWindow(h, QW_NEXTSIBLING);
        }
        return 0;
}

Bool FDialogFocusHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==wm_forwardfocus) {
                if(WmsQueryFocus(GetDesktop()->GetHwnd())==GetWnd()->GetHwnd())
                        WmsSetFocus(GetDesktop()->GetHwnd(),HWNDFROMMP(ev.mp1));
                return True;
        }
        if(ev.GetID()!=WM_FOCUSCHANGING)
                return False;
        if(UINT161FROMMP(ev.mp2)) {
                //gaining focus
                WmsHWND newFocus = WmsQueryFocus(GetDesktop()->GetHwnd());
                if(newFocus==GetWnd()->GetHwnd()) {
                        //the dialog itself has gained focus
                        if(wm_forwardfocus==0)
                                wm_forwardfocus = RegisterDynamicMessage("wm_forwardfocus");
                        if(!hwndSavedFocus)
                                hwndSavedFocus = findFirstTab(GetWnd()->GetHwnd());
                        if(hwndSavedFocus)
                                WmsPostMsg(GetWnd()->GetHwnd(), wm_forwardfocus, MPFROMHWND(hwndSavedFocus), 0);
                } else {
                        //a child has received the focus
                        focusChanged(newFocus);
                }
        } else {
                //losing focus
                WmsHWND hwndGaining = HWNDFROMMP(ev.mp1);
                if(WmsIsChild(GetWnd()->GetHwnd(),hwndGaining)) {
                        //focus changed within the dialog
                        focusChanged(hwndGaining);
                } else {
                        //losing focus to non-child
                        hwndSavedFocus = WmsQueryFocus(GetDesktop()->GetHwnd());
                }
        }
        return False; //let default processing occur
}


//FDialogDismissHandler
Bool FDialogDismissHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()!=WM_DISMISSDLG)
                return False;
        Dismiss(UINT161FROMMP(ev.mp1));
        return True;
}

// FDialogResourceHandler-------------------------------------------------------
Bool FDialogResourceHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_QUERYRESOURCEITEMPTR && dlgres) {
                int id=(int)UINT161FROMMP(ev.mp1);
                const DialogItemTemplate *p=dlgres->QueryDlgItems();
                while(p->dis!=DIS_END) {
                        if(p->ID==id) {
                                //found
                                ev.SetResult(MRFROMP(p));
                                return True;
                        } else
                                p++;
                }
                ev.SetResult(MRFROMP(0));
                return True;
        } else
                return False;
}

// FDialog----------------------------------------------------------------------
FDialog::FDialog()
  : FFrameWindow(),
    FCloseHandler(this),
    FCommandHandler(this),
    FDialogFocusHandler(this),
    FDialogDismissHandler(this),
    FDialogResourceHandler(this),
    keyboardhandler(this),
    hwndDefaultButton(0),
    dismissed(False)
{
        TRACE_METHOD1("FDialog::FDialog");
        //nothing
}

WmsHWND FDialog::findTheDefaultButton() {
        WmsHWND h=WmsQueryWindow(GetHwnd(), QW_FIRSTCHILD);
        while(h) {
                uint32 dlgc = (uint32)WmsSendMsg(h, WM_QUERYDLGCODE, 0,0);
                if((dlgc&DLGC_CONTROL) && (dlgc&0x000f)==DLGC_PUSHBUTTON && (dlgc&DLGC_DEFAULT))
                        return h;
                h = WmsQueryWindow(h, QW_NEXTSIBLING);
        }
        return 0;
}

static WmsHWND findRealDialogOwner(WmsHWND requestedOwner) {
        //find a frame in the parent hierarchy
        WmsHWND o=requestedOwner;
        for(;;) {
                uint32 fi = (uint32)WmsSendMsg(o, WM_QUERYFRAMEINFO, 0,0);
                if(fi&FI_FRAME) return o;
                WmsHWND p=WmsQueryWindowParent(o);
                if(p) o=p;
                else return o;
        }
}

Bool FDialog::Create(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect ) {
        Bool b=FFrameWindow::Create(pParent,Id,fcf,alf,pRect);
        if(!b) return False;
        if(CreateControls()) {
                WmsSetWindowOwner(GetHwnd(), findRealDialogOwner(pParent?pParent->GetHwnd():GetDesktop()->GetHwnd()));
                hwndDefaultButton = findTheDefaultButton();
                return True;
        } else {
                Destroy();
                return False;
        }
}

static Bool PreProcessDialogTemplate(const DialogResource *dlgres, FRect *wr, long *fcf, long *alf) {
        int props;
        char **name;
        char **value;
        if(!DialogResource::SplitSetup(dlgres->QuerySetup(),&props,&name,&value))
                return False;
        
        for(int p=0; p<props; p++) {
                if(strcmp(name[p],"x")==0 && value[p]) {
                        int x=atoi(value[p]);
                        wr->MoveRelative(x,0);
                } else if(stricmp(name[p],"y")==0 && value[p]) {
                        int y=atoi(value[p]);
                        wr->MoveRelative(0,y);
                } else if(stricmp(name[p],"width")==0 && value[p]) {
                        int cx=atoi(value[p]);
                        wr->SetWidth(cx);
                } else if(stricmp(name[p],"height")==0 && value[p]) {
                        int cy=atoi(value[p]);
                        wr->SetHeight(cy);
                } else if(stricmp(name[p],"notitle")==0) {
                        *fcf &= ~FFrameWindow::fcf_titlebar;
                } else if(stricmp(name[p],"loadicon")==0) {
                        *alf |= FFrameWindow::alf_icon;
                } else if(stricmp(name[p],"loadaccelerator")==0) {
                        *alf |= FFrameWindow::alf_accelerator;
                } else if(stricmp(name[p],"noborder")==0) {
                        *fcf &= ~(long)(FFrameWindow::fcf_border|FFrameWindow::fcf_sizeborder|FFrameWindow::fcf_sizeborder|FFrameWindow::fcf_dialogborder);
                } else if(stricmp(name[p],"border")==0) {
                        *fcf &= ~(long)(FFrameWindow::fcf_border|FFrameWindow::fcf_sizeborder|FFrameWindow::fcf_sizeborder|FFrameWindow::fcf_dialogborder);
                        *fcf |= FFrameWindow::fcf_border;
                } else if(stricmp(name[p],"sizeborder")==0) {
                        *fcf &= ~(long)(FFrameWindow::fcf_border|FFrameWindow::fcf_sizeborder|FFrameWindow::fcf_sizeborder|FFrameWindow::fcf_dialogborder);
                        *fcf |= FFrameWindow::fcf_sizeborder;
                } else if(stricmp(name[p],"dialogborder")==0) {
                        *fcf &= ~(long)(FFrameWindow::fcf_border|FFrameWindow::fcf_sizeborder|FFrameWindow::fcf_sizeborder|FFrameWindow::fcf_dialogborder);
                        *fcf |= FFrameWindow::fcf_dialogborder;
                } else if(stricmp(name[p],"minbutton")==0) {
                        *fcf |= FFrameWindow::fcf_minbutton;
                } else if(stricmp(name[p],"maxbutton")==0) {
                        *fcf |= FFrameWindow::fcf_maxbutton;
                } else if(stricmp(name[p],"hidebutton")==0) {
                        *fcf |= FFrameWindow::fcf_hidebutton;
                } else if(stricmp(name[p],"minmaxbuttons")==0) {
                        *fcf |= FFrameWindow::fcf_minmaxbuttons;
                }
        }
        DialogResource::DeallocateSetup(name,value);
        return True;
}
static Bool PostProcessDialogTemplate(FDialog *dlg, const DialogResource *dlgres) {
        int props;
        char **name;
        char **value;
        if(!DialogResource::SplitSetup(dlgres->QuerySetup(),&props,&name,&value))
                return False;
        
        for(int p=0; p<props; p++) {
                if(stricmp(name[p],"title")==0 && value[p]) {
                        dlg->SetCaption(value[p]);
                } else if(stricmp(name[p],"visible")==0) {
                        dlg->Show();
                } else if(stricmp(name[p],"hidden")==0) {
                        dlg->Hide();
                } else if(stricmp(name[p],"screenalign")==0) {
                        FMoveHandler mh(dlg);
                        int x,y; mh.GetPosition(&x,&y);
                        FPoint p(x,y);
                        WmsMapWindowPoints(GetDesktop()->GetHwnd(),dlg->GetParent()->GetHwnd(),&p,1);
                        mh.Move(p.GetX(),p.GetY());
                }
        }
        DialogResource::DeallocateSetup(name,value);
        return True;
}
Bool FDialog::Create(FWnd *pOwner, int resID) {
        const DialogResource *dlgres = GetCurrentApp()->FindDialogResource(resID);
        if(!dlgres)
                return False;
        SetResourceTemplate(dlgres);

        long fcf=fcf_sysmenu|fcf_titlebar|fcf_dialogborder;
        long alf=0;
        FRect wr(0,0,0,0);
        if(!PreProcessDialogTemplate(dlgres,&wr,&fcf,&alf))
                return False;

        Bool b=FFrameWindow::Create(pOwner,resID,fcf,alf,&wr);
        if(!b) return False;

        if(!PostProcessDialogTemplate(this,dlgres)) {
                Destroy();
                return False;
        }
        if(CreateControls()) {
                WmsSetWindowOwner(GetHwnd(), findRealDialogOwner(pOwner?pOwner->GetHwnd():GetDesktop()->GetHwnd()));
                hwndDefaultButton = findTheDefaultButton();
                return True;
        } else {
                Destroy();
                return False;
        }
}


Bool FDialog::Close(FEvent &) {
        //post a wm_command with did_cancel
        WmsPostMsg(GetHwnd(), WM_COMMAND, MPFROMUINT16(DID_CANCEL), MPFROM2UINT16(CMDSRC_PUSHBUTTON,False));
        return True;
}


void FDialog::focusChanged(WmsHWND hwndNewFocus) {
        //if the focus has moved to a pushbutton is should be set as the default
        uint32 dlgc = (uint32)WmsSendMsg(hwndNewFocus, WM_QUERYDLGCODE, 0,0);
        if(dlgc&DLGC_CONTROL && (dlgc&0x000f)==DLGC_PUSHBUTTON) {
                if((dlgc&DLGC_DEFAULT)==0) {
                        //search for default button and "undefault" it
                        WmsHWND h=findTheDefaultButton();
                        if(h)
                                WmsSendMsg(h, BM_SETDEFAULT, MPFROMUINT16(False), 0);
                        //make the button the default
                        WmsSendMsg(hwndNewFocus, BM_SETDEFAULT, MPFROMUINT16(True), 0);
                }
        } else {
                WmsHWND h=findTheDefaultButton();
                if(h && h!=hwndDefaultButton) {
                        WmsSendMsg(h, BM_SETDEFAULT, MPFROMUINT16(False), 0);
                        WmsSendMsg(hwndDefaultButton, BM_SETDEFAULT, MPFROMUINT16(True), 0);
                }
        }
}

void FDialog::Dismiss(unsigned code) {
        dismissed=True;
        result = code;
        Hide();
}

Bool FDialog::Command(FCommandEvent &ev) {
        WmsPostMsg(GetHwnd(), WM_DISMISSDLG, MPFROMUINT16(ev.GetItem()), 0);
        return True;
}


int FModalDialog::DoModal(FWnd *pOwner, int resID) {
        if(!Create(pOwner,resID))
                return DID_ERROR;

        Show(); //show dialog
        BringToFront();

        if(pOwner && !WmsIsChild(pOwner->GetHwnd(),GetHwnd())) {
                //disable owner (but not if we are a child of it)
                WmsSetWindowStyleBits(pOwner->GetHwnd(), WS_DISABLED,WS_DISABLED);
        }

        FApplication *pApp = GetCurrentApp();
        FEvent ev;
        while(IsValid() && (!dismissed) && pApp->GetEvent(ev)) {
                GetWndMan()->PumpEvent(ev);
        }

        if(pOwner && !WmsIsChild(pOwner->GetHwnd(),GetHwnd())) {
                //enable owner
                WmsSetWindowStyleBits(pOwner->GetHwnd(), 0,WS_DISABLED);
        }

        if(IsValid())
                Destroy();

        if(ev.GetID()==WM_QUIT) {
                //repost
                WmsPostQueueMsg(GetWndMan()->GetHMQ(0), WM_QUIT, ev.mp1, ev.mp2);
        }

        return result;
}

int FModalDialog::DoModal(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect) {
        if(!Create(pParent,Id,fcf,alf,pRect))
                return DID_ERROR;

        Show(); //show dialog
        BringToFront();

        if(pParent && !WmsIsChild(pParent->GetHwnd(),GetHwnd())) {
                //disable owner (but not if we are a child of it)
                WmsSetWindowStyleBits(pParent->GetHwnd(), WS_DISABLED,WS_DISABLED);
        }

        FApplication *pApp = GetCurrentApp();
        FEvent ev;
        while(IsValid() && (!dismissed) && pApp->GetEvent(ev)) {
                GetWndMan()->PumpEvent(ev);
        }

        if(pParent && !WmsIsChild(pParent->GetHwnd(),GetHwnd())) {
                //enable owner
                WmsSetWindowStyleBits(pParent->GetHwnd(), 0,WS_DISABLED);
        }

        if(IsValid())
                Destroy();

        if(ev.GetID()==WM_QUIT) {
                //repost
                WmsPostQueueMsg(GetWndMan()->GetHMQ(0), WM_QUIT, ev.mp1, ev.mp2);
        }

        return result;
}


/***
Visibility: public:
Parameters:
        iReturn   return code
Description:
        Dismiss a modal dialog. DoModal will return <iReturn>
Note:
        Dismissing a dialog is not the same as destroying it
***/
void FModalDialog::EndDialog(int iReturn) {
        TRACE_METHOD1("FDialog::EndDialog");
        Dismiss(iReturn);
}

