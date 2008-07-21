/***
Filename: dialog.cpp
Description:
  Implementation of FDialog
Host:
  Watcom 10.0a
History:
  ISJ 94-11-?? - 95-01-?? Creation
  ISJ 97-02-01 Add support for specifying resource module
***/
#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_APPLICATION
#include <bif.h>
#include <dbglog.h>

//Dialogs normally don't have FCF_TASKLIST flag, so the window will not be
//in the tasklist. This is a problem if a dialog is the main window of the 
//application. This handler will add a switch entry for such dialogs. 
//(but only dialogs without an owner)

FDialogSwitchEntryHandler::FDialogSwitchEntryHandler(FFrameWindow *pwnd)
  : FHandler(pwnd), createSwitchEntry(False), switchEntryCreated(False)
{ 
        SetDispatch(dispatchFunc_t(&FDialogSwitchEntryHandler::Dispatch)); 
}

Bool FDialogSwitchEntryHandler::Dispatch(FEvent& ev) {
        if(ev.GetID()==GetWndMan()->wm_created) {
                if(!createSwitchEntry) 
                        return False;    //dont create
                if(WinQuerySwitchHandle(GetWnd()->GetHwnd(),0))
                        return False;   //someone else has already created it
                HWND hwndCaption=WinWindowFromID(GetWnd()->GetHwnd(),FID_TITLEBAR);
                if(!hwndCaption) 
                        return False;  //no caption = no title = blank entry in task list = bad
                SWCNTRL swcntrl;
                WinQueryWindowText(hwndCaption,MAXNAMEL,swcntrl.szSwtitle);
                swcntrl.hwnd=GetWnd()->GetHwnd();
                swcntrl.hwndIcon = NULL; //default icon
                swcntrl.hprog=NULL;      //this program
                swcntrl.idProcess=0;     //this process
                swcntrl.idSession=0;     //this session
                swcntrl.uchVisibility=SWL_VISIBLE;
                swcntrl.fbJump=SWL_JUMPABLE;
                swcntrl.bProgType=0;     //default
                HSWITCH hswitch=WinAddSwitchEntry(&swcntrl);
                if(hswitch!=NULLHANDLE) {
                        switchEntryCreated=True;
                        WinSetWindowBits(GetWnd()->GetHwnd(),QWL_STYLE,FS_TASKLIST,FS_TASKLIST);
                } else {
                        NOTE0("FDialogSwitchEntryhandler: could not create switch entry");
                } 
        } else if(ev.GetID()==WM_DESTROY) {
                if(switchEntryCreated) {
                        //remove the switch entry
                        //note: the frame should do this for us
                        HSWITCH hswitch=WinQuerySwitchHandle(GetWnd()->GetHwnd(),0);
                        if(hswitch) {
                                WinRemoveSwitchEntry(hswitch);
                        } else {
                                NOTE0("FDialogSwitchEntryhandler: could not find switch entry");
                        }
                }
        }
        return False;
}



// FDialog----------------------------------------------------------------------
MRESULT EXPENTRY BifInitDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
        FWnd *pwnd=GetWndMan()->GetCreationWindow(0);
        if(pwnd) {
                WinSetWindowPtr(hwnd, QWP_PFNWP, (PVOID)(PFNWP)WinDefDlgProc);
                GetWndMan()->SubRegWnd(pwnd,hwnd);
                FEvent event(hwnd,msg,mp1,mp2);
                if(pwnd->DispatchNoDefault(event))
                        return event.GetResult();
                else
                        return WinDefDlgProc(hwnd,msg,mp1,mp2);
        } else {
                //this should not happen
                return WinDefDlgProc(hwnd,msg,mp1,mp2);
        }
}

FDialog::FDialog()
  : FFrameWindow(),
    FCloseHandler(this),
    FCommandHandler(this),
    FDialogSwitchEntryHandler(this)
{
        TRACE_METHOD1("FDialog::FDialog");
        //nothing
}


/***
Visibility: public:
Parameters:
        pOwner    owner of the dialog
        resID     the resource-ID of the dialog
        module    resource-module
Description:
        Load a dialog from the resources
Return:
        True if the dialog has been successfully loaded and initialized
***/
Bool FDialog::Create(FWnd *pOwner, int resID, FModule *module) {
        TRACE_METHOD1("FDialog::Create");

        if(pOwner==0 || pOwner==GetDesktop())
                createSwitchEntry=True;    //probably a main window

        GetWndMan()->SetCreationWindow(this,0);

        HMODULE hmod;
        if(module)
                hmod = module->GetMODULE();
        else
                hmod = FModule::GetDefaultResourceModule()->GetMODULE();
        HWND hwnd=WinLoadDlg(HWND_DESKTOP,                              //parent
                             pOwner?pOwner->GetHwnd():HWND_DESKTOP,     //owner
                             BifInitDlgProc,                            //dlgproc
                             hmod,                                      //hmodule
                             resID,                                     //dlgtemplate resource ID
                             NULL                                       //pCreateParams
                            );
        if(!hwnd) 
                return False;

        //Get the caption and set it again, so that the caption can be
        //manipulated with FFrame::SetSubtitle() and FFrame::SetCaption()
        HWND hwndTitlebar=WinWindowFromID(hwnd,FID_TITLEBAR);
        if(hwndTitlebar) {
                char caption[128];
                WinQueryWindowText(hwndTitlebar,128,caption);
                if(caption[0]=='-')
                        FFrameWindow::SetSubtitle(caption+1);
                else
                        FFrameWindow::SetCaption(caption);
        }

        //Let the subclass create its control-interfaces
        return CreateControls();
}


/***
Visibility: public:
Parameters:
        pOwner    owner of the dialog (will be while the dialog is used disabled)
        resID     resource-ID of the dialog template
        module    resource-module
Description:
        Do a dialog modal loop. This function first returns when the dialog has
        been closed
Return:
        -1 if something went wrong
        something else if it has been terminated normally (this is usually the
        ID of the pushbutton that closed the dialog)
***/
int FModalDialog::DoModal(FWnd *pOwner, int resID, FModule *module) {
        TRACE_METHOD1("FDialog::DoModal");

        if(!Create(pOwner,resID,module))
                return False;

        int iReturn = WinProcessDlg(GetHwnd());

        if(IsValid()) Destroy();

        if(iReturn==DID_ERROR)
                return -1;
        else
                return iReturn;
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
        WinDismissDlg(GetHwnd(),iReturn);
}

