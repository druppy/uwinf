#ifndef __DIALOG_H_INCLUDED
#define __DIALOG_H_INCLUDED

#define DID_OK          1
#define DID_CANCEL      2
#define DID_ERROR       (-1)

class BIFCLASS FDialogKeyboardHandler : public FKeyboardHandler {
public:
        FDialogKeyboardHandler(FWnd *pwnd)
          : FKeyboardHandler(pwnd)
          { }
        virtual Bool CharInput(FKeyboardEvent&);
        virtual Bool KeyPressed(FKeyboardEvent&);
protected:
        virtual Bool nextControl();
        virtual Bool prevControl();
        virtual Bool nextTab();
        virtual Bool prevTab();
        virtual Bool escape();
        virtual Bool enter();
        virtual Bool findMnemonic(char c);
};

class BIFCLASS FDialogFocusHandler : public FHandler {
        WmsHWND hwndSavedFocus;
        static WmsMSG wm_forwardfocus;
protected:
        FDialogFocusHandler(FWnd *pwnd)
          : FHandler(pwnd),
            hwndSavedFocus(0)
          { SetDispatch(dispatchFunc_t(FDialogFocusHandler::Dispatch)); }
        Bool Dispatch(FEvent &ev);

        virtual void focusChanged(WmsHWND hwndNewFocus) =0;
};

class BIFCLASS FDialogDismissHandler : public FHandler {
protected:
        FDialogDismissHandler(FWnd *pwnd)
          : FHandler(pwnd)
          { SetDispatch(dispatchFunc_t(FDialogDismissHandler::Dispatch)); }
        Bool Dispatch(FEvent &ev);
        virtual void Dismiss(unsigned code) =0;
};

class BIFCLASS FDialogResourceHandler : public FHandler {
        const DialogResource *dlgres;
public:
        FDialogResourceHandler(FWnd *pwnd)
          : FHandler(pwnd), dlgres(0)
        { SetDispatch(dispatchFunc_t(FDialogResourceHandler::Dispatch)); }
protected:
        Bool Dispatch(FEvent &ev);
        void SetResourceTemplate(const DialogResource *p) {
                dlgres = p;
        }
};

class BIFCLASS FDialog : public FFrameWindow,
                         public FCloseHandler,
                         public FCommandHandler,
                         public FDialogFocusHandler,
                         public FDialogDismissHandler,
                         private FDialogResourceHandler
                         
{
        FDialogKeyboardHandler keyboardhandler;
        WmsHWND hwndDefaultButton;
        WmsHWND findTheDefaultButton();
protected:
        Bool dismissed;
        unsigned result;
public:
        FDialog();

        Bool Create(FWnd *pOwner, int resID);
        Bool Create(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect = 0 );
        

        virtual Bool CreateControls() { return True; }

protected:
        Bool Close(FEvent &);
        Bool Command(FCommandEvent &ev);

        void focusChanged(WmsHWND hwndNewFocus);

        void Dismiss(unsigned code);
};


class BIFCLASS FModalDialog : public FDialog {
public:
        int DoModal(FWnd *pOwner, int resID);
        int DoModal(FWnd *pParent, int Id, long fcf, long alf, FRect *pRect);
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