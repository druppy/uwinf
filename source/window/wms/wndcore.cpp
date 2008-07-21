/***
Filename: wndcore.cpp
Description:
  This file contains the ccore of BIF/Window. It implements FWndMan, FHandler
  and FWindow
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation (converted from Windows)
  ISJ 95-02-03 Fixed a serious bug in hab and hmq lookups
  ISJ 95-02-09 Fixed logical bug in FWnd::Create and recursion bug in
               BifInitProc
  ISJ 95-03-29 Fixed bug in thread deregistration (would never return if the
               thread was not the first in the list)
               Releaxed the non-null restriction in SetOwner()
  ISJ 96-04-23 Creation (converted from PM)
  ISJ 96-09-16 Changed wm_keyboard and wm_focuschanging to owner-passup instead
               of parent-passup
***/
#define BIFINCL_WINDOW
#define BIFINCL_DYNMSG
#define BIFINCL_THREAD
#define BIFINCL_SEMAPHORES
#define BIFINCL_SPINSEMAPHORES
#define WMSINCL_MSGS
#define WMSINCL_BW
#define WMSINCL_ANCHORBLOCK
#define WMSINCL_MSGQUEUE
#define WMSINCL_CAPTURE
#define WMSINCL_FOCUS
#define WMSINCL_DESKTOP
#include <bif.h>
#include "dbgLog.h"

//for memset only:
#include <string.h>

#define BIF_WM_CREATED "BIF/Window-wm_created"  //The name of the special wm_created message

//declare a mutex semaphore to protect FWndMan's internal lists
static FSpinMutexSemaphore WndManMutex;
class WndManLock {
public: WndManLock() { WndManMutex.Request(); }
        ~WndManLock() { WndManMutex.Release(); }
};


//FHandler implementation----------------------------------------------------
// In order to be aware of the addHandler function in the FWnd class
FHandler::FHandler( FWnd *pWnd )
{
        TRACE_METHOD1("FHandler::FHandler");
        m_pDispatch = NULL;
        m_pWnd = pWnd;
        pWnd->addHandler(this);
}

FHandler::~FHandler() {
        TRACE_METHOD1("FHandler::~FHandler");
        m_pWnd->removeHandler(this);
}


/***
Visibility: ?
Parameters:  event
Description:
        Try to let the dispatchfunction process the event
Return:
        False   event was not handled, try other handlers or default action
        True    event has been handled, and must not be processed any further
***/
Bool FHandler::DoDispatch( FEvent &event )
{
        //tracing here degrades performance heavily...
        //TRACE_METHOD0("FHandler::DoDispatch");

        if( m_pDispatch )
                return (this->*m_pDispatch)(event);
        else
                return False;   //not dispatched
}






//FWnd implementation--------------------------------------------------------

/***
Visibility: public
Description:
        Default constructor
***/
FWnd::FWnd() {
        TRACE_METHOD1("FWnd::FWnd");
        m_hWnd = 0;
#ifndef NDEBUG
        m_pNext = (FWnd*)0xDEADBEEFL;   //undefined until AfterCreate() has been called
#endif
        m_pFirstHndl = NULL;            // Init the Handler chain.
}

/***
Visibility: public
Parameters: none
Description:
        destructor. Destroy the native window if it still exists
***/
FWnd::~FWnd() {
        TRACE_METHOD1("FWnd::~FWnd");
        if( m_hWnd )
                Destroy();                  // Make some testing here later
#ifndef NDEBUG
        m_hWnd = NULL;
        m_pNext = (FWnd*)0xDEADBEEFL;
        m_pFirstHndl = (FHandler*)0xDEADBEEFL;
#endif
}

/***
Visibility: public
Parameters: none
Description:
    Destroyes the window at remove it from the calling chain in the
    WinManager, this is the omly way to remove a window in this claas
    library.
Return:
    TRUE : Roger over and done = OK
    FALSE : Can't destroy and remove the window.
***/
Bool FWnd::Destroy()
{
        TRACE_METHOD1("FWnd::Destroy");
        //BeforeDestroy() is not necessary if the window is DestroyWindow()'ed anyway
        if( m_hWnd ) {
                if(!WmsDestroyWindow(m_hWnd))
                        return False;
                AfterDestroy();
        }
        return True;
}

/***
Visibility: protected
Parameters: none
Description:
        Tell the window that the native window has been destroyed. The window
        should deregister itself from the wndman chain
Return:
        True
***/
Bool FWnd::AfterDestroy() {
        TRACE_METHOD0("FWnd::AfterDestroy");
        if(m_hWnd) {
                GetWndMan()->RemoveWnd(this);
        }
        m_hWnd=NULL;
        return True;
}

/***
Visibility: public
Parameters: event
Description:
    Default message handler, must be called every time a message is
    not processed, to make sure that every part of the window get a
    chance to work with all kind of messages.
Warning:
        Don't mess with this one...
***/
void FWnd::Default( FEvent &event )
{
        TRACE_METHOD0("FWnd::Default");
        WmsMRESULT lReturn = 0L;
        //
        //       Our mini version of WinDefWindowProc
        //
        WmsMPARAM mp1=event.GetMP1();
        WmsMPARAM mp2=event.GetMP2();
        switch( event.GetID()) {
                case WM_HITTEST: { //self-handling
                        if(WmsIsWindowEnabled(m_hWnd))
                                lReturn=(WmsMRESULT)HT_NORMAL;
                        else
                                lReturn=(WmsMRESULT)HT_ERROR;
                        break;
                }

                case WM_SETPOINTER: { //parent pass-up
                        WmsHWND hwndParent=WmsQueryWindowParent(m_hWnd);
                        if(hwndParent) {
                                //let the parent do its worst
                                FPoint p(UINT161FROMMP(mp1),UINT162FROMMP(mp2));
                                WmsMapWindowPoints(m_hWnd,hwndParent,&p,1);
                                if(WmsSendMsgTimeout(hwndParent,WM_SETPOINTER,MPFROM2UINT16(p.GetX(),p.GetY()),mp2,500)) {
                                        lReturn = (WmsMRESULT)True;
                                        break;
                                }
                        }
                        GetSystemArrowPointer()->UsePointer();
                        lReturn=(WmsMRESULT)True;
                        break;
                }

                case WM_MOUSEMOVE: { //cascade
                        //send a wm_setpointer to itself so the pointer shape can be changed
                        WmsSendMsg(m_hWnd,WM_SETPOINTER,mp1,MPFROMHWND(m_hWnd));
                        lReturn=0;
                        break;
                }

                case WM_BUTTON1DOWN:
                case WM_BUTTON2DOWN:
                case WM_BUTTON3DOWN:
                case WM_BUTTON1DBLCLK:
                case WM_BUTTON2DBLCLK:
                case WM_BUTTON3DBLCLK: { //cascade
                        WmsSendMsg(m_hWnd,WM_MOUSEACTIVITY,mp1,mp2);
                        lReturn=0;
                        break;
                }

                case WM_MOUSEACTIVITY: { //parent pass-up
                        WmsSendMsg(WmsQueryWindowParent(m_hWnd),WM_MOUSEACTIVITY,mp1,mp2);
                        lReturn=0;
                        break;
                }

                case WM_KEYBOARD: { //owner pass-up
                        WmsSendMsg(WmsQueryWindowOwner(m_hWnd),WM_KEYBOARD,mp1,mp2);
                        lReturn=0;
                        break;
                }

                case WM_FOCUSCHANGING: { //owner pass-up + cascade
                        if(WmsQueryFocus(HWND_DESKTOP)==m_hWnd) {
                                //*we* are gaining/losing focus
                                WmsSendMsg(m_hWnd,WM_SETFOCUS,mp1,mp2);
                        }
                        //notify owner
                        WmsHWND hwndOwner=WmsQueryWindowOwner(m_hWnd);
                        if(hwndOwner)
                                lReturn = WmsSendMsg(hwndOwner,WM_FOCUSCHANGING,mp1,mp2);
                        else
                                lReturn = (WmsMRESULT)True;
                        break;
                }

                case WM_SETTEXT: { //self-handling
                        if(prop.SetText((const char*)PFROMMP(mp1))!=0)
                                lReturn = (WmsMRESULT)False;
                        else
                                lReturn = (WmsMRESULT)True;
                        break;
                }

                case WM_QUERYTEXTLENGTH: { //self-handling
                        lReturn = (WmsMRESULT)prop.GetTextLength();
                        break;
                }

                case WM_QUERYTEXT: { //self-handling
                        prop.GetText((char*)PFROMMP(mp1),UINT161FROMMP(mp2));
                        lReturn = (WmsMRESULT)True;
                        break;
                }

                case WM_QUERYWINDOWPARAM: { //self-handling
                        if(prop.GetData(PFROMMP(mp1),UINT161FROMMP(mp2))!=0)
                                lReturn = (WmsMRESULT)False;
                        else
                                lReturn = (WmsMRESULT)True;
                        break;
                }

                case WM_SETWINDOWPARAM: { //self-handling
                        if(prop.SetData(PFROMMP(mp1),UINT161FROMMP(mp2))!=0)
                                lReturn = (WmsMRESULT)False;
                        else
                                lReturn = (WmsMRESULT)True;
                        break;
                }

                case WM_QUERYPRESPARAM: { //self-handling
                        if(prop.GetPresParam(UINT162FROMMP(mp2),PFROMMP(mp1),UINT161FROMMP(mp2))!=0)
                                lReturn = (WmsMRESULT)False;
                        else
                                lReturn = (WmsMRESULT)True;
                        break;
                }

                case WM_SETPRESPARAM: { //cascade
                        if(PFROMMP(mp1)==0) {
                                prop.RemovePresParam(UINT162FROMMP(mp2));
                                lReturn = (WmsMRESULT)True;
                        } else {
                                if(prop.SetPresParam(UINT162FROMMP(mp2),PFROMMP(mp1),UINT161FROMMP(mp2))!=0)
                                        lReturn = (WmsMRESULT)False;
                                else
                                        lReturn = (WmsMRESULT)True;
                        }
                        if(lReturn==(WmsMRESULT)True) {
                                WmsSendMsg(m_hWnd,WM_PRESPARAMCHANGED,MPFROMHWND(m_hWnd),MPFROMUINT16(UINT162FROMMP(mp2)));
                        }
                        break;
                }

                case WM_PRESPARAMCHANGED: { //ownee pass-down
                        if(HWNDFROMMP(mp1)!=m_hWnd &&
                           prop.GetPresParamLength(UINT162FROMMP(mp2))>=0) {
                                //we have overridden the presparam, so the ownees should not be notified
                                lReturn = (WmsMRESULT)0;
                        } else {
                                //notify ownees
                                WmsHMQ ourHMQ=WmsQueryWindowMsgQueue(m_hWnd);
                                for(WmsHWND hwndOwnee=WmsQueryWindow(m_hWnd,QW_FIRSTOWNEE);
                                    hwndOwnee;
                                    hwndOwnee=WmsQueryWindow(hwndOwnee,QW_NEXTOWNEE)) {
                                        //Send msg to ownees using the same msgqueue,
                                        //otherwise we can easily fill up the msgqueue
                                        if(ourHMQ==WmsQueryWindowMsgQueue(hwndOwnee))
                                                WmsSendMsg(hwndOwnee,WM_PRESPARAMCHANGED,mp1,mp2);
                                        else
                                                WmsPostMsg(hwndOwnee,WM_PRESPARAMCHANGED,mp1,mp2);
                                }
                                lReturn = (WmsMRESULT)0;
                        }
                        break;
                }

                case WM_STYLECHANGED: { //cascade
                        uint32 prev=UINT32FROMMP(mp1);
                        uint32 now=UINT32FROMMP(mp2);
                        if((prev&WS_VISIBLE) && (now&WS_VISIBLE)==0)
                                //window has been hidden
                                WmsPostMsg(m_hWnd,WM_SHOW,MPFROMUINT16(False),0);
                        else if((prev&WS_VISIBLE)==0 && (now&WS_VISIBLE))
                                //windows has been unhidden
                                WmsPostMsg(m_hWnd,WM_SHOW,MPFROMUINT16(True),0);
                        if((prev&WS_DISABLED) && (now&WS_DISABLED)==0)
                                //window has been enabled
                                WmsPostMsg(m_hWnd,WM_ENABLE,MPFROMUINT16(True),0);
                        else if((prev&WS_DISABLED)==0 && (now&WS_DISABLED))
                                //windows has been disabled
                                WmsPostMsg(m_hWnd,WM_ENABLE,MPFROMUINT16(False),0);
                        lReturn = (WmsMRESULT)0;
                        break;
                }

                case WM_DESTROY : //self-handling
                        // This is the last message sent to a window, before death
                        AfterDestroy();
                        lReturn = 0;
                default: { // Fall-through
                        WMSDISPATCHFUNC dp=GetWndMan()->GetDP(0);
                        if(dp)
                                lReturn = (*dp)(&event);
                        else
                                lReturn = 0;
                        break;
                }
        }
        event.SetResult(lReturn);
}

/***
Visibility: public
Parameters:
    dwExStyle       : dwExStyle to the CreateEx funktion
    pszWindowName   : The title of the window.
    dwStyle         : The normal style setting
    x, y, nWidth, nHight : The size of the window
    hwndParent      : The parent of the window
    hMenu           : The menu attached to the window.
Description:
    The full scale window creation, including attaching to the Manager list
    and storing the unique number.

Return:
        True   Native window has been created
        False  Error
***/
Bool FWnd::Create( WmsHWND hwndParent, const char *pszName,
                   uint32 flStyle, FRect *rect, WmsHWND hwndOwner, WmsHWND hwndInsertBehind,
                   uint16 id
                 )
{
        TRACE_METHOD1("FWnd::Create");

        WmsHWND hwnd;

        //create the 'native' WMS window
        hwnd = WmsCreateWindow(hwndParent,
                               rect->xLeft,rect->yTop,
                               rect->GetWidth(),rect->GetHeight(),
                               hwndInsertBehind,
                               hwndOwner,
                               flStyle,
                               id,
                               0
                              );
        if(hwnd) {
                GetWndMan()->SubRegWnd(this,hwnd);
                if(pszName)
                        WmsSendMsg(hwnd,WM_SETTEXT,MPFROMP(pszName),0);
                WmsSendMsg(hwnd,WM_CREATE,0,0);
                return True;
        } else
                return False;
}



/***
Visibility: private (fwndman is friend)
Parameters: event
Description:
    This function is called by the window manager for every WMS window
    message attended to this window. It may not be overwritten or used
    from any other part of the library, therefore it is a private member
    function and can only be used by friends of the FWnd class.

    Every event sent to the window will arrive here and be sent to
    the handlers connected to the window. The last connected handler is
    called first!. If none of these return TRUE this function will treat
    the event as undone and will pass the event to the Default function.
***/
void FWnd::Dispatch(FEvent &event)
{
        TRACE_METHOD0("FWnd::Dispatch");

        ASSERT(m_hWnd == event.hwnd );

        // Call the handlers connected to this window.
        FHandler *pWalk = m_pFirstHndl;

        while( pWalk ) {
                if(pWalk->DoDispatch(event))
                        return;
                pWalk = pWalk->m_pNext;
        }

        Default(event);
}

Bool FWnd::DispatchNoDefault(FEvent &event) {
        TRACE_METHOD0("FWnd::DispatchNoDefault");

        ASSERT(m_hWnd == event.hwnd );

        // Call the handlers connected to this window.
        FHandler *pWalk = m_pFirstHndl;

        while( pWalk ) {
                if(pWalk->DoDispatch(event))
                        return True;
                pWalk = pWalk->m_pNext;
        }
        return False;
}

/***
FUNCTION
  CallHandlerChain
PARAMETERS:
  event          event to call the handlers with
  after          last handler not to call. If NULL no handlers are called
  performDefault If true Default() will be called if no handlers handles the
                 event
DESCRIPTION
  This function is handy when a handler needs to let other handlers and/or the
  default action take place before it modifies the event.
***/
Bool FWnd::CallHandlerChain(FEvent &event, FHandler *after, Bool performDefault) {
        if(after!=0) {
                FHandler *pWalk=m_pFirstHndl;
                while(pWalk) {
                        if(pWalk==after) {
                                pWalk = pWalk->m_pNext;
                                break;
                        }
                        pWalk = pWalk->m_pNext;
                }
                while(pWalk) {
                        if(pWalk->DoDispatch(event))
                                return True;
                        pWalk = pWalk->m_pNext;
                }
        }
        if(performDefault) {
                Default(event);
                return True;
        } else
                return False;
}

/***
Visibility:
    public of FWnd
Parameters
    pHandler : pointer to a handel to register.
Description
    Every new handler who is attached to a window have to call this
    function in order to be called if the window receive some system
    events (MSG).
    The Handler it self contains a reference pointer to the next handler
    or a NULL if is is the last in the list, witch is the first handler
    who is regitsred.
Note:
    If a handler has to override the functionality of another handler
    the handler has to be registered later than the handler to override.
***/
void FWnd::addHandler( FHandler *pHandler )
{
        TRACE_METHOD0("FWnd::addHandler");

        pHandler->m_pNext = m_pFirstHndl;
        m_pFirstHndl = pHandler;

        m_cnHandle++;
}

void FWnd::removeHandler(FHandler *pHandler)
{
        TRACE_METHOD0("FWnd::removeHandler");
        if(pHandler==m_pFirstHndl) {
                m_pFirstHndl=pHandler->m_pNext;
        } else {
                FHandler *ph=m_pFirstHndl;
                while(ph) {
                        if(ph->m_pNext==pHandler) {
                                //found it
                                ph->m_pNext = ph->m_pNext->m_pNext;
                                return;
                        }
                        ph=ph->m_pNext;
                }
                //not found - this is an error
        }
}

Bool FWnd::PostEvent(const FEvent &e)
{
        //TRACE_METHOD1("FWnd::PostEvent");
        return (Bool)WmsPostMsg(e.hwnd,e.msg,e.mp1,e.mp2);
}

Bool FWnd::SendEvent(FEvent &e)
{
        //TRACE_METHOD1("FWnd::SendEvent");

        //We cannot optimize here as in the win16 version, because WMS
        //does some synchronization in WmsSendMsg
        e.SetResult(WmsSendMsg(e.hwnd,e.msg,e.mp1,e.mp2));

        return True;
}

// visiblility
Bool FWnd::Show() {
        return (Bool)WmsSetWindowPos(GetHwnd(), 0, 0,0,0,0, SWP_SHOW);
}

Bool FWnd::Hide() {
        return (Bool)WmsSetWindowPos(GetHwnd(), 0, 0,0,0,0, SWP_HIDE);
}

Bool FWnd::IsVisible() {
        return (Bool)WmsIsWindowVisible(GetHwnd());
}


//enable/disable
Bool FWnd::Enable() {
        return (Bool)WmsSetWindowStyleBits(GetHwnd(),0,WS_DISABLED);
}

Bool FWnd::Disable() {
        return (Bool)WmsSetWindowStyleBits(GetHwnd(),WS_DISABLED,WS_DISABLED);
}

Bool FWnd::IsEnabled() {
        return (Bool)WmsIsWindowEnabled(GetHwnd());
}



// parent/owner relationship
//The parent of a window the the window in which the window is clipped
//The owner is the window that the window sends notification to
FWnd *FWnd::GetParent() {
        WmsHWND hwndParent=WmsQueryWindowParent(GetHwnd());
        return GetWndMan()->FindWnd(hwndParent);
}

FWnd *FWnd::SetParent( FWnd *pWnd ) {
        if(pWnd) {
                FWnd *oldParent=GetParent();
                if(WmsSetWindowParent(GetHwnd(),pWnd->GetHwnd()))
                        return oldParent;
                else
                        return 0;
        } else
                return 0; //WMS can handle null parents, but BIF/Window does not
                          //normally allow this (the desktop is an exception)
}

FWnd *FWnd::GetOwner() {
        WmsHWND hwndOwner=WmsQueryWindowOwner(GetHwnd());
        return GetWndMan()->FindWnd(hwndOwner);
}

FWnd *FWnd::SetOwner(FWnd *pWnd) {
        WmsHWND hwndNewOwner=pWnd?pWnd->GetHwnd():0;
        FWnd *oldOwner=GetOwner();
        if(WmsSetWindowOwner(GetHwnd(),hwndNewOwner))
                return oldOwner;
        else
                return 0;
}



// Z-order & siblings

Bool FWnd::Zorder(zorder z, FWnd *pWnd) {
        switch(z) {
                case zorder_first: {
                        //move window to start of siblings
                        return (Bool)WmsSetWindowPos(GetHwnd(), HWND_TOP, 0,0,0,0, SWP_ZORDER);
                }
                case zorder_last: {
                        //move the window to the end of the siblings
                        return (Bool)WmsSetWindowPos(GetHwnd(), HWND_BOTTOM, 0,0,0,0, SWP_ZORDER);
                }
                case zorder_before: {
                        //place the window before a window
                        if(!pWnd) return False;
                        WmsHWND hwndPrev=WmsQueryWindow(pWnd->GetHwnd(),QW_PREVSIBLING);
                        if(!hwndPrev)
                                return Zorder(zorder_first);
                        else
                                return (Bool)WmsSetWindowPos(GetHwnd(), hwndPrev, 0,0,0,0, SWP_ZORDER);
                }
                case zorder_after: {
                        //place the window after another window
                        if(!pWnd) return False;
                        return (Bool)WmsSetWindowPos(GetHwnd(), pWnd->GetHwnd(), 0,0,0,0, SWP_ZORDER);
                }
                default:
                        return False;
        }
}

FWnd *FWnd::GetFirstSibling() {
        return GetWndMan()->FindWnd(WmsQueryWindow(GetHwnd(),QW_FIRSTSIBLING));
}

FWnd *FWnd::GetNextSibling() {
        return GetWndMan()->FindWnd(WmsQueryWindow(GetHwnd(),QW_NEXTSIBLING));
}



//Misc
uint32 FWnd::GetStyle() {
        return WmsQueryWindowStyle(GetHwnd());
}

void FWnd::SetStyle(uint32 style) {
        WmsSetWindowStyle(GetHwnd(), style);
}

void FWnd::SetStyle(uint32 and_mask, uint32 or_mask) {
        WmsSetWindowStyleBits(GetHwnd(),or_mask,~and_mask);
}



//FDesktop implementation----------------------------------------------------
//Unlike the Windows and Presentation Manager implementations the desktop is
//located in bwwdt.cpp
#include "bwwdt.h"


//FWndMan implementation-----------------------------------------------------

static FWndMan *g_pWndMan;

//Hooks
static SetStyleHook_f oldStyleHook=0;
static void StyleHook(WmsHWND hwnd, uint32 prevStyle, uint32 newStyle) {
        WmsPostMsg(hwnd, WM_STYLECHANGED, MPFROMUINT32(prevStyle),MPFROMUINT32(newStyle));
        if(oldStyleHook) (*oldStyleHook)(hwnd,prevStyle,newStyle);
}

static SWPHook_f oldSWPHook=0;
static void SWPHook(WmsSWP *swp) {
        if(swp->flags&SWP_CHANGING)
                WmsSendMsg(swp->hwnd, WM_ADJUSTWINDOWPOS, MPFROMP(swp), MPFROMUINT16(swp->flags));
        else
                WmsSendMsg(swp->hwnd, WM_WINDOWPOSCHANGED, MPFROMP(swp), MPFROMUINT16(swp->flags));
        if(oldSWPHook) (*oldSWPHook)(swp);
}

static DestroyHook_f oldPreDestroyHook=0;
static void PreDestroyHook(WmsHWND hwnd) {
        WmsSendMsg(hwnd, WM_DESTROY, 0,0);
        if(oldPreDestroyHook) (*oldPreDestroyHook)(hwnd);
}

static DestroyHook_f oldPostDestroyHook=0;
static void PostDestroyHook(WmsHWND hwnd) {
        FWnd *pwnd=GetWndMan()->FindWnd(hwnd);
        if(pwnd)
                GetWndMan()->RemoveWnd(pwnd);
        if(oldPostDestroyHook) (*oldPostDestroyHook)(hwnd);
}


/***
Visibility: public
Description:
    Just some init of object data.
***/
FWndMan::FWndMan()
  : wm_created(RegisterDynamicMessage(BIF_WM_CREATED)),
    m_threadregs(0),
    initializerThread(FThreadManager::GetCurrentThread()->GetTID())
{
        TRACE_METHOD1("FWndMan::FWndMan");
        ASSERT(g_pWndMan==NULL);

        //set global pointer to us
        g_pWndMan=this;
        
        // Init wnd cache
        m_hWndCache = 0;
        m_nWndCnt = 0;

        // Reset the hash table.
        memset( m_aWndHash, 0, sizeof(m_aWndHash[0]) * HASHSIZE );

        //set WMS hooks
        oldStyleHook=WmsSetSetStyleHook(StyleHook);
        oldSWPHook=WmsSetSWPHook(SWPHook);
        oldPreDestroyHook=WmsSetPreDestroyHook(PreDestroyHook);
        oldPostDestroyHook=WmsSetPostDestroyHook(PostDestroyHook);

        //initialize WMS
        InitializeWMS();
}

/***
Visibility: public
Description:
        Destroy all windows known to the wndman and do other clean-ups
***/
FWndMan::~FWndMan()
{
        TRACE_METHOD1("FWndMan::~FWndMan");


        BWWTerminateDesktop();

        for( int nLoop = 0; m_nWndCnt && nLoop < HASHSIZE; nLoop++ ) {
                FWnd *pWnd = m_aWndHash[ nLoop ];
                while( pWnd ) {
                        pWnd->Destroy();
                        pWnd = pWnd->m_pNext;
                        m_nWndCnt--;
                }
        }
        ASSERT( m_nWndCnt==0 );

        //remove WMS hooks
        WmsSetSetStyleHook(oldStyleHook); oldStyleHook=0;
        WmsSetSWPHook(oldSWPHook); oldSWPHook=0;
        WmsSetPreDestroyHook(oldPreDestroyHook); oldPreDestroyHook=0;
        WmsSetPostDestroyHook(oldPostDestroyHook); oldPostDestroyHook=0;

        //unset global pointer
        g_pWndMan = NULL;

        //terminate WMS
        TerminateWMS();
}



/***
Visibility: protected
Parameters:
        pWnd  pointer to the window class
Description:
        This function adds the window to wndman's window table, so it 
        later it can be found when PumpEvent() is called.
Note:
        A window can only be added once.
***/
void FWndMan::AddWnd( FWnd *pWnd)
{
        TRACE_METHOD0("FWndMan::AddWnd");

        ASSERT( pWnd != NULL );
        ASSERT( pWnd->GetHwnd() != 0 );         //ensure the hwnd has been set
        ASSERT( !FindWnd(pWnd->GetHwnd()) );    //ensure it's not already registered

        //insert the window into the hash chain
        int nVal = MakeHash( pWnd->GetHwnd() );
        WndManLock wml;
        pWnd->m_pNext=m_aWndHash[nVal];
        m_aWndHash[nVal]=pWnd;
        m_nWndCnt++;                            //One more wnd in the hash table
        
} // End of AddWnd


/***
Visibility: protected 
Parameters:
        pWnd : pointer to the wnd class to remove from hash
Return:
        True if the window was found and removed, False otherwise.
Description:
        To remove a window for the hash table this is the function to
        use, but notice that this function doesn't delete the object but
        just remove it from the window calling table.
Note:
        It _is_ allowed to call this function with a window that has already 
        been deregistered.
***/
Bool FWndMan::RemoveWnd( FWnd *pWnd )
{
        TRACE_METHOD0("FWndMan::RemoveWnd");
        ASSERT( pWnd != NULL );
        TRACE_FORMAT0("FWndMan::RemoveWnd: removing hwnd=%lx",(long)(int)pWnd->GetHwnd());

        int wndCount;
        {
                WndManLock wml; //interlocked decrement would be enough
                wndCount= --m_nWndCnt;
        }

        if( wndCount == 0) // Quit application if last window
                WmsPostQueueMsg(GetHMQ(initializerThread),WM_QUIT,0,0);


        WmsHWND hWnd=pWnd->GetHwnd();
        int nVal = MakeHash( hWnd );


        WndManLock wml;      //we need to be left alone for just 2 microseconds or so...

        //remove the window from the cache
        if( m_hWndCache == hWnd) {
                // Reset the cache
                m_hWndCache = 0;
                m_pWndCache = NULL;
        }

        //remove it from the chain
        if( pWnd==m_aWndHash[nVal] ) {
                //window is first in chain
                m_aWndHash[nVal] = pWnd->m_pNext;
#ifndef NDEBUG
                pWnd->m_pNext = (FWnd*)0xDEADBEEFL;
#endif
                return True;
        } else {
                //must be non-first in chain
                FWnd *pprev=m_aWndHash[nVal];
                while( pprev && pprev->m_pNext != pWnd)
                        pprev = pprev->m_pNext;

                if(pprev) {
                        //found in chain
                        pprev->m_pNext = pWnd->m_pNext;
#ifndef NDEBUG
                        pWnd->m_pNext = (FWnd*)0xDEADBEEFL;
#endif
                        return True;
                } else {
                        //not in chain (can happen)
                        return False;
                }
        }
} // End of RemoveWnd


/***
Visibility: private
Parameters:
        hWnd : Handle to the window to find.
Return:
        A pointer to the window object or NULL if the window could not be found.
Description:
        Here is the reason why all this work-around with a hash table is
        done, and this, I hope, are a fast way of finding a window in the
        object pool, if not I'm depressed !!!.
***/
FWnd *FWndMan::FindWnd( WmsHWND hWnd )
{
        TRACE_METHOD0("FWndMan::FindWnd");


        int nVal = MakeHash(hWnd);

        WndManLock wml;
        //check the cache.
        if( hWnd == m_hWndCache )
                return m_pWndCache;     //should happen most of the time

        FWnd *pWnd = m_aWndHash[nVal];
        while(pWnd && pWnd->m_hWnd!=hWnd)
              pWnd = pWnd->m_pNext;

        if(pWnd) {
              // Set the cache
              m_hWndCache = hWnd;
              m_pWndCache = pWnd;
        }

        return pWnd;
} // End of FindWnd



/***
Visibility:
    public of FWndMan
Parameters:
    event   The message event to process
Description:
    Dispatches the event to the destination window
***/
void FWndMan::PumpEvent( FEvent &event )
{
        //TRACE_METHOD1("FWndMan::PumpEvent");

        //this is a piece of cake compared to win16
        event.SetResult(WmsDispatchMsg(&event));
} // End of PumpMessage


/***
Visibility: global (but ought to by local to this module if possible)
Parameters:
    hwnd    The handle of the window, to sent this to
    msg     The message type ( WM_?? )
    mp1     first message-specific parameter
    mp1     second message-specific parameter
Return
    WmsMRESULT depending on the message type.
Description:
    This is the window subclassing wndproc.
***/
WmsMRESULT BifDispatchFunc(const WmsQMSG *qmsg)
{
        TRACE_PROC0("BifDispatchFunc");

        FWnd *pWnd=GetWndMan()->FindWnd(qmsg->hwnd);
        if(pWnd!=NULL) {
                FEvent event(*qmsg);
                pWnd->Dispatch(event);
                return event.GetResult();
        } else {
                //A truely native window ?!!!
                WMSDISPATCHFUNC dp=GetWndMan()->GetDP(0);
                if(dp)
                        return (*dp)(qmsg);
                else
                        return 0;
        }
}


//hab&hmq registration
static unsigned long GetCurrentTID() {
        return FThreadManager::GetCurrentThread()->GetTID();
}

/***
Visibility: public (but only people who know what they are doing should call it)
Parameters:
        tid   thread ID (if zero the current thread is assumed)
        hab   Anchor Block 
        hmq   Message Queue (can be NULLHANDLE)
Return:
        True on success, False otherwise
Description:
        This member registers the threads anchor block and hab and hmq so they 
        can later be found by misc. classes and functions.
***/
Bool FWndMan::RegisterThread(unsigned long tid, WmsHAB hab, WmsHMQ hmq) {
        TRACE_METHOD0("FWndMan::RegisterThread");
        if(tid==0) tid=GetCurrentTID();
        ThreadRegistration *reg=new ThreadRegistration;
        if(!reg) return False;

        reg->tid=tid;
        reg->hab=hab;
        reg->hmq=hmq;
        reg->prevDispatchFunc = WmsQueryMsgQueueDispatchFunction(hmq);
        WmsSetMsgQueueDispatchFunction(hmq,BifDispatchFunc);

        {
                WndManLock wml;
                reg->next=m_threadregs;
                m_threadregs=reg;
        }

        if(reg->next==0) {
                //first thread registering
                //initialize desktop
                BWWInitializeDesktop();
        }
        return True;
}

/***
Visibility: public (but only people who know what they are doing should call it)
Parameters:
        thread  thread pointer or 0 for current
Return:
        True if found, False otherwise
Description:
        This function deregisters the anchor block and message queue previously 
        registered with RegisterThread()
***/
Bool FWndMan::DeregisterThread(unsigned long tid) {
        TRACE_METHOD0("FWndMan::DeregisterThread");
        if(tid==0) tid=GetCurrentTID();

        WndManLock wml;
        ThreadRegistration *prev=0, *current=m_threadregs;
        while(current) {
                if(current->tid==tid) {
                        if(prev)
                                prev->next=current->next;
                        else
                                m_threadregs=current->next;
                        delete current;
                        return True;
                } else {
                        prev=current;
                        current=current->next;
                }
        }

        NOTE0("FWndMan::DeregisterThread: could not find thread entry");
        return False;
}

/***
Visibility: public (but only people who know what they are doing should call it)
Parameters:
Return
Description:
***/
WmsHAB FWndMan::GetHAB(unsigned long tid) {
        if(tid==0) tid=GetCurrentTID();
        WndManLock wml;
        ThreadRegistration *reg=m_threadregs;
        while(reg) {
                if(reg->tid==tid) {
                        return reg->hab;
                } else
                        reg=reg->next;
        }
        return 0;  //not found
}


/***
Visibility: public (but only people who know what they are doing should call it)
Parameters:
        thread  thread pointer or 0 for current
Return
        HMQ of the thread
        NULL if the thread doesn't have a HMQ or it isn't registered
***/
WmsHMQ FWndMan::GetHMQ(unsigned long tid) {
        if(tid==0) tid=GetCurrentTID();
        WndManLock wml;
        ThreadRegistration *reg=m_threadregs;
        while(reg) {
                if(reg->tid==tid) {
                        return reg->hmq;
                } else
                        reg=reg->next;
        }
        return 0;  //not found
}



/***
Visibility: public (but only people who know what they are doing should call it)
Parameters:
        thread  thread pointer or 0 for current
Return
Description:
***/
WMSDISPATCHFUNC FWndMan::GetDP(unsigned long tid) {
        if(tid==0) tid=GetCurrentTID();
        WndManLock wml;
        ThreadRegistration *reg=m_threadregs;
        while(reg) {
                if(reg->tid==tid) {
                        return reg->prevDispatchFunc;
                } else
                        reg=reg->next;
        }
        return 0;  //not found
}


/***
Visibility: public (but only expert should call it)
Parameters:
        pwnd    Window pointer
        hWnd    handle to the window
Description:
        This function should be called when a WMS-window has been created
        and is to be associated with the BIF-window object
***/
void FWndMan::SubRegWnd(FWnd *pwnd, WmsHWND hWnd) {
        TRACE_METHOD0("FWndMan::SubRegWnd");
        ASSERT( hWnd != NULL );                 //WMS-window must have been created
        if(hWnd!=NULL) {
                // Store the window handle
                pwnd->m_hWnd = hWnd;

                //register the window in the window list
                AddWnd(pwnd);

                //tell the handlers that the window has been created
                WmsSendMsg(hWnd,wm_created,0,0);
        }
}



FWndMan * BIFFUNCTION GetWndMan() {
        return g_pWndMan;
}

