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
        
        Bool Create(FWnd *pOwner, int resID);
        Bool Create(FWnd *pOwner, const char *pszName);

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
        int DoModal(FWnd *pOwner, int resID);
        int DoModal(FWnd *pOwner, const char *pszName);

protected:
        void EndDialog(int iReturn=0);
};

#endif

#ifdef BIFINCL_STDDIALOGS
#  define BIFINCL_STDFILEDIALOG
#endif

#ifdef BIFINCL_STDFILEDIALOG
#  include <window\win16\filedlg.h>
#endif
//more standard dialogs to come in the future

