#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION

#define BIFINCL_CONTROLS
#define BIFINCL_STATICTEXT
#define BIFINCL_GROUPBOX
#define BIFINCL_CHECKBOX
#define BIFINCL_RADIOBUTTON
#define BIFINCL_PUSHBUTTON
#define BIFINCL_SLE
#define BIFINCL_SCROLLBAR
#define BIFINCL_LISTBOX
#define BIFINCL_COMBOBOX
#define BIFINCL_FRAME
#define BIFINCL_RESOURCETEMPLATE
#define BIFINCL_APPWINDOW
#define BIFINCL_DIALOG
#define BIFINCL_MESSAGEBOX
#define WMSINCL_FOCUS
#include <bif.h>

#define DID_TESTDIALOG 500

STRING(100,"Test frame 1");
STRING(200,"Test frame 2");
STRING(300,"Test appwnd 1");
STRING(400,"Test dialog");

DIALOG(DID_TESTDIALOG, "x=10 y=2 width=30 height=22 title=\"Test Dialog\"")
  DLGITEM(10,"x=2 y=2 width=18 height=1 text=\"Hej med dig\"")
  DLGITEM(20,"x=2 y=3 width=18 height=5 text=\"Groupbox\"")
  DLGITEM(30,"x=4 y=4 width=15 height=1 text=\"checkbox @a\" tabstop 3state")
  DLGITEM(31,"x=4 y=5 width=15 height=1 text=\"checkbox @b\" 3state")
  DLGITEM(40,"x=2 y=8 width=18 height=1 text=\"radio @1\" tabstop group")
  DLGITEM(41,"x=2 y=9 width=18 height=1 text=\"radio @2\"")
  DLGITEM(42,"x=2 y=10 width=18 height=1 text=\"radio @3\"")
  DLGITEM(50,"x=2 y=12 width=13 height=2 text=\"@Ok\" tabstop group default")
  DLGITEM(51,"x=2 y=14 width=13 height=2 text=\"@Cancel\"")
  DLGITEM(60,"x=2 y=17 width=13 height=1 text=\"hej med dig\" tabstop group autohscroll")
  DLGITEM(70,"x=2 y=19 width=13 height=1 horizontal")
  DLGITEM(80,"x=19 y=16 width=10 height=7 tabstop group sort extended")
  DLGITEM(90,"x=19 y=16 width=10 height=7 tabstop group")
ENDDIALOG

class FTestDialog : public FModalDialog {
        FStaticText st;
        FGroupBox gb;
        FCheckBox cb1,
                  cb2;
        FRadioButton rb1,
                     rb2,
                     rb3;
        FPushButton pb1,pb2;
        FSLE sle;
        FScrollBar sb;
        FMultipleSelectionTextListBox lb;
        FTextSelectionComboBox combo;
public:
        Bool CreateControls();
};

Bool FTestDialog::CreateControls() {
        FRect r;

        st.Create(this,10);
        gb.Create(this,20);

        cb1.Create(this,30);
        cb2.Create(this,31);

        rb1.Create(this,40);
        rb2.Create(this,41);
        rb3.Create(this,42);

        pb1.Create(this,50);
        pb2.Create(this,51);

        sle.Create(this,60);

        sb.Create(this,70);
/*
        lb.Create(this, 80);
        lb.AddItem("aitem 1");
        lb.AddItem("bitem 2");
        lb.AddItem("item 3");
        lb.AddItem("item 4");
        lb.AddItem("iitem 5");
        lb.AddItem("item 6");
        lb.AddItem("item 7");
        lb.AddItem("cgitem 8");
        lb.AddItem("item 9");
        lb.AddItem("item 10");
        lb.AddItem("citem 11");
        lb.AddItem("cacbitem 12");
        lb.AddItem("item 13");
        lb.AddItem("item 14");
        lb.AddItem("item 15");
        lb.AddItem("item 16");
*/
        combo.Create(this,90);
        combo.AddItem("item 1");
        combo.AddItem("item 2");
        combo.AddItem("item 3");

        return True;
}

class FTestApplication : public FMTApplication {
        FFrameWindow fw1,fw2;
        FAppWindow aw;
        FPushButton client;
        FTestDialog dlg;
public:
        Bool StartUp(int,char**);
//        int Main(int,char**);
        int ShutDown() {return 0;};
};

#include "..\..\..\source\window\wms\switchls.h"

Bool FTestApplication::StartUp(int,char**) {
   GetSwitchList();
        FRect r;
        /*
        r.Set(10,2,40,30);
        dlg.DoModal(0,
                   400,
                   FFrameWindow::fcf_sysmenu|
                   FFrameWindow::fcf_titlebar|
                   FFrameWindow::fcf_dialogborder,
                   FFrameWindow::alf_caption,
                   &r
                  );
        */

        dlg.DoModal(0, DID_TESTDIALOG);

        MessageBox(0, "Hello world! And a very, very, very, very long text\nAnd yet another line", "No error", mb_okcancel);

        r.Set(30,35,70,45);
        fw1.Create(0,
                  100,
                  FFrameWindow::fcf_sysmenu|
                  FFrameWindow::fcf_titlebar|
                  FFrameWindow::fcf_minbutton|
                  FFrameWindow::fcf_maxbutton|
                  FFrameWindow::fcf_hidebutton|
                  FFrameWindow::fcf_horzscroll|
                  FFrameWindow::fcf_vertscroll|
                  FFrameWindow::fcf_sizeborder,
                  FFrameWindow::alf_caption,
                  &r
                 );
        fw1.Show();

        r.Set(20,15,50,25);
        fw2.Create(0,
                  200,
                  FFrameWindow::fcf_sysmenu|
                  FFrameWindow::fcf_titlebar|
                  FFrameWindow::fcf_minbutton|
                  FFrameWindow::fcf_maxbutton|
                  FFrameWindow::fcf_hidebutton|
                  FFrameWindow::fcf_horzscroll|
                  FFrameWindow::fcf_vertscroll|
                  FFrameWindow::fcf_sizeborder,
                  FFrameWindow::alf_caption,
                  &r
                 );
        fw2.SetOwner(&fw1);
        fw2.Show();

        r.Set(10,10,40,20);
        aw.Create(0,
                  300,
                  FFrameWindow::fcf_sysmenu|
                  FFrameWindow::fcf_titlebar|
                  FFrameWindow::fcf_minbutton|
                  FFrameWindow::fcf_maxbutton|
                  FFrameWindow::fcf_hidebutton|
                  FFrameWindow::fcf_horzscroll|
                  FFrameWindow::fcf_vertscroll|
                  FFrameWindow::fcf_sizeborder,
                  FFrameWindow::alf_caption,
                  &r
                 );
        client.Create(&aw,FID_CLIENT,&r,"",False);
        aw.SetClient(&client);
        aw.Show();
        aw.BringToFront();
        
        return True;
}
/* speed test:
int FTestApplication::Main(int argc,char **argv) {
  StartUp(argc,argv);
  WmsHMQ hmq=GetWndMan()->GetHMQ(0);
  WmsQMSG qmsg;
  for(int i=0; i<1000; i++) {
    while(WmsPeekMsg(hmq,&qmsg,0,0,0,PM_REMOVE)) {
      WmsDispatchMsg(&qmsg);
    }
    WmsInvalidateRect(GetDesktop()->GetHwnd(),0,True);
  }
  ShutDown();
  return 0;
}
*/

DEFBIFMIXMAIN(FTestApplication,FWMainWindowThread);

