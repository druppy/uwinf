/*************************************************************************
FILE 
  Dialog.cpp
DESCRIPTION
  Implementation of FDialog
HOST
  BC40, SC61, WC10
HISTORY
  94-11-?? ISJ  Creation
  95-01-?? BL   Changed dlgproc to NULL
  95-02-28 ISJ  Changed to support newstyle of creation
  95-03-10 ISJ  Changed to new PumpEvent style
  95-03-26 ISJ  Corrected problem with multiple modal dialogs
                Worked around non-frame dialog owners
*************************************************************************/
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_DIALOG
#include <bif.h>

#include <dbglog.h>
////////////////////////////////////////////////
// Implementation of FDialogTerminationhandler

// Handles how to terminate a modal dialog.
UINT FDialogTerminationhandler::wm_terminatedialog = 0;

Bool FDialogTerminationhandler::RegTerminateMsg( void ) 
{
        if(wm_terminatedialog) 
                return True;
        
        wm_terminatedialog = RegisterWindowMessage("BIF/Window-terminateloop");
        
        if( wm_terminatedialog )
                return True;
        else
                return False;
}

FDialogTerminationhandler::FDialogTerminationhandler(FDialog *pwnd)
  : FHandler(pwnd)
{
        RegTerminateMsg();      
        SetDispatch(dispatchFunc_t(&FDialogTerminationhandler::Dispatch));
        terminated=False;
        returnCode=-1;
}

Bool FDialogTerminationhandler::Dispatch(FEvent& ev) 
{
        if(ev.GetID() == wm_terminatedialog) {
                //<missing> OS/2 behaviour would be
                //ShowWindow(GetWnd()->GetHwnd(),SW_HIDE);
                terminated = True;
                returnCode = (int)ev.GetWParam();
                return True;    //eaten
        } else if(ev.GetID()==WM_DESTROY) {
                if(!terminated) {
                        terminated = True;
                        returnCode = -1;
                }
                return False;
        }

        return False;
}


///////////////////////////////////////
// Implementation of FDialog 

FDialog::FDialog( void )
  : FFrameWindow(),
    FCloseHandler(this),
    FCommandHandler(this)
{
        TRACE_METHOD1("FDialog::FDialog");
        //nothing
}



/*
 * The stub is not used, we use a NULL dlgproc instead
//stub dialog proc (the real handling is done with subclassing)
static BOOL CALLBACK doNothingDlgProc(HWND, UINT message, WPARAM, LPARAM) 
{
        if(message == WM_INITDIALOG)    
                return TRUE;
        else
                return FALSE;
}
*/

BOOL CALLBACK BifDlgInitProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
        FWnd *pWnd = GetWndMan()->GetCreationWindow();
        
        if( pWnd ) {
                //Replace dlgproc with NULL
                //(NULL dlgprocs is partly undocumented)
                ::SetWindowLong( hWnd, DWL_DLGPROC, (LONG)NULL );
                
                //Subclass and register the window
                GetWndMan()->SubRegWnd( pWnd, hWnd );
                
                //Dispatch the event (but not to default because we are already there)
                FEvent event(hWnd,msg,wParam,lParam);
                if(pWnd->DispatchNoDefault(event)) {
                        //Check for special cases (it's very unlikely that any
                        //of these will be the first message to arrive, but
                        //the chance is there)
#if BIFOS_==BIFOS_WIN16_
                        if(msg==WM_CTLCOLOR ||
#else
                        if(msg==WM_CTLCOLORBTN ||
                           msg==WM_CTLCOLORDLG ||
                           msg==WM_CTLCOLOREDIT ||
                           msg==WM_CTLCOLORLISTBOX ||
                           msg==WM_CTLCOLORMSGBOX ||
                           msg==WM_CTLCOLORSCROLLBAR ||
                           msg==WM_CTLCOLORSTATIC ||
#endif
                           msg==WM_COMPAREITEM ||
                           msg==WM_VKEYTOITEM ||
                           msg==WM_CHARTOITEM ||
                           msg==WM_QUERYDRAGICON ||
                           msg==WM_INITDIALOG)
                                return (BOOL)event.GetResult();
                        else
                                return TRUE;
                } else
                        return FALSE;                
        }
        return FALSE;
}

        

/*************************************************************************
FUNCTION
        FWnd *calculateRealDialogOwner(FWnd *pOwner)
PARAMETER
        pOwner  : proposed owner of the dialog.
VISIBILITY
        local to this module
DESCRIPTION
        Find the real owner of the dialog. That is, the first upward parent
        which is not a child window.
RETURN
        The real owner or NULL
*************************************************************************/
static FWnd *calculateRealDialogOwner(FWnd *pOwner)
{
        while(pOwner && pOwner!=GetDesktop()) {
                LONG style=pOwner->GetStyle();
                if((style&WS_CHILD)==0)
                        return pOwner;
                pOwner=pOwner->GetParent();
        }
        return pOwner;
}



Bool FDialog::Create(FWnd *pOwner, int resID, FModule *module)
{
        return Create(pOwner, MAKEINTRESOURCE(resID), module);
}


/*************************************************************************
FUNCTION
        Bool FDialog::Create(FWnd *pOwner, const char *pszName)
PARAMETER
        pOwner  : owner of the dialog.
        pszName : the resource-name of the dialog.
VISIBILITY
        public of FDialog
DESCRIPTION
        Load a dialog from the resources
*************************************************************************/
Bool FDialog::Create(FWnd *pOwner, const char *pszName, FModule *module) 
{
        HWND hwndDlg, hwndOwner;

        TRACE_METHOD1("FDialog::Create");
        if(!module) module=FModule::GetDefaultResourceModule();
        
        pOwner = calculateRealDialogOwner(pOwner);
        hwndOwner = (pOwner) ? pOwner->GetHwnd() : GetDesktop()->GetHwnd();

        FARPROC lpfnBifDlgInitProc=MakeProcInstance((FARPROC)BifDlgInitProc,GetWndMan()->GetInstance());
        GetWndMan()->SetCreationWindow( this );
        hwndDlg = CreateDialog(module->GetHINSTANCE(),
                               pszName, hwndOwner,
                               (DLGPROC)lpfnBifDlgInitProc
                              );
#if BIFOS_!=BIFOS_WIN32_
        FreeProcInstance(lpfnBifDlgInitProc);
#endif
    
        if( hwndDlg ) {
                // Get the caption and set it again, so that the caption can be
                // manipulated with FFrameWindow::SetSubtitle() and
                // FFrameWindow::SetCaption()
                char caption[128];

                GetWindowText(GetHwnd(), caption, 128);
                if(caption[0] == '-')
                        FFrameWindow::SetSubtitle(caption+1);
                else
                        FFrameWindow::SetCaption(caption);

                // Let the subclass create its control-interfaces
                return CreateControls();
        } else {
                GetWndMan()->GetCreationWindow();  // Remove from manager
                return False;
        }
}


Bool FDialog::Close(FEvent& ev)
{
        TRACE_METHOD0("FDialog::Close");

        //Selecting close is the same as pushing a cancel button
#if BIFOS_==BIFOS_WIN16_
        PostEvent(FEvent(GetHwnd(),WM_COMMAND,IDCANCEL,MAKELPARAM(0,0)));
#else
        PostEvent(FEvent(GetHwnd(),WM_COMMAND,MAKEWPARAM(IDCANCEL,BN_CLICKED),(LPARAM)0));
#endif

        ev.SetResult(0);
        return True;
}


Bool FDialog::Command(FCommandEvent& ev)
{
        TRACE_METHOD0("FDialog::Command");
        //Default action on a wm_command is to end the dialog with return value
        //set to the control id
        ::EndDialog(GetHwnd(),ev.GetItem());
        
        return True;
}

Bool FDialog::PumpEvent(FEvent &ev) {
        //support keyboard handling and accelerators
        //control mnemonics have precedence over accelerators
        //Note: We don't do Add/RemovePumpWindow because the frame has already
        //      done it
        if(::IsDialogMessage(GetHwnd(),&ev) ||
           FFrameWindow::PumpEvent(ev))
                return True;
        else
                return False;
}


////////////////////////////////
// FModalDialog implementation

FModalDialog::FModalDialog()
  : FDialog(),
    FDialogTerminationhandler(this)
{
}
    

int FModalDialog::DoModal(FWnd *pOwner, int resID, FModule *module)
{
        TRACE_METHOD1("FDialog::DoModal");
        return DoModal(pOwner, MAKEINTRESOURCE(resID), module);
}

/***
Visibility: public
Parameters
        pOwner  : owner of the dialog (will be while the dialog is used disabled)
        pszName : resource-name of the dialog template
Return value
        -1 if something went wrong
        something else if it has been terminated normally (this is usually the
        ID of the pushbutton that closed the dialog)
Description
        Do a dialog modal loop. This function first returns when the dialog has
        been closed.
        This code is based on the description in the _OS/2_ toolkit - Windows
        does not document how it makes its modal dialog loops.
***/
int FModalDialog::DoModal(FWnd *pOwner, const char *pszName, FModule *module)
{
        TRACE_METHOD1("FDialog::DoModal");

        pOwner = calculateRealDialogOwner(pOwner);
        if( !Create(pOwner, pszName, module) )
                return -1;      //error
                
        Show(); // Show dialog

        if( pOwner && pOwner!=GetDesktop()) {
                // Disable owner (but not if it is the desktop)
                pOwner->Disable();
        }

        FApplication *pApp = GetCurrentApp();
        FEvent ev;
        while(IsValid() && (!terminated) && pApp->GetEvent(ev)) {
                GetWndMan()->PumpEvent(ev);
        }

        if( pOwner ) {
                //enable owner again
                pOwner->Enable();
        }

        if(IsValid())
                Destroy();

        if(ev.GetID()==WM_QUIT) {
                //messageloop terminated by a wm_quit - repost wm_quit
                if(IsWindow(ev.GetHwnd()))
                        PostMessage(ev.hwnd,ev.GetID(),ev.wParam,ev.lParam);
                else
#if BIFOS_==BIFOS_WIN16_
                PostAppMessage(GetCurrentTask(),WM_QUIT,ev.wParam,ev.lParam);
#else
                PostThreadMessage(GetCurrentThreadId(),ev.GetID(),ev.wParam,ev.lParam);
#endif
        }
        
        return returnCode;
} // End of DoModal


/***
Visibility: public
Parameters
  iReturn : return code
Description
  Dismiss a modal dialog. DoModal will return <iReturn>
***/
void FModalDialog::EndDialog(int iReturn)
{
        TRACE_METHOD1("FDialog::EndDialog");
        PostEvent(FEvent(GetHwnd(),wm_terminatedialog,iReturn,0));
}

