/*************************************************************************
FILE
        WndCore.h
AUTHOR
        Bo Lorentsen & Ivan J›rgensen
VERSION
        ? (non yet)
DESCRIPTION
        Class prototype of all window related ellements in the BIFoundation
        witch include the tree main groupes :

        Event :
                Who is the container off all system events/messages and return
            values.

        Windows :
                Identify all windows in the application, and is the servever
                of all messages sent to the window. If a message is sent to
                the system it will first "hit" the manager, who will find the
                right window to sent the event/message to, and the window will
                then ask all its handles if they wandt the event/message, and
                if not the window are responceble fore calling the Default
                member funktion, who call the system related default handler.

        Handles :
                Every event/message for a given window are sent to one or more
                handles who is respoceble for taking the importen aktion on a
                given system responce, and if there is some system dependen
                aktions it have to hide thise.
*************************************************************************/
#ifndef __WNDCORE_H_INCLUDED
#define __WNDCORE_H_INCLUDED

#define WMSINCL_MSGQUEUE
#define WMSINCL_BW
#include <wms.hpp>
//-----------------------------------------------------------------------------
//      Definition of the FEvent class
class BIFCLASS FEvent : public WmsQMSG
{
public:
        FEvent( void ) {m_nSysReturn = 0;}
        FEvent( WmsHWND hWnd, WmsMSG msg, WmsMPARAM MP1, WmsMPARAM MP2 )
          { Set(hWnd, msg, MP1, MP2); m_nSysReturn=0; }
        FEvent(const WmsQMSG &qmsg) { *((WmsQMSG*)(this)) = qmsg; }

        //message (id) access
        WmsMSG GetID() const
          { return msg; }
        void SetID( WmsMSG nId )
          { msg = nId; }

        //MP1 access
        WmsMPARAM GetMP1() const
          { return mp1; }
        void SetMP1(WmsMPARAM param )
          { mp1 = param; }

        //MP2 access
        WmsMPARAM GetMP2() const
          { return mp2; }
        void SetMP2(WmsMPARAM param )
          { mp2 = param; }


        //hwnd access
        WmsHWND GetHwnd() const
          { return hwnd; }
        void SetHwnd(WmsHWND hw)
          { hwnd=hw; }

        void Set(WmsHWND hWnd, WmsMSG nId, WmsMPARAM MP1, WmsMPARAM MP2)
          {hwnd = hWnd; msg = nId; mp1 = MP1; mp2 = MP2; }

        //result access
        void SetResult( WmsMRESULT lRet )
          { m_nSysReturn = lRet; }
        WmsMRESULT GetResult() const
          { return m_nSysReturn; }

        WmsMRESULT m_nSysReturn;   // Return value to return to the system
}; // End of FEvent



//-----------------------------------------------------------------------------
//      Definition of the FHandler class
//get property class
#include <window/wms/wndprop.h>
class FWnd;                     // Forward declaration of FWnd
class BIFCLASS FHandler {
        friend class FWnd;
public:
        FHandler( FWnd *pWnd );
        virtual ~FHandler();
protected:
        typedef Bool (FHandler::*dispatchFunc_t)(FEvent&);
        void SetDispatch(dispatchFunc_t func)
          { m_pDispatch=func; }
        FWnd *GetWnd() const { return m_pWnd; }
private:
        dispatchFunc_t m_pDispatch;     // The local dispatch funktion
        Bool DoDispatch( FEvent &event );
        FHandler *m_pNext;              // Pointer to the next handler, if any. Part
                                        // of a FWnd struture
        FWnd *m_pWnd;
}; // End of FHandler


//-----------------------------------------------------------------------------
//      Definition of the FWnd class
class BIFCLASS FWnd {
        friend class FWndMan;
        // Common WindowProc for all windows
        friend WmsMRESULT BifDispatchFunc(const WmsQMSG *qmsg);
public:
        FWnd( void );
        virtual ~FWnd();

        // Simple version of Create
        Bool Create(const char *pszName, uint32 flStyle, FRect *rect,
                    WmsHWND hwndParent, uint16 id)
          { return Create(hwndParent, pszName, flStyle, rect, hwndParent, HWND_TOP, id); }

        virtual Bool Destroy(void);     // Destroy the system window in any means.

protected:
        // Full scale Create
        Bool Create( WmsHWND hwndParent, const char *pszName,
                     uint32 flStyle, FRect *rect, WmsHWND hwndOwner, WmsHWND hwndInsertBehind,
                     uint16 id
                   );

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

        WmsHWND GetHwnd() const
          { return m_hWnd; }

        Bool IsValid() const
          { return (Bool)(m_hWnd!=0); }

        // zorder & siblings
        enum zorder {
                zorder_first,
                zorder_last,
                zorder_before,
                zorder_after
        };

        Bool Zorder(zorder z, FWnd *pWnd = 0 );
        FWnd *GetFirstSibling();
        FWnd *GetNextSibling();

        // Event handling
        Bool PostEvent(const FEvent &event);    // Post a FEvent
        Bool SendEvent(FEvent &event);          // Send a FEvent

        //Miscellaneous
        uint32 GetStyle();
        void SetStyle(uint32 style);
        void SetStyle(uint32 and_mask, uint32 or_mask);
protected:
        void Default(FEvent &event);
        WmsHWND m_hWnd;                         // The handle to this window
        WindowProperty prop;                    // window properties
private:
        // FHandler constructor and destructor needs to call addHandler/removeHandler
        friend FHandler::FHandler( FWnd* );
        friend FHandler::~FHandler();
        // Internal used to register handlers
        void addHandler(FHandler *pHandler);
        void removeHandler(FHandler *pHandler);
        FHandler *m_pFirstHndl; // The first handler in the handler chain
        int m_cnHandle;         // Handler count.

        // Internal event dispatcher, called by the manager and itself
        void Dispatch( FEvent &event );
public: //only experts should call this member
        Bool DispatchNoDefault(FEvent &event);
        Bool CallHandlerChain(FEvent &event, FHandler *after, Bool performDefault);
private:
        FWnd *m_pNext;          // pointer to next window, if any, or NULL. This is part of the FWndMan structure (DBH)
}; // End of FWnd





//desktop
class BIFCLASS FDesktop : public FWnd {
public:
        int GetWidth();
        int GetHeight();        
};

FDesktop * BIFFUNCTION GetDesktop();




class BIFCLASS FWndMan {
        unsigned long initializerThread;             //The thread that called the constructor
public:
        FWndMan();
        ~FWndMan();

        //public but only dear friends should use them
        void AddWnd( FWnd *pWnd);               // Register in windows table
        Bool RemoveWnd( FWnd *pWnd );           // Remove from table
        FWnd *FindWnd( WmsHWND hWnd );          // Find a wnd class

        void PumpEvent( FEvent &event );        // A place to put the MSG from static loop

        //hab&hmq registration
        Bool RegisterThread(unsigned long tid, WmsHAB hab, WmsHMQ hmq);
        Bool DeregisterThread(unsigned long tid);
        WmsHAB GetHAB(unsigned long tid=0);
        WmsHMQ GetHMQ(unsigned long tid=0);
        WMSDISPATCHFUNC GetDP(unsigned long tid=0);

        void SubRegWnd(FWnd *pwnd, WmsHWND hWnd);
private:
        // Concerning the hash table on FWnd -> hwnd relations
        enum { HASHSIZE=67 };           // Size of hwnd hash table (should be a prime)
        FWnd *m_aWndHash[HASHSIZE];     // hashtabel itself

        // Generate the hash value, HWNDs are sequential
        int MakeHash( WmsHWND hWnd ) { return ((int)hWnd) % HASHSIZE;}

        // The WmsHWND cache
        FWnd *m_pWndCache;              // Last pWnd used in system
        WmsHWND m_hWndCache;            // hWnd to the last object

        unsigned m_nWndCnt;             // Number of wnd's in the manager

        struct ThreadRegistration {
                ThreadRegistration *next;
                unsigned long tid;
                WmsHAB hab;
                WmsHMQ hmq;
                FWnd *creationWindow;
                WMSDISPATCHFUNC prevDispatchFunc;
        } *m_threadregs;                //single-linked list without header

public:
        //only FCreateDestroyHandler should access this
        const WmsMSG wm_created;
};

FWndMan * BIFFUNCTION GetWndMan();

#endif // __WNDCORE_H_INCLUDED

