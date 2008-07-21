#ifndef __DIALOG_H_INCLUDED
#define __DIALOG_H_INCLUDED

class BIFCLASS FDialogSwitchEntryHandler : public FHandler {
protected:
        FDialogSwitchEntryHandler(FFrameWindow *pwnd);
        Bool Dispatch(FEvent&);

        Bool createSwitchEntry;
private:
        Bool switchEntryCreated;
};

class BIFCLASS FDialog : public FFrameWindow,
                         public FCloseHandler,
                         public FCommandHandler,
                         private FDialogSwitchEntryHandler 
{
public:
        FDialog();

        Bool Create(FWnd *pOwner, int resID, FModule *module=0);

        virtual Bool CreateControls() { return True; }
};

class BIFCLASS FModalDialog : public FDialog {
public:
        int DoModal(FWnd *pOwner, int resID, FModule *module=0);
protected:
        void EndDialog(int iReturn=0);
};

#endif

#ifdef BIFINCL_STDDIALOGS
#  define BIFINCL_STDFILEDIALOG
#endif

#ifdef BIFINCL_STDFILEDIALOG
#  include <window\pm\filedlg.h>
#endif
