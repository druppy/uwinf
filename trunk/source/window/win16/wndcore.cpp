/*************************************************************************
FILE
  WndCore.Cpp
DESCRIPTION
  This file contains the core of BIF/Window. It implements FWndMan,
  FHandler, FWindow and FDesktop
HOST
  BC40, SC61, WC10
AUTHOR
  BL, ISJ
HISTORY
  02.06.94 : BL   Start. (main code in one day)
  25.07.94 : BL   Converted to the real BIF project.
  10.10.94 : BL   development
  94-11-?? : ISJ  fixes and more development
  94-12-21 : ISJ  Moved window.cpp into this file
  95-01-?? : BL   changed subclassing in window initialization so
                  more messages are caught
  95-02-01 : ISJ  made it work again :-)
  95-02-17 : ISJ  Prepared the source for DLLing
  95-03-10 : ISJ  Changed the FWnd::PumpEvent and keyboardinterface mechanism
  95-03-11 : ISJ  Made it DLLable (BW_DATA macro and other changes)
  95-03-14 : ISJ  Made BC40 happy (operator new)
  95-04-17 : ISJ  Windows-window class is now registered with a null brush
  96-06-02 : ISJ  Removed DLL support (was too ugly)
  97-11-01 : ISJ  Changed scan for pump windows to ask most recently
                  installed PumpWindow first
*************************************************************************/
#include <windows.h>
#include <bifimex.w16>
#include <window\winc\wndmisc.h>
#include <window\win16\wndcore.h>
#include <window\win16\applicat.h>

#include "dbglog.h"

#include <stddef.h>

//for memset only:
#include <string.h>

static FDesktop *pDesktop=0;

static FWndMan *g_pWndMan;

FDesktop * BIFFUNCTION GetDesktop( void )
{
        if(!pDesktop) pDesktop=new FDesktop;
        return pDesktop;
}

FWndMan * BIFFUNCTION GetWndMan( void )
{
        return( g_pWndMan );
}

// Global definitions for this module

#define BIFWNDNAME "BifWndClass"        // The name of the global window class

LRESULT CALLBACK BifInitProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

////////////////////////////
//  FHandler class implementation
//

/*************************************************************************
FUNCTION
  FHandler( FWnd *pWnd )
PARAMETER
  pWnd : The this pointer to the "owner" window.
VISIBILITY
  public of FHandler
DESCRIPTION
  This is the ONLY legal constructor of the FHandler base class.
  In order to be aware of the addHandlre function in the FWnd class
*************************************************************************/
FHandler::FHandler( FWnd *pWnd )
{
        TRACE_METHOD1("FHandler::FHandler");
        m_pDispatch = NULL;
        m_pWnd = pWnd;
        pWnd->addHandler(this);
}

/*************************************************************************
FUNCTION
  ~FHandler( void )
VISIBILITY
  public of FHandler
DESCRIPTION
  The destructor de-registers the handler in the window
*************************************************************************/
FHandler::~FHandler()
{
        TRACE_METHOD1("FHandler::~FHandler");
        m_pWnd->removeHandler(this);
}

/*************************************************************************
FUNCTION
  Bool DoDispatch( FEvent &event )
PARAMETER
  event : The event (or message) til dispatch.
RETURN
  False   event was not handled, try other handlers or default action
  True    event has been handled, and must not be processed any further
VISIBILITY
  protected of FHandler
DESCRIPTION
  Try to let the dispatchfunction process the event
*************************************************************************/
Bool FHandler::DoDispatch( FEvent &event )
{
        // tracing here degrades performance heavily...
        // TRACE_METHOD0("FHandler::DoDispatch");

        if( m_pDispatch )
                return (this->*m_pDispatch)(event);
        else {
                //no dispatch function
                return False;   //not dispatched
        }
}


//////////////////////////////
// FWnd class implementation
//

/*************************************************************************
FUNCTION
  FWnd( void )
VISIBILITY
  public of FWnd
DESCRIPTION
  Default constructor (and only)
**************************************************************************/
FWnd::FWnd( void )
{
        TRACE_METHOD1("FWnd::FWnd");
        m_hWnd = 0;
#ifndef NDEBUG
        //undefined until FWndMan::SubRegWnd() has been called
        m_pNext = (FWnd*)0xDEADBEEFL;
#endif
        m_pWndSuperProc = NULL;
        // Init the Handle chain.
        m_pFirstHndl = NULL;
}

/*************************************************************************
FUNCTION
  ~FWnd()
VISIBILITY
  public of FWnd
DESCRIPTION
  Destructor. Destroy the native window if it still exists
*************************************************************************/
FWnd::~FWnd()
{
        TRACE_METHOD1("FWnd::~FWnd");
        if( m_hWnd )
        Destroy();                              // Make some testing here later
#ifndef NDEBUG
        m_hWnd = NULL;
        m_pNext = (FWnd*)0xDEADBEEFL;
        m_pWndSuperProc = (WNDPROC)0xDEADBEEFL;
        m_pFirstHndl = (FHandler*)0xDEADBEEFL;
#endif
}

/*************************************************************************
FUNCTION
  Bool BeforeDestroy( void )
RETURN
  True : Always true ... yet.
VISIBILITY
  Protected of FWnd
DESCRIPTION
  Prepare the window to be destroyed, that is remove the subclassing.
*************************************************************************/
Bool FWnd::BeforeDestroy( void )
{
        TRACE_METHOD0("FWnd::BeforeDestroy");

        // Remove subclassing (if any)
        if( m_pWndSuperProc ) {
#if 0
                //ensure that our subclass proc is still there
                if((WNDPROC)GetWindowLong(m_hWnd,GWL_WNDPROC) == BifSubclassProc) {
                        SetWindowLong(m_hWnd,GWL_WNDPROC,(LONG)m_pWndSuperProc);
                } else {
                        //someone else has subclassed the window further
                        NOTE_TEXT0("FWnd::BeforeDestroy : cannot remove subclassing");
                }
#else
                SetWindowLong(m_hWnd,GWL_WNDPROC,(LONG)m_pWndSuperProc);
#endif
                m_pWndSuperProc = NULL;
        }
        return True;
}

/*************************************************************************
FUNCTION
  Bool Destroy( void )
Return:
  True : Roger over and done = OK
  False: Can't destroy and remove the window.
VISIBILITY
  public of FWnd
DESCRIPTION
  Destroys the window and remove it from the calling chain in the
  WinManager, this is the only way to remove a window in this claas
  library.
*************************************************************************/
Bool FWnd::Destroy( void )
{
        TRACE_METHOD1("FWnd::Destroy");

        // BeforeDestroy() is not necessary if the window is
        // DestroyWindow()'ed anyway
        if( m_hWnd ) {
                DestroyWindow( m_hWnd );
                return AfterDestroy();
        } else
                return( True );
}

/*************************************************************************
FUNCTION
  Bool AfterDestroy( void )
RETURN
  True : Always True ... yet.
VISIBILITY
  protected of FWnd
DESCRIPTION
  Tell the window that the native window has been destroyed. The window
  should deregister itself from the FWndMan window chain
*************************************************************************/
Bool FWnd::AfterDestroy( void )
{
        TRACE_METHOD0("FWnd::AfterDestroy");

        if( m_hWnd ) {
                GetWndMan()->RemoveWnd( this );
                m_hWnd = NULL;
        }
        return( True );
}

/*************************************************************************
FUNCTION
  LRESULT Default( FEvent &event )
PARAMETER
  event : The FEvent to process thue the default actions.
RETURN
  LRESULT to be return by the WNDPROC
VISIBILITY
  public of FWnd
DESCRIPTION
  Default message handler, must be called every time a message are
  not prossesed, to make sure that every part of the window get the
  chance to work with all kind of messages.
WARNING
  Don't mess with this one ...
**************************************************************************/
void FWnd::Default( FEvent &event )
{
        TRACE_METHOD0("FWnd::Default");
        LRESULT lReturn = 0L;
        //
        //       Our mini version of DefWindowsProc
        //
        switch( event.GetID()) {
                case WM_CLOSE:
                        //DefWindowProc calls DestroyWindow. this is not
                        //portable. We want the program to terminate instead.
                        //(this is the OS/2-way of handling wm_close)

                        GetCurrentApp()->TerminateApp();
                        lReturn=0;
                        break;
                case WM_NCDESTROY :
                case WM_DESTROY :
                        // This is the last message sent to a window, before death
                        AfterDestroy();
                default: // Fall-through
                        if( m_pWndSuperProc )   // Make the subclassing work
#ifdef STRICT
                                lReturn =::CallWindowProc(m_pWndSuperProc,
#else
                                lReturn =::CallWindowProc((FARPROC)m_pWndSuperProc,
#endif
                                                          event.hwnd,
                                                          event.message,
                                                          event.wParam,
                                                          event.lParam);
                        else
                                lReturn =::DefWindowProc(event.hwnd,
                                                         event.message,
                                                         event.wParam,
                                                         event.lParam);
                        break;
        } // EndSwitch
        event.SetResult( lReturn );
} // End of Default

/*************************************************************************
FUNCTION
  Bool Create(DWORD dwExStyle, LPCSTR lpszClassName, LPCSTR lpszWindowName,
              DWORD dwStyle, FRect *rect, HWND hwndParent, HMENU hMenu )
PARAMETER
  dwExStyle       : dwExStyle to the CreateEx function
  lpszClassName   : The name of the regitserd class to make.
  lpszWindowName  : The title of the window.
  dwStyle         : The normal style setting
  x, y, nWidth, nHight : The size of the window
  hwndParent      : The parent of the window
  hMenu           : The menu attached to the window.
RETURN
  True   Native window has been created
  False  Error
VISIBILITY
  public of FWnd
DESCRIPTION
  The full scale window creation, include subclassing and attaching to
  the Manager list

  lpszClassName can be the name of any preregistered window class or NULL
*************************************************************************/
Bool FWnd::Create( DWORD dwExStyle, LPCSTR lpszClassName, LPCSTR lpszWindowName,
                   DWORD dwStyle, FRect *rect, HWND hwndParent, HMENU hMenu )
{
        TRACE_METHOD1("FWnd::Create");

        HWND hWnd;
        Bool bAlienClass;

        m_pWndSuperProc = NULL;  // Prevent any wrong call

        if( lpszClassName == NULL ) {
                bAlienClass = False;
                lpszClassName = BIFWNDNAME;

                // Init our own base windows class.
                GetWndMan()->SetCreationWindow( this );
        } else
                bAlienClass = True;     // not bifwndclass

        hWnd = ::CreateWindowEx(dwExStyle,
                lpszClassName,
                lpszWindowName,
                dwStyle,
                rect->GetLeft(),
                rect->GetTop(),
                rect->GetWidth(),
                rect->GetHeight(),
                hwndParent,
                hMenu,
                GetWndMan()->GetInstance(),
                NULL );
        if( !hWnd ) {
                if(!bAlienClass)
                        GetWndMan()->GetCreationWindow();       //remove the pointer
                return False;
        }

        if( bAlienClass ) {
                //alien class, so it can't have subreg'ed itself, we must do it
                GetWndMan()->SubRegWnd(this, hWnd);
        }

        return( True );
} // End of Create


/*************************************************************************
FUNCTION
  void FWnd::Dispatch( FEvent &event )
PARAMETER
  event : The event to dispatch.
VISIBILITY
  private of FWnd (FWndMan is friend)
DESCRIPTION
  Normally, messages will arrive here (except for the very first message)
  Calls DispatchNoDefault to let the handlers have a try. If no handlers
  handled the event Default() is called.

*************************************************************************/
void FWnd::Dispatch(FEvent &event)
{
        TRACE_METHOD0("FWnd::Dispatch");

        ASSERT(m_hWnd == event.hwnd );

        if( !DispatchNoDefault(event) )
                Default(event);
} // End of Dispatch


/*************************************************************************
FUNCTION
  Bool FWnd::DispatchNoDefault( FEvent &event )
PARAMETER
  event : The event to dispatch.
RETURNS
  True  : A handler has handled the event
  False : event no handled, pass it on to the default processing
VISIBILITY
  private of FWnd (FWndMan is friend)
DESCRIPTION
  This function is called by the windows manager for every Windows
  message attended to this window. It may not be overridden or used
  from any other part of the library, therefore it is a private member
  function and can only be used from friends of the FWnd class.

  Every message event sent to the window will arrive here and sent to
  the handlers connected to the window. The last connected handler is
  called first!
*************************************************************************/
Bool FWnd::DispatchNoDefault( FEvent &event )
{
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
} // End of DispatchNoDefault


/*************************************************************************
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
*************************************************************************/
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

/*************************************************************************
FUNCTION
  addHandler( FHandler *pHndl )
VISIBILEY
  public of FWnd
PARAMETER
  pHndl : pointer to a handle to register.
DESCRIPTION
  Every new handler that is attached to a window have to call this
  function in order to be called when the window receives system
  events (MSG).
  The handler contains a pointer to the next handler
  or a NULL if is is the last in the list, which is the first handle
  who is registered.
NOTE
  If a handler has to override the functionality of another handler
  the handler have to be registered later than the handler to override.
*************************************************************************/
void FWnd::addHandler( FHandler *pHandler )
{
        TRACE_METHOD0("FWnd::addHandler");

        pHandler->m_pNext = m_pFirstHndl;
        m_pFirstHndl = pHandler;

        m_cnHandle++;
} // End of addHandler

/*************************************************************************
FUNCTION
  void removeHandler(FHandler *pHandler)
PARAMETER
  pHandler : The handler to remove.
VISIBILITY
  protected of FWnd
DESCRIPTION
  Removes a handler from the window's handler list. This MUST be
  done before the handler is destoyed.
*************************************************************************/
void FWnd::removeHandler(FHandler *pHandler)
{
        TRACE_METHOD0("FWnd::removeHandler");

        if(pHandler == m_pFirstHndl) {
                m_pFirstHndl=pHandler->m_pNext;
        } else {
                FHandler *ph=m_pFirstHndl;
                while(ph) {
                        if(ph->m_pNext == pHandler) {
                                // found it
                                ph->m_pNext = ph->m_pNext->m_pNext;
                                return;
                        }
                        ph = ph->m_pNext;
                }
                //not found - this is an error
        }
} // End of removeHandler


/*************************************************************************
FUNCTION
  Bool PostEvent( FEvent &ev )
PARAMETER
  ev : the event to post to a window.
RETURN
  True  : All is OK and done.
  False : Something went wrong. This _can_ happen if the receiving window's
          message queue is full
VISIBILITY
  public of FWnd
DESCRIPTION
  This is just a simpel wrapper function, for the FEvent Post function.
*************************************************************************/
Bool FWnd::PostEvent(const FEvent &ev)
{
        TRACE_METHOD1("FWnd::PostEvent");

        return (Bool)::PostMessage(ev.hwnd,ev.message,ev.wParam,ev.lParam);
}

/*************************************************************************
FUNCTION
  Bool SendEvent( FEvent &e )
PARAMETER
  e : the event to send to a window.
RETURN
  True : All is OK and done.
  False : Hmm, somethings wrong but what ???
VISIBILITY
  public of FWnd
DESCRIPTION
  This function sends the event to the window specified in the event. It will
  bypass Windows' mecanism if the destination window is known by the FWndMan
*************************************************************************/
Bool FWnd::SendEvent( FEvent &e )
{
        //TRACE_METHOD1("FWnd::SendEvent");

        if( m_hWnd == e.hwnd )
                Dispatch( e );
        else {
                FWnd *pWnd = GetWndMan()->FindWnd(e.hwnd);

                if( pWnd != NULL) {
                        // Send directly
                        pWnd->Dispatch( e );
                } else
                        e.SetResult(::SendMessage(e.hwnd,e.message,e.wParam,e.lParam));
        }
        return True;
}


// visiblility
Bool FWnd::Show( void )
{
        ::ShowWindow(GetHwnd(), SW_SHOW);
        return True;
}

Bool FWnd::Hide( void ) {
        ::ShowWindow(GetHwnd(), SW_HIDE);
        return True;
}

Bool FWnd::IsVisible( void )
{
        return (Bool)::IsWindowVisible(GetHwnd());
}


// enable/disable
Bool FWnd::Enable( void )
{
        return (Bool)::EnableWindow(GetHwnd(),TRUE);
}

Bool FWnd::Disable( void )
{
        return (Bool)::EnableWindow(GetHwnd(),FALSE);
}

Bool FWnd::IsEnabled( void )
{
        return (Bool)::IsWindowEnabled(GetHwnd());
}


/*************************************************************************
FUNCTION
  FWnd GetParent( void )
RETURN
  A pointer to the parent object window.
VISIBILITY
  public of FWnd
DESCRIPTION
  parent/owner relationship
  The parent of a window the the window in which the window is clipped
  The owner is the window that the window sends notification to.
*************************************************************************/
FWnd *FWnd::GetParent()
{
        if( GetStyle()&WS_CHILD ) {
                //We are a "normal" window, so ::GetParent works
                return GetWndMan()->FindWnd(::GetParent(GetHwnd()));
        } else {
                //We are a frame (=overlapped or popup)
                //so our parent is always the desktop
                return GetDesktop();
        }
}

/*************************************************************************
FUNCTION
  FWnd *SetParent( FWnd *pWnd )
PARAMETER
  pWnd : Pointer to the (new) parent window object.
RETURN
  The old parent object .. if any.
VISIBILITY
  public of FWnd
DESCRIPTION
  To make it posible to change the current window object's parent,
  with a new one.
*************************************************************************/
FWnd *FWnd::SetParent( FWnd *pWnd )
{
        if( pWnd ) {
                if( GetStyle()&WS_CHILD ) {
                        //We are a normal window
                        FWnd *oldParent=GetParent();
                        ::SetParent(GetHwnd(),pWnd->GetHwnd());
                        return oldParent;
                } else {
                        //We are a overlapped or popup window
                        //Windows doesn't support this properly
                        //<missing> what about MDI childs?
                        return NULL;
                }
        } else
                return NULL;
}

FWnd *FWnd::GetOwner()
{
        HWND hwndOwner=::GetWindow(GetHwnd(),GW_OWNER);
        if( hwndOwner==::GetDesktopWindow() )
                return GetDesktop();
        else
                return GetWndMan()->FindWnd(hwndOwner);
}

FWnd *FWnd::SetOwner( FWnd * pwnd)
{
        if( GetStyle()&WS_CHILD ) {
                //not supported under Windows
                //<missing> what about MDI childs ?
                return 0;
        } else {
                //<missing> does this really work?
                FWnd *oldOwner=GetOwner();
                ::SetParent(GetHwnd(),pwnd->GetHwnd());
                return oldOwner;
        }
}

// zorder & siblings
Bool FWnd::Zorder(zorder z, FWnd *pWnd) {
        switch(z) {
                case zorder_first: {
                        //move window to start of siblings
                        return (Bool)SetWindowPos(GetHwnd(), HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
                }
                case zorder_last: {
                        //move the window to the end of the siblings
                        return (Bool)SetWindowPos(GetHwnd(), HWND_BOTTOM, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
                }
                case zorder_before: {
                        // place the window before a window
                        if( !pWnd )
                                return False;
                        HWND hwndPrev = ::GetWindow(pWnd->GetHwnd(),GW_HWNDPREV);
                        if( !hwndPrev )
                                return Zorder(zorder_first);
                        else
                                return (Bool)SetWindowPos(GetHwnd(), hwndPrev, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
                }
                case zorder_after: {
                        //place the window after another window
                        if( !pWnd )
                                return False;
                        return (Bool)SetWindowPos(GetHwnd(), pWnd->GetHwnd(), 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
                }
                default:
                        return False;
        }
}

FWnd *FWnd::GetFirstSibling( void )
{
        return GetWndMan()->FindWnd(::GetWindow(GetHwnd(),GW_HWNDFIRST));
}

FWnd *FWnd::GetNextSibling( void )
{
        return GetWndMan()->FindWnd(::GetWindow(GetHwnd(),GW_HWNDNEXT));
}


// Misc
LONG FWnd::GetStyle( void )
{
        return ::GetWindowLong(GetHwnd(),GWL_STYLE);
}

void FWnd::SetStyle( LONG style )
{
        ::SetWindowLong(GetHwnd(), GWL_STYLE, style);
}

void FWnd::SetStyle(LONG and_mask, LONG or_mask)
{
        LONG style=GetStyle();
        style&= and_mask;
        style|= or_mask;
        SetStyle(style);
}


//////////////////////////////////
// FDesktop class implementation

FDesktop::FDesktop( void ) : FWnd()
{ 
        m_hWnd = ::GetDesktopWindow(); 
}

FDesktop::~FDesktop() 
{
        m_hWnd = NULL; 
}

int FDesktop::GetWidth( void )
{
        return GetSystemMetrics( SM_CXSCREEN );
}

int FDesktop::GetHeight( void )
{
        return GetSystemMetrics( SM_CYSCREEN );
}


/////////////////////////////////
// FWinMan class implementation

/*************************************************************************
FUNCTION
  FWndMan( HINSTANCE hInst )
VISIBILEY
  public of FWndMan
PARAMETER
  hInst : the current module instance
DESCRIPTION
  Just some init of object data.
*************************************************************************/
FWndMan::FWndMan(HINSTANCE hInst, HINSTANCE hinstPrev)
  : wm_created(RegisterWindowMessage("BIF/Window-wm_created")),
    m_hInst(hInst)
{
        TRACE_METHOD1("FWndMan::FWndMan");
        ASSERT(g_pWndMan == NULL);

        //set global pointer to us
        g_pWndMan = this;

        // Init wnd cache
        m_hWndCache = 0;
        m_nWndCnt = 0;

        // Reset the hash table.
        memset( m_aWndHash, 0, sizeof(m_aWndHash[ 0 ]) * HASHSIZE );

        // Reset the window init pointer 
        m_pInitWnd = NULL;
        
        // init keyboard window table
        pumpWindowTable=0;
        pumpWindows=0;

        //make procedure instance of subclass wndproc
        lpfnBifSubclassProc = MakeProcInstance((FARPROC)BifSubclassProc, m_hInst);

        if( !hinstPrev ) {
                // Register Windows class
                WNDCLASS wndcls;

                memset( &wndcls, 0, sizeof(wndcls));

                wndcls.style         = CS_DBLCLKS;
                wndcls.hInstance     = m_hInst;
                wndcls.hCursor       = LoadCursor( NULL, IDC_ARROW );
                wndcls.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
                wndcls.lpszClassName = BIFWNDNAME;
                wndcls.lpfnWndProc   = (WNDPROC)BifInitProc;

                // Register the window class
                ATOM atom=RegisterClass( &wndcls );
                ASSERT(atom != 0);
        }
}

/*************************************************************************
FUNCTION
  FWndMan( void )
VISIBILEY
  public of FWndMan
DESCRIPTION
  No sensible actions yet :-)
*************************************************************************/
FWndMan::~FWndMan()
{
        TRACE_METHOD1("FWndMan::~FWndMan");

        m_hInst = 0;

        for( int nLoop = 0; m_nWndCnt>0 && nLoop < HASHSIZE; nLoop++ ) {
                FWnd *pWnd = m_aWndHash[ nLoop ];
                while( pWnd ) {
                        pWnd->Destroy();
                        pWnd = pWnd->m_pNext;
                        m_nWndCnt--;
                }
        }
        ASSERT( m_nWndCnt == 0 );

        delete[] pumpWindowTable;

        //free procedure instance
        FreeProcInstance(lpfnBifSubclassProc);
        
        //unset global pointer
        g_pWndMan = 0;
}

/*************************************************************************
FUNCTION
  void FWndMan::AddWnd( FWnd *pWnd, HWND hWnd )
PARAMETER
  pWnd : pointer to the window class
  hWnd : Handle to the window
DESCRIPTION
  To add a new window to the manager use this function who
  insert it in a hash table, for speede lookup.
*************************************************************************/
void FWndMan::AddWnd( FWnd *pWnd )
{
        TRACE_METHOD0("FWndMan::AddWnd");

        ASSERT( pWnd != NULL );
        ASSERT( pWnd->IsValid() );
        ASSERT( !FindWnd(pWnd->GetHwnd()) );

        int nVal = MakeHash( pWnd->GetHwnd() );

        // insert the window into the hash chain
        pWnd->m_pNext = m_aWndHash[ nVal ];
        m_aWndHash[ nVal ] = pWnd;
        m_nWndCnt++;            // One more wnd in the hash table
} // End of AddWnd


/*************************************************************************
FUNCTION
  Bool RemoveWnd( FWnd *pWnd )
VISIBILITY
  protected of FWndMan
PARAMETER
  pWnd : pointer to the wnd class to remove from hash
RETURN
  True if all is done OK, else False.
DESCRIPTION
  To remove a window for the hash table this is the function to
  us, by notice that this function don't delete the object but
  just remove it from the window calling table.
*************************************************************************/
Bool FWndMan::RemoveWnd( FWnd *pWnd )
{
        TRACE_METHOD0("FWndMan::RemoveWnd");
        ASSERT( pWnd != NULL );

        int nVal = MakeHash( pWnd->m_hWnd );

        if( m_hWndCache == pWnd->GetHwnd()) {
                // Reset the cache
                m_hWndCache = 0;
                m_pWndCache = NULL;
        }

        //decrease window count
        m_nWndCnt--;
        if( m_nWndCnt == 0 ) // Quit if last wnd
                ::PostQuitMessage( 1 );


        // remove it from the chain
        if( pWnd == m_aWndHash[ nVal ] ) {
                //first in chain
                m_aWndHash[nVal] = pWnd->m_pNext;
#ifndef NDEBUG
                pWnd->m_pNext = (FWnd*)0xDEADBEEFL;
#endif
                return( True );
        } else {
                //must be non-first in chain
                FWnd *pprev = m_aWndHash[nVal];

                while( pprev && pprev->m_pNext != pWnd)
                       pprev = pprev->m_pNext;

                if( pprev ) {
                        pprev->m_pNext = pWnd->m_pNext;
#ifndef NDEBUG
                        pWnd->m_pNext = (FWnd*)0xDEADBEEFL;
#endif
                        return( True );
                }
                // Fall-through if not in chain (can happen)
        }
        return( False );
} // End of RemoveWnd

/*************************************************************************
FUNCTION
  FWnd *FindWnd( HWND hWnd )
VISIBILITY
  private of FWndMan
PARAMETER
  hWnd : Handle to the window to find.
RETURN
  A pointer to the windows object or NULL if any error.
DESCRIPTION
  Here is the reason why all this work-around with a hash table is
  done, and this, I hope, are a fast way of finding a window in the
  object pool, if not i'm depressed !!!.
*************************************************************************/
FWnd *FWndMan::FindWnd( HWND hWnd )
{
        TRACE_METHOD0("FWndMan::FindWnd");

        ASSERT( hWnd != 0 );

        if( hWnd == m_hWndCache )       // Make some extra speed, we can use it
                return( m_pWndCache );

        int nVal = MakeHash( hWnd );
        FWnd *pWnd = m_aWndHash[ nVal ];

        if( pWnd ) {
                while( pWnd && pWnd->m_hWnd != hWnd )
                       pWnd = pWnd->m_pNext;

                if( pWnd ) {
                        // Set the cache
                        m_hWndCache = hWnd;
                        m_pWndCache = pWnd;
                        return( pWnd );
                }
        }
        return( NULL );
} // End of FindWnd


//*** keyboard interface
Bool FWndMan::AddPumpWindow( FWnd *pwnd )
{
        TRACE_METHOD0("FWndMan::AddPumpWindow");

        int newSize = pumpWindows+1;
        FWnd **newTable = new FWnd*[newSize];

        if( newTable ) {
                if( pumpWindowTable ) {
                        memcpy(newTable,pumpWindowTable,sizeof(pumpWindowTable[0])*pumpWindows);
                        delete[] pumpWindowTable;
                }
                pumpWindowTable = newTable;
                pumpWindowTable[ pumpWindows ] = pwnd;
                pumpWindows++;
                return True;
        } else
                return False;
}

void FWndMan::RemovePumpWindow( FWnd *pwnd )
{
        TRACE_METHOD0("FWndMan::RemovePumpWindow");

        for(int i = 0; i < pumpWindows; i++) {
                if(pumpWindowTable[ i ] == pwnd) {
                        //found it
                        for(;i < pumpWindows - 1; i++) {
                                pumpWindowTable[ i ] = pumpWindowTable[ i + 1 ];
                        }
                        pumpWindows--;
                        return;
                }
        }
        //not found - ignored
}

Bool FWndMan::CheckPumpWindows( FEvent &ev )
{
        TRACE_METHOD0("FWndMan::CheckPumpWindows");

        for(int i=pumpWindows-1; i>=0; i--) {
                if(pumpWindowTable[i]->PumpEvent(ev)) {
                        return True;
                }
        }
        return False;
}


/*************************************************************************
FUNCTION
  void PumpEvent( FEvent &event )
VISIBILEY
  public of FWndMan
PARAMETER
  event : The message event to prosses.
DESCRIPTION
  To make this system swing, it is necessary to call this function
  every time a message arrives in the application. If one don't do this
  keyboard interface, character input, accelerators etc. won't work.
*************************************************************************/
void FWndMan::PumpEvent( FEvent &event )
{
        TRACE_METHOD1("FWndMan::PumpEvent");

        if(!CheckPumpWindows(event)) {
                // Send it to the rest of the app, if fall through
                ::TranslateMessage( &event );
                event.SetResult(::DispatchMessage(&event));
        }

} // End of PumpMessage

/*************************************************************************
FUNCTION
  Bool SetCreationWindow( FWnd *pWnd )
PARAMETER
  pWnd : The wnd to be created.
RETURN
  True : Just make that window.
  False : Can't make the window, try again.
MEMBER
  private of FWndMan.
DESCRIPTION
  Remember the window that is going to be created. This enables BIF to in the
  window-specific hook (wndproc/dlgproc/hook) to retreive the window and
  subclass it before the creation function returns and thus enabling BIF to
  catch more messages.
  To find out how the window is registered, please look in the 
  BifInitWndProc and BifSubclassProc.
NOTE
  Only 1 window pointer can be remembered
  It is an error to try to remember more than one window at the time.
*************************************************************************/
Bool FWndMan::SetCreationWindow( FWnd *pWnd )
{
        ASSERT( pWnd!=NULL );
        ASSERT( m_pInitWnd == NULL );
                
        m_pInitWnd = pWnd;
        
        return( True );
}

/*************************************************************************
FUNCTION
  FWnd *GetCreationWindow()
RETURN
  pointer to the remembered window, NULL if none is remembered
DESCRIPTION
  Returns the pointer previously stored with SetCreationWindow().
NOTE
  The remembered pointer can only be read once. Before the function returns
  it will clear the internal variable holding the pointer.
*************************************************************************/
FWnd *FWndMan::GetCreationWindow( void )
{
        FWnd *pWnd = m_pInitWnd;
        
        m_pInitWnd = NULL;              //this operation is read-once
        return( pWnd );
}

/*************************************************************************
FUNCTION
  void SubRegWnd( FWnd *pWnd, HWND hWnd )
MEMBER
  public of FWndMan
DESCRIPTION
  This function attaches the 'native' window with the BIF-window. The
  window handle (HWND) is stored in the BIF-window, the window is
  registered in the global hwnd,fwnd dictionary, and the window is
  subclassed
*************************************************************************/
void FWndMan::SubRegWnd( FWnd *pWnd, HWND hWnd )
{
        ASSERT( pWnd!=NULL );
        ASSERT( pWnd->m_pWndSuperProc == NULL ); //we can only subclass the window once
        ASSERT( hWnd!=NULL );
        
        // Store the window handle
        pWnd->m_hWnd = hWnd;

        // Register us at the window manager
        AddWnd( pWnd );

        // Subclass the window, by swapping the current with the new.
        pWnd->m_pWndSuperProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (LONG)lpfnBifSubclassProc);

        // Tell the handlers that the window has been created
        SendMessage( hWnd, wm_created, 0, 0);
}

/*************************************************************************
FUNCTION
  LRESULT BifInitProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
DESCRIPTION
  This is a function called only one time until it have registered the
  window that is being created, and calls SubRegWnd, to subclass
  it and to register it in the global window pointer hash table. The last
  thing done is to dispatch the current message and distribute it to
  the object being created.
*************************************************************************/
LRESULT CALLBACK __export BifInitProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
        TRACE_PROC0("BifInitProc");
        if(hWnd != NULL) {
                FWnd *pWnd = GetWndMan()->GetCreationWindow();
                if(pWnd) {
                        //remove this initwndproc from the subclass chain
                        ::SetWindowLong( hWnd, GWL_WNDPROC, (LONG)DefWindowProc );
                        //subclass and register the window
                        GetWndMan()->SubRegWnd( pWnd, hWnd );
                        //dispatch the event
                        FEvent event(hWnd,msg,wParam,lParam);
                        if(pWnd->DispatchNoDefault(event))
                                return event.GetResult();
                        else
                                return DefWindowProc(hWnd,msg,wParam,lParam);
                } else {
                        //this should not happen!
                        NOTE_TEXT0("BifInitProc: no creation window!?!");
                        return DefWindowProc(hWnd,msg,wParam,lParam);
                }
        } else {
                //hwnd is NULL (Windows sometimes sends such messages!)
                return DefWindowProc(hWnd,msg,wParam,lParam);
        }
}                       


/*************************************************************************
FUNCTION
  LRESULT BifSubclassProc( HWND hwnd, MSG msg, WPARAM wParam, LPARAM lParam )
PARAMETER
  hwnd    : The handle of the window, to sent this to
  msg     : The message type ( WM_?? )
  wParam  : Word (16 bit) parameter to this message.
  lParam  : Long (32 bit) parameter.
RETURN
  Long value depending on the message type.
DESCRIPTION
  This is a friend function to the window manager who is reciving all
  system messages to the BIF framework. It is used in all Class definitions
  all-over the class framework, and are resposible of calling the proper
  object for every msg.
*************************************************************************/
LRESULT CALLBACK __export BifSubclassProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
        TRACE_PROC0("BifSubclassProc");

        FWnd *pWnd = GetWndMan()->FindWnd( hWnd );        // Find the related window object

        if( pWnd != NULL ) {
                FEvent event(hWnd,msg,wParam,lParam);
                pWnd->Dispatch(event);
                return( event.GetResult());
        } else {
                // this should not happen unless the msg is wm_ncdestroy
                return( 0L );           
        }
}

/************************
*  End of WndCore.Cpp   *
************************/

