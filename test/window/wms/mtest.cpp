#define BIFINCL_THREAD
#define BIFINCL_MAINTHREAD
#define BIFINCL_WINDOWTHREAD
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION

#define BIFINCL_CONTROLS
#define BIFINCL_PUSHBUTTON
#define BIFINCL_FRAME
#define BIFINCL_RESOURCETEMPLATE
#define BIFINCL_APPWINDOW
#define BIFINCL_MESSAGEBOX
#define BIFINCL_COMMONHANDLERS

#define BIFINCL_ADVANCEDHANDLERS
#define BIFINCL_SIZERESTRICTIONHANDLERS
#include <bif.h>
#include "..\..\..\source\window\wms\menuwin.h"

STRING(100,"Test frame 1");
STRING(200,"Test frame 2");
STRING(300,"Test appwnd 1");

MENU(500)
  MENUITEM(501,"text=\"@First item\tF1\"")
  MENUITEM(502,"text=\"@Second item\" checked")
  MENUITEM(503,"separator")
  SUBMENU(600,"text=\"submenu\"")
ENDMENU
MENU(600)
  MENUITEM(601,"text=\"Item @1\"")
  MENUITEM(602,"text=\"Item @2\" disabled")
  MENUITEM(603,"text=\"Item @3\"")
ENDMENU

MENU(300)
  SUBMENU(1100,"text=\"@File\"")
  SUBMENU(1200,"text=\"@Edit\"")
  SUBMENU(1400,"text=\"@Search\"")
  SUBMENU(1500,"text=\"@Options\"")
  SUBMENU(1300,"text=\"@Help\"")
ENDMENU
MENU(1100)
  MENUITEM(1101,"text=\"@New\tShift+F3\"")
  MENUITEM(1102,"text=\"@Open...\tF3\"")
  MENUITEM(-1,"separator")
  MENUITEM(1103,"text=\"@Save\tF2\" disabled")
  MENUITEM(1104,"text=\"Save @as...\tShift+F2\" disabled")
  MENUITEM(-1,"separator")
  MENUITEM(1105,"text=\"E@xit\tAlt+X\"")
ENDMENU
MENU(1200)
  MENUITEM(1201,"text=\"@Undo\tAlt+BkSpc\"")
  MENUITEM(1202,"text=\"@Redo\tShift+Alt+BkSpc\"")
  MENUITEM(-1,"separator")
  MENUITEM(1203,"text=\"Cu@t\tShift+Del\"")
  MENUITEM(1204,"text=\"@Copy\tCtrl+Ins\"")
  MENUITEM(1205,"text=\"@Paste\tShift+Ins\" disabled")
  MENUITEM(1205,"text=\"Clea@r\tCtrl+Del\"")
ENDMENU
MENU(1300)
  MENUITEM(1301,"text=\"General help\" help")
  MENUITEM(1302,"text=\"Index\" help")
  MENUITEM(1303,"separator")
  MENUITEM(1304,"text=\"Product @information\" help")
ENDMENU
MENU(1400)
  MENUITEM(1401,"text=\"@Find...\"")
  MENUITEM(1402,"text=\"@Replace...\"")
  MENUITEM(1403,"text=\"@Search again\tCtrl+L\"")
  MENUITEM(-1,"separator")
  MENUITEM(1405,"text=\"@Go to line number...\"")
  MENUITEM(1406,"text=\"@Previous error\" disabled")
  MENUITEM(1407,"text=\"@Next error\" disabled")
  MENUITEM(1408,"text=\"@Locate function\" disabled")
ENDMENU
MENU(1500)
  MENUITEM(1501,"text=\"@Application...\"")
  SUBMENU(1600,"text=\"@Compiler\"")
  MENUITEM(1502,"text=\"@Transfer...\"")
  MENUITEM(1503,"text=\"@Make....\"")
  SUBMENU(1700,"text=\"@Linker\"")
  MENUITEM(1504,"text=\"L@ibrarian...\"")
  MENUITEM(1505,"text=\"De@bugger...\"")
  MENUITEM(1506,"text=\"@Directories...\"")
  MENUITEM(-1,"separator")
  SUBMENU(1800,"text=\"@Environment\"")
  MENUITEM(-2,"separator")
  MENUITEM(1508,"text=\"@Save...\"")
ENDMENU
MENU(1600)
  MENUITEM(1601,"text=\"@Code generation...\"")
  MENUITEM(1602,"text=\"@Advanced code generation\"")
  MENUITEM(1603,"text=\"@Entry/exit code\"")
  MENUITEM(1604,"text=\"C@++ options...\"")
  MENUITEM(1605,"text=\"Ad@vanced C++ options...\"")
  MENUITEM(1606,"text=\"@Optimizations...\"")
  MENUITEM(1607,"text=\"@Source...\"")
  SUBMENU(1900,"text=\"@Messages\"")
  MENUITEM(1609,"text=\"@Names...\"")
ENDMENU
MENU(1700)
  MENUITEM(1701,"text=\"@Settings..\"")
  MENUITEM(1702,"text=\"@Libraries\"")
ENDMENU
MENU(1800)
  MENUITEM(1800,"text=\"@Preferences...\"")
  MENUITEM(1800,"text=\"@Editor...\"")
  MENUITEM(1800,"text=\"@Mouse...\"")
  MENUITEM(1800,"text=\"@Desktop...\"")
  MENUITEM(1800,"text=\"@Startup...\"")
  MENUITEM(1800,"text=\"@Colors...\"")
ENDMENU
MENU(1900)
  MENUITEM(1901,"text=\"@Display...\"")
  MENUITEM(-1,"separator")
  MENUITEM(1903,"text=\"@Portability...\"")
  MENUITEM(1904,"text=\"@Ansi violations...\"")
  MENUITEM(1905,"text=\"@C++ warnings...\"")
  MENUITEM(1906,"text=\"@Frequent errors...\"")
  MENUITEM(1907,"text=\"@Less frequent errors...\"")
ENDMENU

class MyAppWindow : public FAppWindow,
                    public FCommandHandler
{
        FPushButton client;
        FMenu *m;
public:
        MyAppWindow()
          : FAppWindow(),
            FCommandHandler(this)
          {}
        Bool Create();
        Bool Command(FCommandEvent &ev);
};

Bool MyAppWindow::Create() {
        FRect r(10,10,40,20);
        FAppWindow::Create(0,
                  300,
                  fcf_sysmenu|
                  fcf_menu|
                  fcf_titlebar|
                  fcf_minbutton|
                  fcf_maxbutton|
                  fcf_hidebutton|
                  fcf_horzscroll|
                  fcf_vertscroll|
                  fcf_sizeborder,
                  alf_caption,
                  &r
                 );
        client.Create(this,FID_CLIENT,&r,"",False);
        SetClient(&client);
        
        m = new FMenu(this,500);
        return True;
}

Bool MyAppWindow::Command(FCommandEvent &) {
        m->Popup(0,this);
        return True;
}

class FTestApplication : public FMTApplication {
        FFrameWindow fw1,fw2;
        MyAppWindow aw;
public:
        Bool StartUp(int,char**);
//        int Main(int,char**);
        int ShutDown() {return 0;};
};

Bool FTestApplication::StartUp(int,char**) {
        FRect r;

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

        aw.Create();
        aw.Show();
        aw.BringToFront();

        return True;
}

DEFBIFMIXMAIN(FTestApplication,FWMainWindowThread);

