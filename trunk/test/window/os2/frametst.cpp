#define BIFINCL_WINDOW
#define BIFINCL_APPWND
#define BIFINCL_APPLICATION
#define BIFINCL_MESSAGEBOX
#include <bif.h>
#include "frametst.idh"

class FrameTestApplication : public FApplication {
        FFrameWindow ffw;
public:
        Bool StartUp(int,char**);
        int ShutDown() {
                ffw.Destroy();
                return 0;
        }
};

Bool FrameTestApplication::StartUp(int,char**) {
        if(!ffw.Create(0,
                       ID_TESTBW,
                       FFrameWindow::fcf_sysmenu|
                       FFrameWindow::fcf_titlebar|
                       FFrameWindow::fcf_minmaxbuttons|
                       FFrameWindow::fcf_menu|
                       FFrameWindow::fcf_sizeborder|
                       FFrameWindow::fcf_horzscroll|
                       FFrameWindow::fcf_vertscroll,
                       FFrameWindow::alf_icon|
                       FFrameWindow::alf_caption|
                       FFrameWindow::alf_menu,
                       0                            //let Windows decide position
                      ))
                return False;
        ffw.Show();
        ffw.BringToFront();
        return True;
}

DEFBIFWINDOWMAIN(FrameTestApplication);

