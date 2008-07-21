#ifndef __DIALOG_H_INCLUDED
#define __DIALOG_H_INCLUDED

class BIFCLASS FDialog;
class BIFCLASS FDialogTerminationhandler : public FHandler {
public:
        FDialogTerminationhandler(FDialog *pwnd);
protected:
        Bool Dispatch(FEvent& ev);

        static UINT wm_terminatedialog;
        static Bool RegTerminateMsg();

        Bool terminated;
        int returnCode;
};

class BIFCLASS FDialog : public FFrameWindow,
                         public FCloseHandler,
                         public FCommandHandler
{
public:
        FDialog();
        
        Bool Create(FWnd *pOwner, int resID, FModule *module=0);
        Bool Create(FWnd *pOwner, const char *pszName, FModule *module=0);

        virtual Bool CreateControls() { return True; }

protected:
        Bool Close(FEvent& ev);
        Bool Command(FCommandEvent&);

        Bool PumpEvent(FEvent&);
};

class BIFCLASS FModalDialog : public FDialog,
                              private FDialogTerminationhandler
{
public:
        FModalDialog();
        int DoModal(FWnd *pOwner, int resID, FModule *module=0);
        int DoModal(FWnd *pOwner, const char *pszName, FModule *module=0);

protected:
        void EndDialog(int iReturn=0);
};

#endif

#ifdef BIFINCL_STDDIALOGS
#  define BIFINCL_STDFILEDIALOG
#endif

#ifdef BIFINCL_STDFILEDIALOG
#  include <window\winc\filedlg.h>
#endif
//more standard dialogs to come in the future

