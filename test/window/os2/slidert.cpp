#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_APPWND
#define BIFINCL_CONTROLS
#define BIFINCL_SLIDER
#include <bif.h>

class SliderTestFrame : public FAppWindow {
        FSlider sl;
public:
        SliderTestFrame() : FAppWindow() {}

        Bool Create(FWnd *pOwner);
};

Bool SliderTestFrame::Create(FWnd *pOwner) {
        if(!FAppWindow::Create(pOwner,999,fcf_sysmenu|fcf_titlebar|fcf_sizeborder,alf_none))
                return False;
        if(!sl.Create(this,0,
                      0,sl.sls_horizontal|sl.sls_center|sl.sls_buttonsright,
                      0,100,10))
                return False;
        SetClient(&sl);
        Show();
        BringToFront();

        for(int pos=0; pos<=100; pos+=10)
                sl.SetTickSize(pos,pos/10+1);
        sl.SetTickText(0,"0");
        sl.SetTickText(50,"50");
        sl.SetTickText(100,"100");
        if(sl.AddDetent(37)==0)
                return False;

        return True;
}

class SliderTestApp : public FApplication {
        SliderTestFrame f;
public:
        Bool StartUp(int,char**) {
                if(!f.Create(GetDesktop()))
                        return False;
                f.Show();
                f.BringToFront();
                return True;
        }
        int ShutDown() {
                f.Destroy();
                return 0;
        }
};

DEFBIFWINDOWMAIN(SliderTestApp);

