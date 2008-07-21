#define BIFINCL_WINDOW
#define BIFINCL_APPWND
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_APPLICATION
#define BIFINCL_DIALOG
#include <bif.h>
#include "defres.h"
#include <string.h>

class TestDialog : public FModalDialog {
public:
        int DoModal(FWnd *powner) {
                return FModalDialog::DoModal(powner,DIALOG_1);
        }
        Bool Command(FCommandEvent &ev) {
                EndDialog(ev.GetItem());
                return True;
        }
};

class TestAppWindow : public FAppWindow,
                      public FCommandHandler
{
public:
        TestAppWindow()
          : FAppWindow(),
            FCommandHandler(this)
          {}

        Bool Create(FWnd *powner) {
                return FFrameWindow::Create(powner,
                                            ID_DEFRES,
                                            fcf_sysmenu|
                                            fcf_titlebar|
                                            fcf_bothbuttons|
                                            fcf_menu|
                                            fcf_sizeborder,
                                            alf_icon|
                                            alf_caption|
                                            alf_menu,
                                            0
                                           );
        }
        
        Bool Command(FCommandEvent &ev) {
                switch(ev.GetItem()) {
                        case MID_TESTDIALOG: {
                                TestDialog dlg;
                                dlg.DoModal(this);
                                return True;
                        }
                        case MID_EXIT: {
                                GetCurrentApp()->TerminateApp();
                                return True;
                        }
                        default:
                                return False;
                }
        }
};


class DefResTestApplication : public FApplication {
        TestAppWindow taw;
        FModule *resModule;
public:
        DefResTestApplication()
          : resModule(0)
          {}
          
        Bool StartUp(int,char**);
        int ShutDown() {
                taw.Destroy();
                FModule::SetDefaultResourceModule(0);
                delete resModule;
                return 0;
        }
};

Bool DefResTestApplication::StartUp(int argc, char **argv) {
        if(argc>1 && stricmp(argv[1],"dansk")==0)
                resModule = new FModule("dansk");
        if(!resModule)
                resModule = new FModule("english");
        if(!resModule)
                return False;
        FModule::SetDefaultResourceModule(resModule);
        if(!taw.Create(0))
                return False;
        taw.Show();
        taw.BringToFront();
        return True;
}

DEFBIFWINDOWMAIN(DefResTestApplication);

