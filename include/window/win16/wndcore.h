/*************************************************************************
FILE
  WndCore.h
AUTHOR
  Bo Lorentsen & Ivan J›rgensen
VERSION
  ? (non yet)
DESCRIPTION
  Class prototype of all window related ellements in the BIFoundation which
  include the tree main groups :
    Event :
      Who is the container off all system events/messages and return values.

    Window :
      Identify all windows in the application, and is the servever
      of all messages sent to the window. If a message is sent to
      the system it will first "hit" the manager, who will find the
      right window to sent the event/message to, and the window will
      then ask all its handles if they wandt the event/message, and
      if not the window are responceble fore calling the Default
      member funktion, who call the system related default handler.

    Handlers :
      Every event/message for a given window are sent to one or more
      handlers that is responsible for taking the important action on a
      given system responce, and if there is some system dependen
      aktions it have to hide thise.
HISTORY:
  94-05-20  BL  Created
  95-02-07  BL  Close to complete :-)
*************************************************************************/
#ifndef __WNDCORE_H_INCLUDED
#define __WNDCORE_H_INCLUDED

/////////////////////////////////////////
//  Definition of the FEvent class
class BIFCLASS FEvent : public MSG
{
public:
        FEvent( void )
          { m_nSysReturn = 0; }
        FEvent( HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar )
          { Set(hWnd, msg, wPar, lPar); m_nSysReturn=0; }

        // Message (id) access
        UINT GetID() const
          { return message; }
        void SetID( UINT nId )
          { message = nId; }

        // WParam access
        WPARAM GetWParam() const
          { return wParam; }
        void SetWParam(WPARAM param )
          { wParam = param; }

        //LParam access
        LPARAM GetLParam() const
          { return lParam; }
        void SetLParam(LPARAM param )
          { lParam = param; }

        // hwnd access
        HWND GetHwnd() const
          { return hwnd; }
        void SetHwnd(HWND hw)
          { hwnd=hw; }

        void Set(HWND hWnd, UINT msg, WPARAM wPar, LPARAM lPar)
          {hwnd = hWnd; message = msg; wParam = wPar; lParam = lPar;}

        // Result access
        void SetResult( LRESULT lRet )
          { m_nSysReturn = lRet; }
        LRESULT GetResult() const
          { return m_nSysReturn; }

        LRESULT m_nSysReturn;   // Return value to return to the system
}; // End of FEvent


//////////////////////////////////////////
//  Definition of the FHandler class
class BIFCLASS FWnd; //forward declaration of FWnd

class BIFCLASS FHandler {
        friend class FWnd;
public:
        FHandler( FWnd *pWnd );
        virtual ~FHandler();
protected:
#if defined(__WATCOMC__) && defined(BIF_IN_DLL)
        typedef Bool (__cdecl FHandler::*dispatchFunc_t)(FEvent&);
#else
        typedef Bool (FHandler::*dispatchFunc_t)(FEvent&);
#endif
        void SetDispatch(dispatchFunc_t func)
          { m_pDispatch=func; }
        FWnd *GetWnd() const
          { return m_pWnd; }
private:
        dispatchFunc_t m_pDispatch;     // The local dispatch funktion
        Bool DoDispatch( FEvent &event );
        FHandler *m_pNext;      // Pointer to the next handler, if any. Part
                                // of a FWnd struture (DBH)
        FWnd *m_pWnd;
}; // End of FHandler


//-----------------------------------------------------------------------------
//      Definition of the FWnd class
class BIFCLASS FWnd {
        friend class BIFCLASS FWndMan;
        // Common WindowsProc for all windows
        friend LRESULT CALLBACK BifSubclassProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
public:
        FWnd( void );
        virtual ~FWnd();

        // Simple version of Create
        Bool Create(LPCSTR lpszWindowName, DWORD dwStyle, FRect *rect,
                HWND hwndParent, HMENU hMenu = NULL )
          { return Create(0L, NULL, lpszWindowName, dwStyle, rect, hwndParent, hMenu); }

        virtual Bool Destroy( void );   // Destroy the system window in any means.
protected:
        // Full scale Create
        Bool Create(DWORD dwExStyle, LPCSTR lpszClassName, LPCSTR lpszWindowName,
                    DWORD dwStyle, FRect *rect, HWND hwndParent, HMENU hMenu = NULL );

        Bool BeforeDestroy();
        Bool AfterDestroy();
public:
        // visiblility
        Bool Show();
        Bool Hide();
        Bool IsVisible();

        //enable/disable
        Bool Enable();
        Bool Disable();
        Bool IsEnabled();

        // parent/owner relationship
        FWnd *GetParent(void);
        FWnd *SetParent( FWnd *pWnd );
        FWnd *GetOwner( void );
        FWnd *SetOwner( FWnd *pWnd );

        HWND GetHwnd(void)
        { return m_hWnd; }

        Bool IsValid() const
        { return (Bool)(m_hWnd!=NULL); }

        // zorder & siblings
        enum zorder {
                zorder_first,
                zorder_last,
                zorder_before,
                zorder_after
        };

        Bool Zorder(zorder z, FWnd *pWnd = NULL );
        FWnd *GetFirstSibling();
        FWnd *GetNextSibling();

        // Event handling
        Bool PostEvent(const FEvent &event);    // Post a FEvent
        Bool SendEvent(FEvent &event);          // Send a FEvent

        // Miscellaneous
        LONG GetStyle();
        void SetStyle(LONG style);
        void SetStyle(LONG and_mask, LONG or_mask);
protected:
        void Default(FEvent &event);
        HWND m_hWnd;                    // The handle to this window

        // pre-dispatch message eating
        virtual Bool PumpEvent(FEvent &) { return False; }
private:
        // FHandler constructor and destructor needs to calladdHandler/removeHandler
        friend FHandler::FHandler( FWnd* );
        friend FHandler::~FHandler();

        // Internal used to register handlers
        void addHandler(FHandler *pHandler);
        void removeHandler(FHandler *pHandler);
        FHandler *m_pFirstHndl; // The first handler in the handler chain
        int m_cnHandle;         // Handler count.

        // Internal event dispatcher, called by the manager and itself
        void Dispatch( FEvent &event );
public://public but only experts should call it
        Bool DispatchNoDefault(FEvent &event);
        Bool CallHandlerChain(FEvent &event, FHandler *after, Bool performDefault);
private:
        FWnd *m_pNext;          // pointer to next window, if any, or NULL. This is part of the FWndMan structure (DBH)
        WNDPROC m_pWndSuperProc;// Super class if any
}; // End of FWnd

// Desktop Window alias
class BIFCLASS FDesktop : public FWnd {
public:
        FDesktop( void );
        ~FDesktop();

        int GetWidth();
        int GetHeight();        
};

class BIFCLASS FWndMan {
public:
        FWndMan( HINSTANCE hInst, HINSTANCE hinstPrev );
        ~FWndMan();

        //public but only dear friends should use them
        void AddWnd( FWnd *pWnd);               // Register in windows table
        Bool RemoveWnd( FWnd *pWnd );           // Remove from table
        FWnd *FindWnd( HWND hWnd );             // Find a wnd class

        Bool AddPumpWindow(FWnd *pwnd);
        void RemovePumpWindow(FWnd *pwnd);

        void PumpEvent( FEvent &event );        // A place to put the MSG from static loop

        HINSTANCE GetInstance( void )
          { return m_hInst ; }
        
        // Only used during creation of a window, stricly internal
        Bool SetCreationWindow( FWnd *pWnd );      // remember a new window
        FWnd *GetCreationWindow( void );           //read-once (destructive read)
        void SubRegWnd( FWnd *pWnd, HWND hWnd );
private:
        HINSTANCE m_hInst;      // local instance handle

        // Concerning the hash table on FWnd -> hwnd relations
        enum { HASHSIZE=67 };           // Size of hwnd hash table (should be a prime)
        FWnd *m_aWndHash[ HASHSIZE ]; // hashtabel itself

        // Generate the hash value. HWNDs seem to be dword-aligned (what a surprise :-)
        static unsigned MakeHash( HWND hWnd )
          { return(((unsigned)hWnd >> 2) % HASHSIZE );}

        // The HWND cache
        FWnd *m_pWndCache;      // Last pWnd used in system
        HWND m_hWndCache;       // hWnd to the last object

        int m_nWndCnt;          // Number of wnd's in the manager
        
        FWnd *m_pInitWnd;       // The wnd to be created

        FWnd **pumpWindowTable;  //dyn. table over windows needing global messages
        int pumpWindows;         //# of         -"-
        Bool CheckPumpWindows(FEvent&);

        FARPROC lpfnBifSubclassProc;
public:
        // Only FCreateDestroyHandler should access this
        const UINT wm_created;
};


FWndMan * BIFFUNCTION GetWndMan();   // Get the global window manager.
FDesktop * BIFFUNCTION GetDesktop(); // Get the desktop window.

#endif // __WNDCORE_H_INCLUDED

