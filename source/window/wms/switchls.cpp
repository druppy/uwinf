/***
Filename: switchls.cpp
Description:
        Implementation a a window list/task list/switch list
        A FAppWindow subclass is defined that contains the actual switchlist
        The desktop is hooked so switchlist messages sent to the desktop are
        forwarded to the switchlist frame.
History:
        ISJ 1996-09-19 Creation
***/
#define BIFINCL_WINDOW
#define BIFINCL_FRAME
#define BIFINCL_APPWINDOW
#define BIFINCL_CONTROLS
#define BIFINCL_LISTBOX
#define BIFINCL_MOUSE
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_KEYBOARDHANDLERS
#define WMSINCL_BW
#define WMSINCL_FOCUS
#define WMSINCL_MSGS
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#define BIFINCL_SPINSEMAPHORES
#include <bif.h>
#include "switchls.h"


Bool FSwitchList::AddEntry(const SwitchEntry *entry) {
        return (Bool)WmsSendMsg(GetDesktop()->GetHwnd(), WM_ADDSWITCHENTRY, MPFROMP(entry), 0);
}
Bool FSwitchList::ChangeEntry(const SwitchEntry *entry) {
        return (Bool)WmsSendMsg(GetDesktop()->GetHwnd(), WM_CHANGESWITCHENTRY, MPFROMP(entry), 0);
}
Bool FSwitchList::QueryEntry(FWnd *pwnd, SwitchEntry *entry) {
        return (Bool)WmsSendMsg(GetDesktop()->GetHwnd(), WM_QUERYSWITCHENTRY, MPFROMP(entry), MPFROMHWND(pwnd->GetHwnd()));
}
Bool FSwitchList::DeleteEntry(FWnd *pwnd) {
        return (Bool)WmsSendMsg(GetDesktop()->GetHwnd(), WM_DELETESWITCHENTRY, MPFROMHWND(pwnd->GetHwnd()), 0);
}


static void createSwitchList();
static FSpinMutexSemaphore switchCreate;
static FSwitchList switchlist;
static Bool switchlistcreated=False;
static WmsHWND hwndSwitchList=(WmsHWND)0;

FSwitchList * BIFFUNCTION GetSwitchList() {
        switchCreate.Request();
        if(!switchlistcreated) {
                switchlistcreated=True;
                switchCreate.Release();
                createSwitchList();
        } else
                switchCreate.Release();
        return &switchlist;
}


class FSwitchListFrameHandler : public FHandler {
public:
        FSwitchListFrameHandler(FWnd *pwnd)
          : FHandler(pwnd)
          { SetDispatch(dispatchFunc_t(FSwitchListFrameHandler::Dispatch)); }
        Bool Dispatch(FEvent &ev);
        virtual Bool wm_addswitchentry(const FSwitchList::SwitchEntry *entry) =0;
        virtual Bool wm_changeswitchentry(const FSwitchList::SwitchEntry *entry) =0;
        virtual Bool wm_queryswitchentry(FSwitchList::SwitchEntry *entry, WmsHWND hwnd) =0;
        virtual Bool wm_deleteswitchentry(WmsHWND hwnd) =0;
        virtual void wm_showswitchlist(Bool show) =0;
        virtual Bool wm_focuschanging(FEvent &) {return False;}
};

Bool FSwitchListFrameHandler::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_ADDSWITCHENTRY:
                        ev.SetResult((WmsMRESULT)wm_addswitchentry((const FSwitchList::SwitchEntry*)PFROMMP(ev.mp1)));
                        return True;
                case WM_CHANGESWITCHENTRY:
                        ev.SetResult((WmsMRESULT)wm_changeswitchentry((const FSwitchList::SwitchEntry*)PFROMMP(ev.mp1)));
                        return True;
                case WM_QUERYSWITCHENTRY:
                        ev.SetResult((WmsMRESULT)wm_queryswitchentry((FSwitchList::SwitchEntry*)PFROMMP(ev.mp1),HWNDFROMMP(ev.mp2)));
                        return True;
                case WM_DELETESWITCHENTRY:
                        ev.SetResult((WmsMRESULT)wm_deleteswitchentry(HWNDFROMMP(ev.mp1)));
                        return True;
                case WM_SHOWSWITCHLIST:
                        wm_showswitchlist((Bool)UINT161FROMMP(ev.mp1));
                        return True;
                case WM_FOCUSCHANGING:
                        return wm_focuschanging(ev);
                default:
                        return False;
        }
};

class FSwitchList_DesktopHook : public FHandler {
public:
        FSwitchList_DesktopHook(FWnd *pdesktop)
          : FHandler(pdesktop)
          { SetDispatch(dispatchFunc_t(FSwitchList_DesktopHook::Dispatch)); }
protected:
        Bool Dispatch(FEvent &ev);
};

Bool FSwitchList_DesktopHook::Dispatch(FEvent &ev) {
        switch(ev.GetID()) {
                case WM_ADDSWITCHENTRY:
                case WM_CHANGESWITCHENTRY:
                case WM_QUERYSWITCHENTRY:
                case WM_DELETESWITCHENTRY:
                case WM_SHOWSWITCHLIST:
                        //forward to switchlist
                        ev.SetResult(WmsSendMsg(hwndSwitchList, ev.GetID(), ev.mp1, ev.mp2));
                        return True;
                default:
                        return False;
        }
}

class FSwitchListFrame : public FAppWindow,
                         public FSwitchListFrameHandler,
                         public FKeyboardHandler,
                         public FOwnerNotifyHandler
{
        FSingleSelectionTextListBox listbox;
public:
        enum { MAXENTRIES=20 };
        struct foo : public FSwitchList::SwitchEntry {
                WmsHWND hwnd;
        } e[MAXENTRIES];
        unsigned entries;

        FSwitchListFrame()
          : FAppWindow(),
            FSwitchListFrameHandler(this),
            FKeyboardHandler(this),
            FOwnerNotifyHandler(this),
            entries(0)
          {}

        Bool Create(FWnd *pdesktop);
        Bool wm_addswitchentry(const FSwitchList::SwitchEntry *entry);
        Bool wm_changeswitchentry(const FSwitchList::SwitchEntry *entry);
        Bool wm_queryswitchentry(FSwitchList::SwitchEntry *entry, WmsHWND hwnd);
        Bool wm_deleteswitchentry(WmsHWND hwnd);
        void wm_showswitchlist(Bool show);
        Bool wm_focuschanging(FEvent &ev);

        Bool KeyPressed(FKeyboardEvent &ev);
        Bool Notify(FOwnerNotifyEvent&);
private:
        void cleanupList();
};

Bool FSwitchListFrame::Create(FWnd *pdesktop) {
        FRect dr;
        WmsQueryWindowRect(pdesktop->GetHwnd(),&dr);
        FRect r((dr.GetWidth()-22)/2, (dr.GetHeight()-10)/2, 0,0);
        r.SetWidth(22);
        r.SetHeight(10);

        if(!FFrameWindow::Create(pdesktop,
                                 -1,
                                 fcf_titlebar|
                                 fcf_hidebutton|
                                 fcf_sizeborder,
                                 alf_none,
                                 &r
                                ))
                return False;
        SetCaption("Window List");
        wm_deleteswitchentry(GetHwnd()); //remove ourselves

        FRect lbr(0,0,0,0);
        if(!listbox.Create(this, FID_CLIENT, &lbr)) {
                Destroy();
                return False;
        }
        SetClient(&listbox);

        return True;
}


Bool FSwitchListFrame::wm_addswitchentry(const FSwitchList::SwitchEntry *entry) {
        cleanupList();
        if(entries>=MAXENTRIES) return False;
        for(unsigned i=0; i<entries; i++) {
                if(e[i].hwnd==entry->pwnd->GetHwnd())
                        return False;
        }
        *(FSwitchList::SwitchEntry*)(e+entries) = *entry;
        e[entries].hwnd = entry->pwnd->GetHwnd();
        entries++;
        return True;
}

Bool FSwitchListFrame::wm_changeswitchentry(const FSwitchList::SwitchEntry *entry) {
        for(unsigned i=0; i<entries; i++) {
                if(e[i].hwnd==entry->pwnd->GetHwnd()) {
                        *(FSwitchList::SwitchEntry*)(e+i) = *entry;
                        return True;
                }
        }
        return False;
}

Bool FSwitchListFrame::wm_queryswitchentry(FSwitchList::SwitchEntry *entry, WmsHWND hwnd) {
        for(unsigned i=0; i<entries; i++) {
                if(e[i].hwnd==hwnd) {
                        *entry = e[i];
                        return True;
                }
        }
        return False;
}

Bool FSwitchListFrame::wm_deleteswitchentry(WmsHWND hwnd) {
        for(unsigned i=0; i<entries; i++) {
                if(e[i].hwnd==hwnd) {
                        for(; i<entries-1; i++)
                                e[i] = e[i+1];
                        entries--;
                        return True;
                }
        }
        return False;
}

void FSwitchListFrame::wm_showswitchlist(Bool show) {
        if(show) {
                //refresh listbox
                cleanupList();
                WmsHWND hwndFocus=WmsQueryFocus(GetDesktop()->GetHwnd());
                listbox.Clear();
                int li=0;
                for(unsigned i=0; i<entries; i++) {
                        if(!e[i].visible) continue;
                        listbox.AddItem(e[i].title);
                        WmsSendMsg(listbox.GetHwnd(), LM_SETITEMDATA, MPFROMUINT16(li), MPFROMUINT32(e[i].hwnd));
                        if(hwndFocus==e[i].hwnd || WmsIsOwnee(e[i].hwnd,hwndFocus))
                                listbox.SetCurrentSelection(li);
                        li++;
                }

                Show();
                BringToFront();
                WmsSetFocus(GetDesktop()->GetHwnd(),GetHwnd());
        } else
                Hide();
}

Bool FSwitchListFrame::wm_focuschanging(FEvent &ev) {
        Bool b=(Bool)UINT161FROMMP(ev.mp2);
        if(!b) {
                //losing
                WmsHWND other_hwnd = HWNDFROMMP(ev.mp1);
                //If we are losing to a window outside out little family we
                //hide the list
                if(other_hwnd!=GetHwnd() &&
                   !WmsIsChild(GetHwnd(),other_hwnd))
                        WmsPostMsg(GetHwnd(), WM_SHOWSWITCHLIST, MPFROMUINT16(False), 0);
        }
        return False;
}

Bool FSwitchListFrame::KeyPressed(FKeyboardEvent &ev) {
        if(ev.GetLogicalKey()==lk_esc) {
                WmsPostMsg(GetHwnd(), WM_SHOWSWITCHLIST, MPFROMUINT16(False), 0);
                return True;
        } else
                return False;
}

Bool FSwitchListFrame::Notify(FOwnerNotifyEvent &ev) {
        if(ev.GetItem()==listbox.GetID() &&
           ev.GetNotifyCode()==LN_ENTER) {
                int li = listbox.GetCurrentSelection();
                WmsHWND hwnd=(WmsHWND)WmsSendMsg(listbox.GetHwnd(), LM_QUERYITEMDATA, MPFROMUINT16(li), 0);
                for(unsigned i=0; i<entries && e[i].hwnd!=hwnd; i++) ;
                if(i<entries &&
                   e[i].jumpable &&
                   WmsIsWindow(hwnd) &&
                   WmsIsWindowEnabled(hwnd)) {
                        WmsSetWindowPos(hwnd, 0, 0,0,0,0, SWP_SHOW);
                        if(WmsIsWindowVisible(hwnd))
                                WmsSetFocus(GetDesktop()->GetHwnd(), hwnd);
                }
                return True;
        } else
                return False;
}

void FSwitchListFrame::cleanupList() {
        unsigned i=0;
        while(i<entries) {
                if(!WmsIsWindow(e[i].hwnd)) {
                        for(unsigned j=i; j<entries-1; j++)
                                e[j] = e[j+1];
                        entries--;
                } else
                        i++;
        }
}


static void createSwitchList() {
        FSwitchListFrame *wnd = new FSwitchListFrame;
        if(!wnd) return;
        wnd->Create(GetDesktop());
        hwndSwitchList = wnd->GetHwnd();
        //hook the desktop
        new FSwitchList_DesktopHook(GetDesktop());
}

