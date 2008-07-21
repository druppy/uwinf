#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_APPLICATION
#include <bif.h>
#include "dlgtest.idh"

#define MODAL

class MyDlg : public FModalDialog {
public:
	MyDlg()
	  : FModalDialog()
	  {}

	int DoModal(FWnd *pwnd) {
		return FModalDialog::DoModal(pwnd,ID_TESTBW);
        }
        int Create(FWnd *pwnd) {
                return FDialog::Create(pwnd,ID_TESTBW);
        }
protected:
        Bool Command(FCommandEvent& ev) {
#ifdef MODAL
                EndDialog(ev.GetItem());
#else
                GetCurrentApp()->TerminateApp();
#endif
                return True;
        }
};


class DialogTestApplication : public FApplication {
        MyDlg dlg;
public:
#ifdef MODAL
        //modal test
        int Main(int,char**) {
                return dlg.DoModal(0);
        }
#else
        //modeless test
        Bool StartUp(int,char**) {
                if(!dlg.Create(0))
                        return False;
                dlg.Show();
                dlg.BringToFront();
                return True;
        }

        int ShutDown() {
                dlg.Destroy();
                return 0;
        }
#endif
};


DEFBIFWINDOWMAIN(DialogTestApplication);

