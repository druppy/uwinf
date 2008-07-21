#define BIFINCL_APPLICATION
#include "editdlg.hpp"

class MenuEditApplication : public FApplication {
        MenuEditDialog dlg;
public:
        Bool StartUp(int argc, char **argv) {
                if(!dlg.Create(0))
                        return False;
                dlg.Show();
                dlg.BringToFront();
                if(argc==2)
                        dlg.LoadMenu(argv[1]);
                return True;
        }

        int ShutDown() {
                dlg.Destroy();
                return 0;
        }
};

DEFBIFWINDOWMAIN(MenuEditApplication);

