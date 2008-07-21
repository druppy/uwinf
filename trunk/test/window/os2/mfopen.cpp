#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_STDFILEDIALOG
#define BIFINCL_MESSAGEBOX
#define BIFINCL_APPLICATION
#include <bif.h>
//for sprintf only:
#include <stdio.h>

class TestApp : public FApplication {
public:
        int Main(int, char**);
};

int TestApp::Main(int,char**) {
        char filenameBuf[200];
        char *filename[10];
        FStdOpenFileDialog dlg;
        Bool b=dlg.DoModal(0,
                           filenameBuf,200,
                           filename,10
                          );
        MessageLoop();
        WinCancelShutdown(GetWndMan()->GetHMQ(),FALSE); //this is very special
        if(b) {
                for(int i=0; filename[i]; i++) {
                        char msg[256];
                        sprintf(msg,"Filename #%d: '%s'",i,filename[i]);
                        MessageBox(0, msg, "Multifile-open",mb_ok);
                }
        }
        return 0;                          
}


DEFBIFWINDOWMAIN(TestApp);

