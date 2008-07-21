#define BIFINCL_WINDOW
#define BIFINCL_APPWND
#define BIFINCL_APPLICATION
#include "alewin.hpp"

#include "ale.h"


class ALEFrameWindow : public FAppWindow,
                       public FSizeRestrictionFrameHandler
{
public:
        ALEClientWindow client;
        ALEFrameWindow()
          : FAppWindow(),
            FSizeRestrictionFrameHandler(this),
            client(this)
          {}

        Bool Create(FWnd *pOwner);
};

Bool ALEFrameWindow::Create(FWnd *pOwner) {
        if(!FAppWindow::Create(pOwner, 
                               ID_ALE,
                               fcf_sysmenu|
                               fcf_titlebar|
                               fcf_minbutton|
                               fcf_menu|
                               fcf_border,
                               alf_icon|
                               alf_menu|
                               alf_caption|
                               alf_accelerator
                              ))
                return False;
        if(!client.Create(this))
                return False;
        return True;
}


//the application-------------------------------------------------------------
class ALEApplication : public FApplication {
        ALEFrameWindow fw;
public:
        Bool StartUp(int argc, char **argv) {
                if(!fw.Create(0))
                        return False;
                if(!fw.RestorePosition()) 
                        fw.Show();
                fw.BringToFront();
                if(argc==2)
                        fw.client.LoadBitmap(argv[1]);
                return True;
        }
        int ShutDown() {
                fw.SavePosition();
                fw.Destroy();
                return 0;
        }
};

DEFBIFWINDOWMAIN(ALEApplication);

