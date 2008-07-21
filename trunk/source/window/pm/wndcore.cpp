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
               thread was not the first in the list
               Releaxed the non-null restriction in SetOwner()
  ISJ 96-08-29 Changed use of critival section to mutex semaphores to protect
               FWndMan
  ISJ 97-03-03 Fixed a workaround for PM using ULONGS for window IDs, but
  restricting the range to 0-0xffff
***/
#define INCL_DOSPROCESS         /* We need DosGetInfoBlocks() */
#define INCL_DOSSEMAPHORES      /* We need mutex semaphores */
#define BIFINCL_WINDOW
#include <bif.h>
#include "dbgLog.h"

//for memset only:
#include <string.h>


#define BIFWNDNAME "BifWndClass"                //The name of the global window class

#define BIF_WM_CREATED "BIF/Window-wm_created"  //The name of the special wm_created message

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
        m_pWndSuperProc = NULL;
        m_pFirstHndl = NULL;            // Init the Handle chain.
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
        m_pWndSuperProc = (PFNWP)0xDEADBEEFL;
        m_pFirstHndl = (FHandler*)0xDEADBEEFL;
#endif
}

/***
Visibility: protected
Parameters: none
Description:
        Prepare the window to be destroyed
Return:
        True
***/
Bool FWnd::BeforeDestroy() {
        TRACE_METHOD0("FWnd::BeforeDestroy");
        //remove subclassing (if any)
        if(m_pWndSuperProc) {
                WinSubclassWindow(m_hWnd,m_pWndSuperProc);
                m_pWndSuperProc=0;
        }
        return True;
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
                WinDestroyWindow(m_hWnd);
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
    Default message handler, must be called every time a message are
    not prossesed, to make sure that every part of the window get the
    chance to work with all kind of messages.
Warning:
        Don't mess with this one...
***/
void FWnd::Default( FEvent &event )
{
        TRACE_METHOD0("FWnd::Default");
        MRESULT lReturn = 0L;
        //
        //       Our mini version of WinDefWindowProc
        //
        switch( event.GetID()) {
                case WM_DESTROY :
                        // This is the last message sent to a window, before death
                        AfterDestroy();
                default: // Fall-through
                        if(m_pWndSuperProc)   // Make the subclassing work
                                lReturn = (*m_pWndSuperProc)(event.hwnd,event.msg,event.mp1,event.mp2);
                        break;
        }
        event.SetResult(lReturn);
}

/***
Visibility: public
Parameters:
    dwExStyle       : dwExStyle to the CreateEx funktion
    lpszClassName   : The name of the regitserd class to make.
    lpszWindowName  : The title of the window.
    dwStyle         : The normal style setting
    x, y, nWidth, nHight : The size of the window
    hwndParent      : The parant of the window
    hMenu           : The menu attached to the window.
Description:
    The full scale window creation, including attaching to the Manager list
    and storring the unik number.

    pszClass can be any control, or a window registred with the
    WndProc set to the window managers WndProc.

Return:
        True   Native window has been created
        False  Error
***/
Bool FWnd::Create( HWND hwndParent, const char *pszClass, const char *pszName,
                   ULONG flStyle, FRect *rect, HWND hwndOwner, HWND hwndInsertBehind,
                   ULONG id, PVOID pCtlData, PVOID pPresParam
                 )
{
        TRACE_METHOD1("FWnd::Create");

        HWND hwnd;
        Bool alienWindowClass;
        if(pszClass==NULL) {
                pszClass = BIFWNDNAME;
                alienWindowClass=False;
        } else 
                alienWindowClass=True;

        //create the 'native' window
        if(!alienWindowClass)
                GetWndMan()->SetCreationWindow(this,0);

        hwnd = WinCreateWindow(hwndParent,
                               pszClass,
                               pszName?pszName:"",
                               flStyle,
                               rect->xLeft,rect->yBottom,
                               rect->GetWidth(),rect->GetHeight(),
                               hwndOwner,
                               hwndInsertBehind,
                               (ULONG)(id&0xFFFF),
                               pCtlData,
                               pPresParam
                              );
        if(alienWindowClass) {
                GetWndMan()->SubRegWnd(this,hwnd);
                return True;
        } else {
                //since BifInitWndProc calls SubRegWnd() for us we don't need to do it here
                if(hwnd)
                        return True;
                else {
                        GetWndMan()->GetCreationWindow(); //reset pointer
                        return False;
                }
        }
}



/***
Visibility: private (fwndman is friend)
Parameters: event
Description:
    This funktion is called by the window manager for every PM window
    message attended to this window. It may not be overwritten or used
    from any other part of the library, therefore it is a private member
    funktion and can only be used from friends of the FWnd class.

    Every message event send to the window will arrive here and send to
    the handles connected to the window. The last connected handle are
    called first!. If non of these return TRUE this funktion will tread
    the event as undone and will pass the event to the Default funktion.
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
  default action take place before is modifies the event.
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
    Every new handle who is attached to a window have to call this
    funktion in order to be called if the window recive some system
    events (MSG).
    The Handle is self contain a referance pointer to the next handle
    or a NULL if is is the last in the list, witch is the first handle
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
} // End of addHandler

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
        return (Bool)WinPostMsg(e.hwnd,e.msg,e.mp1,e.mp2);
}

Bool FWnd::SendEvent(FEvent &e)
{
        //TRACE_METHOD1("FWnd::SendEvent");

        //We cannot optimize here as in the win16 version, because OS/2 PM 
        //does some synchronization in WinSendMsg
        e.SetResult(WinSendMsg(e.hwnd,e.msg,e.mp1,e.mp2));

        return True;
}

// visiblility
Bool FWnd::Show() {
        return (Bool)WinShowWindow(GetHwnd(), TRUE);
}

Bool FWnd::Hide() {
        return (Bool)WinShowWindow(GetHwnd(), FALSE);
}

Bool FWnd::IsVisible() {
        return (Bool)WinIsWindowVisible(GetHwnd());
}


//enable/disable
Bool FWnd::Enable() {
        return (Bool)WinEnableWindow(GetHwnd(),TRUE);
}

Bool FWnd::Disable() {
        return (Bool)WinEnableWindow(GetHwnd(),FALSE);
}

Bool FWnd::IsEnabled() {
        return (Bool)WinIsWindowEnabled(GetHwnd());
}



// parent/owner relationship
//The parent of a window the the window in which the window is clipped
//The owner is the window that the window sends notification to
FWnd *FWnd::GetParent() {
        HWND hwndParent=WinQueryWindow(GetHwnd(),QW_PARENT);
        if(hwndParent==WinQueryDesktopWindow(GetWndMan()->GetHAB(),0))
                return GetDesktop();
        else
                return GetWndMan()->FindWnd(hwndParent);
}

FWnd *FWnd::SetParent( FWnd *pWnd ) {
        if(pWnd) {
                FWnd *oldParent=GetParent();
                if(WinSetParent(GetHwnd(),pWnd->GetHwnd(),TRUE))
                        return oldParent;
                else
                        return 0;
        } else
                return 0;
}

FWnd *FWnd::GetOwner() {
        HWND hwndOwner=WinQueryWindow(GetHwnd(),QW_OWNER);
        if(hwndOwner==WinQueryDesktopWindow(GetWndMan()->GetHAB(),0))
                return GetDesktop();
        else
                return GetWndMan()->FindWnd(hwndOwner);
}

FWnd *FWnd::SetOwner(FWnd *pWnd) {
        HWND hwndNewOwner=pWnd?pWnd->GetHwnd():NULLHANDLE;
        FWnd *oldOwner=GetOwner();
        if(WinSetOwner(GetHwnd(),hwndNewOwner))
                return oldOwner;
        else
                return 0;
}



// zorder & siblings

Bool FWnd::Zorder(zorder z, FWnd *pWnd) {
        switch(z) {
                case zorder_first: {
                        //move window to start of siblings
                        return (Bool)WinSetWindowPos(GetHwnd(), HWND_TOP, 0,0,0,0, SWP_ZORDER);
                }
                case zorder_last: {
                        //move the window to the end of the siblings
                        return (Bool)WinSetWindowPos(GetHwnd(), HWND_BOTTOM, 0,0,0,0, SWP_ZORDER);
                }
                case zorder_before: {
                        //place the window before a window
                        if(!pWnd) return False;
                        HWND hwndPrev=WinQueryWindow(pWnd->GetHwnd(),QW_PREV);
                        if(!hwndPrev)
                                return Zorder(zorder_first);
                        else
                                return (Bool)WinSetWindowPos(GetHwnd(), hwndPrev, 0,0,0,0, SWP_ZORDER);
                }
                case zorder_after: {
                        //place the window after another window
                        if(!pWnd) return False;
                        return (Bool)WinSetWindowPos(GetHwnd(), pWnd->GetHwnd(), 0,0,0,0, SWP_ZORDER);
                }
                default:
                        return False;
        }
}

FWnd *FWnd::GetFirstSibling() {
        return GetWndMan()->FindWnd(WinQueryWindow(GetHwnd(),QW_TOP));
}

FWnd *FWnd::GetNextSibling() {
        return GetWndMan()->FindWnd(WinQueryWindow(GetHwnd(),QW_NEXT));
}



//Misc
ULONG FWnd::GetStyle() {
        return WinQueryWindowULong(GetHwnd(),QWL_STYLE);
}

void FWnd::SetStyle(ULONG style) {
        WinSetWindowULong(GetHwnd(), QWL_STYLE, style);
}

void FWnd::SetStyle(ULONG and_mask, ULONG or_mask) {
        WinSetWindowBits(GetHwnd(),QWL_STYLE,or_mask,~and_mask);
}



//FDesktop implementation----------------------------------------------------
FDesktop::FDesktop()
  : FWnd()
{ 
        m_hWnd=HWND_DESKTOP; 
}

FDesktop::~FDesktop() {
        m_hWnd=NULL; 
}

int FDesktop::GetWidth() {
        return WinQuerySysValue(HWND_DESKTOP,SV_CXSCREEN);
}

int FDesktop::GetHeight() {
        return WinQuerySysValue(HWND_DESKTOP,SV_CYSCREEN);
}

static FDesktop theDesktop;
FDesktop * BIFFUNCTION GetDesktop() {
        return &theDesktop;
}


//FWndMan implementation-----------------------------------------------------

static FWndMan *g_pWndMan;
static HMTX hmtx_WndMan=(HMTX)0;
/***
Visibility: public
Description:
    Just some init of object data.
***/
FWndMan::FWndMan()
  : wm_created((ULONG)WinAddAtom(WinQuerySystemAtomTable(),BIF_WM_CREATED)),
    m_threadregs(0)
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

        //Create mutex to protect ourselves
        APIRET rc=DosCreateMutexSem((PSZ)0,             //unnamed
                                    &hmtx_WndMan,
                                    0,                  //private
                                    0                   //unowned
                                   );
        if(rc) {
                FATAL("FWndMan::FWndMan(): could not create mutex semaphore");
        }
}

/***
Visibility: public
Description:
        Destroy all windows known the wndman and do other clean-ups
***/
FWndMan::~FWndMan()
{
        TRACE_METHOD1("FWndMan::~FWndMan");

        FWnd *pWnd;

        for( int nLoop = 0; m_nWndCnt && nLoop < HASHSIZE; nLoop++ ) {
                pWnd = m_aWndHash[ nLoop ];
                while( pWnd ) {
                        pWnd->Destroy();
                        pWnd = pWnd->m_pNext;
                        m_nWndCnt--;
                }
        }
        ASSERT( m_nWndCnt==0 );

        //unset global pointer
        g_pWndMan = NULL;

        //deregister our special message
        WinDeleteAtom(WinQuerySystemAtomTable(),(ATOM)wm_created);

        //close mutex
        DosCloseMutexSem(hmtx_WndMan);
        hmtx_WndMan = (HMTX)0;
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
        DosRequestMutexSem(hmtx_WndMan,-1);
           pWnd->m_pNext=m_aWndHash[nVal];
           m_aWndHash[nVal]=pWnd;
           m_nWndCnt++;                            // One more wnd in the hash table
        DosReleaseMutexSem(hmtx_WndMan);
        
} // End of AddWnd


/***
Visibility: protected 
Parameters:
        pWnd : pointer to the wnd class to remove from hash
Return:
        True if the window was found and removed, False otherwise.
Description:
        To remove a window for the hash table this is the funktion to
        us, by notice that this funktion don't delete the object but
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

        DosRequestMutexSem(hmtx_WndMan,-1);
          int wndCount= --m_nWndCnt;
        DosReleaseMutexSem(hmtx_WndMan);

        if( wndCount == 0) // Quit app if last window
                WinPostQueueMsg(GetHMQ((TID)1),WM_QUIT,NULL,NULL);


        HWND hWnd=pWnd->GetHwnd();
        int nVal = MakeHash( hWnd );


        //we need to be left alone for just 2 microseconds or so...
        DosRequestMutexSem(hmtx_WndMan,-1);

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
                //others can now use the wndman again
                DosReleaseMutexSem(hmtx_WndMan);
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
                        DosReleaseMutexSem(hmtx_WndMan);
#ifndef NDEBUG
                        pWnd->m_pNext = (FWnd*)0xDEADBEEFL;
#endif
                        return True;
                } else {
                        //not in chain (can happen)
                        DosReleaseMutexSem(hmtx_WndMan);
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
FWnd *FWndMan::FindWnd( HWND hWnd )
{
        TRACE_METHOD0("FWndMan::FindWnd");

        //check the cache.
        if( hWnd == m_hWndCache )
                return m_pWndCache;     //should happen most of the time

        int nVal = MakeHash(hWnd);

        DosRequestMutexSem(hmtx_WndMan,-1);
          FWnd *pWnd = m_aWndHash[nVal];

          while(pWnd && pWnd->m_hWnd!=hWnd)
                pWnd = pWnd->m_pNext;

          if(pWnd) {
                // Set the cache
                m_hWndCache = hWnd;
                m_pWndCache = pWnd;
          }
        DosReleaseMutexSem(hmtx_WndMan);

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

        event.SetResult(WinDispatchMsg(GetHAB(),&event));
} // End of PumpMessage


/***
Visibility: global (but ought to by local to this module if possible)
Parameters:
    hwnd    The handle of the window, to sent this to
    msg     The message type ( WM_?? )
    mp1     first message-specific parameter
    mp1     second message-specific parameter
Return
    MRESULT depending on the message type.
Description:
    This is the window subclassing wndproc.
***/
MRESULT EXPENTRY BIFFUNCTION BifSubclassProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
        TRACE_PROC0("BifSubclassProc");

        FWnd *pWnd=GetWndMan()->FindWnd(hwnd);
        if(pWnd!=NULL) {
                FEvent event(hwnd,msg,mp1,mp2);
                pWnd->Dispatch(event);
                return event.GetResult();
        } else {
                //this should never happen!
                WARN_TEXT("BifSubclassProc(): don't know hwnd");
                return WinDefWindowProc(hwnd,msg,mp1,mp2);
        }
}


/***
Visibility: public (but should be local to this module)
Description:
        This wndproc is the wndproc for the native bif-window class
        When the first message arrives to a window the wndproc is replaced with 
        WinDefWindowProc, the window is registered and subclassed, and the first 
        message is pumped
***/
MRESULT EXPENTRY BIFFUNCTION BifInitWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
        FWnd *pwnd=GetWndMan()->GetCreationWindow(0);
        if(pwnd) {
                //change the wndproc to DefWindowProc
                WinSetWindowPtr(hwnd, QWP_PFNWP, (PVOID)(PFNWP)WinDefWindowProc);
                //register and subclass
                GetWndMan()->SubRegWnd(pwnd,hwnd);
                //pump message
                FEvent event(hwnd,msg,mp1,mp2);
                if(pwnd->DispatchNoDefault(event))
                        return event.GetResult();
                else
                        return WinDefWindowProc(hwnd,msg,mp1,mp2);
        } else {
                //this should not happen!
                WARN_TEXT("BifInitWndProc(): no creation window ?!?");
                return WinDefWindowProc(hwnd,msg,mp1,mp2);
        }
}


//hab&hmq registration

/***
Visibility: local to this module
Description:
        return the TID of the calling thread
***/
static TID GetCurrentThreadID() {
        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib,&ppib);
        return ptib->tib_ptib2->tib2_ultid;
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
Bool FWndMan::RegisterThread(TID tid, HAB hab, HMQ hmq) {
        TRACE_METHOD0("FWndMan::RegisterThread");
        if(tid==0) tid=GetCurrentThreadID();
        ThreadRegistration *reg=new ThreadRegistration;
        if(!reg) return False;

        reg->tid=tid;
        reg->hab=hab;
        reg->hmq=hmq;

        DosRequestMutexSem(hmtx_WndMan,-1);
          reg->next = m_threadregs;
          m_threadregs = reg;
        DosReleaseMutexSem(hmtx_WndMan);

        if(!reg->next) {
                //first thread - register window class
                APIRET rc;
                rc=WinRegisterClass(hab,
                                    BIFWNDNAME,                    //pszClassName
                                    BifInitWndProc,                //pfnWindowProc
                                    CS_MOVENOTIFY|CS_HITTEST,      //flStyle
                                    0                              //cbWindowData
                                   );
                ASSERT(rc==TRUE);
        }
        return True;
}

/***
Visibility: public (but only people who know what they are doing should call it)
Parameters:
        tid   Thread ID. If zero the current thread is assumed.
Return:
        True if found, False otherwise
Description:
        This function deregisters the anchor block and message queue previously 
        registered with RegisterThread()
***/
Bool FWndMan::DeregisterThread(TID tid) {
        TRACE_METHOD0("FWndMan::DeregisterThread");
        if(tid==0) tid=GetCurrentThreadID();
        DosRequestMutexSem(hmtx_WndMan,-1);
          ThreadRegistration *prev=0, *current=m_threadregs;
          while(current) {
                if(current->tid==tid) {
                        if(prev)
                                prev->next=current->next;
                        else
                                m_threadregs=current->next;
                        DosReleaseMutexSem(hmtx_WndMan);
                        delete current;
                        return True;
                } else {
                        prev=current;
                        current=current->next;
                }
          }
        DosReleaseMutexSem(hmtx_WndMan);

        WARN_TEXT("FWndMan::DeregisterThread: could not find thread entry");
        return False;
}

/***
Visibility: public (but only people who know what they are doing should call it)
Parameters:
Return
Description:
***/
HAB FWndMan::GetHAB(TID) {
//        if(tid==0) tid=GetCurrentThreadID();
//        DosRequestMutexSem(hmtx_WndMan,-1);
//          ThreadRegistration *reg=m_threadregs;
//          while(reg) {
//                if(reg->tid==tid) {
//                        DosExitCritSec();
//                        return reg->hab;
//                } else
//                        reg=reg->next;
//          }
//        DosReleaseMutexSem(hmtx_WndMan);
//        return NULLHANDLE;      //not found
//According to the toolkit documentation:
// "The operating system does not generally use the information supplied by the hab
//  parameter to its calls; instead, it deduces it from the identity of the thread that
//  is making the call. Thus an OS/2 application is not required to supply any
//  particular value as the hab parameter. However, in order to be portable to other
//  environments, an application must provide the hab, that is returned by the
//  WinInitialize function of the thread, to any OS/2 function that requires it."
//So we could say:
        return (HAB)12345;      //any value should do
}


/***
Visibility: public (but only people who know what they are doing should call it)
Parameters:
        tid     thread ID og 0 for current
Return
        HMQ of the thread
        NULLHANDLE if the thread doesn't have a HMQ or it isn't registered
***/
HMQ FWndMan::GetHMQ(TID tid) {
        TRACE_METHOD0("FWndMan::GetHMQ");
        if(tid==0) tid=GetCurrentThreadID();
        DosRequestMutexSem(hmtx_WndMan,-1);
          ThreadRegistration *reg=m_threadregs;
          while(reg) {
                if(reg->tid==tid) {
                        DosReleaseMutexSem(hmtx_WndMan);
                        return reg->hmq;
                } else
                        reg=reg->next;
          }
        DosReleaseMutexSem(hmtx_WndMan);
        return NULLHANDLE;
}


/***
Visibility: public (but only experts should call it)
***/
Bool FWndMan::SetCreationWindow(FWnd *pwnd, TID tid) {
        TRACE_METHOD0("FWndMan::SetCreationWindow");
        if(tid==0) tid=GetCurrentThreadID();
        DosRequestMutexSem(hmtx_WndMan,-1);
          ThreadRegistration *reg=m_threadregs;
          while(reg) {
                if(reg->tid==tid) {
                        DosReleaseMutexSem(hmtx_WndMan);
                        reg->creationWindow=pwnd;
                        return True;
                } else
                        reg=reg->next;
          }
        DosReleaseMutexSem(hmtx_WndMan);
        return False;
}



FWnd *FWndMan::GetCreationWindow(TID tid) {
        TRACE_METHOD0("FWndMan::GetCreationWindow");
        if(tid==0) tid=GetCurrentThreadID();

        DosRequestMutexSem(hmtx_WndMan,-1);
          ThreadRegistration *reg=m_threadregs;
          while(reg) {
                if(reg->tid==tid) {
                        DosReleaseMutexSem(hmtx_WndMan);
                        FWnd *pwnd=reg->creationWindow;
                        reg->creationWindow=0;
                        return pwnd;
                } else
                        reg=reg->next;
          }
        DosReleaseMutexSem(hmtx_WndMan);
        return 0;
}



/***
Visibility: public (but only expert should call it)
Parameters:
        pwnd    Window pointer
        hWnd    handle to the window
Description:
        This function should be called when a OS/2 PM-window has been created
        and is to be associated with the BIF-window object
***/
void FWndMan::SubRegWnd(FWnd *pwnd, HWND hWnd) {
        TRACE_METHOD0("FWndMan::SubRegWnd");
        ASSERT( hWnd != NULL );                 //PM-window must have been created
        ASSERT( pwnd->m_pWndSuperProc==NULL );  //we can only subclass the window once

        // Subclass the window
        pwnd->m_pWndSuperProc = WinSubclassWindow(hWnd,BifSubclassProc);

        // Store the window handle
        pwnd->m_hWnd = hWnd;

        //register the window in the window list
        AddWnd(pwnd);

        //tell the handlers that the window has been created
        WinSendMsg(hWnd,wm_created,0,0);
}



FWndMan * BIFFUNCTION GetWndMan() {
        return g_pWndMan;
}

