#define BIFINCL_WINDOW
#define BIFINCL_APPWND
#define BIFINCL_COMMONHANDLERS
#define BIFINCL_MOUSEHANDLERS
#define BIFINCL_APPLICATION
#include <bif.h>
#include <string.h>

class MyClient : public FClientWindow, public FTranslatedMouseHandler {
public:
        MyClient() : FTranslatedMouseHandler(this) {}
protected:
        virtual Bool MouseDragStart(FMouseEvent&);
        virtual Bool MouseDragEnd(FMouseEvent&);
        virtual Bool MouseSingleSelect(FMouseEvent&);
        virtual Bool MouseOpen(FMouseEvent&);
        virtual Bool MouseContextMenu(FMouseEvent&);
        virtual Bool MouseTextEdit(FMouseEvent&);
        virtual Bool MouseSelectStart(FMouseEvent&);
        virtual Bool MouseSelectEnd(FMouseEvent&);

        void ShowEvent(const char *s);
};

Bool MyClient::MouseDragStart(FMouseEvent&) {
        ShowEvent("MouseDragStart: Start dragging object underneath mouse or all selected objects");
        return True;
}
Bool MyClient::MouseDragEnd(FMouseEvent&) {
        ShowEvent("MouseDragEnd: Drop dragged object at mouse");
        return True;
}
Bool MyClient::MouseSingleSelect(FMouseEvent&) {
        ShowEvent("MouseSingleSelect: Select object underneath mouse according to ctrl and shift state");
        return True;
}
Bool MyClient::MouseOpen(FMouseEvent&) {
        ShowEvent("MouseOpen: Open object underneath mouse or do 'default' action");
        return True;
}
Bool MyClient::MouseContextMenu(FMouseEvent&) {
        ShowEvent("MouseContextMenu: Put up a context menu for the object underneath the mouse");
        return True;
}
Bool MyClient::MouseTextEdit(FMouseEvent&) {
        ShowEvent("MouseEditText: Let the user edit the name/title of the object underneath the mouse");
        return True;
}
Bool MyClient::MouseSelectStart(FMouseEvent&) {
        ShowEvent("MouseSelectStart: Begin marking all object the mouse moves over");
        return True;
}
Bool MyClient::MouseSelectEnd(FMouseEvent&) {
        ShowEvent("MouseSelectEnd: Stop marking objects");
        return True;
}


void MyClient::ShowEvent(const char *s) {
        HPS hps=WinGetPS(GetHwnd());
        int w,h;
        GetSize(&w,&h);
        FRect r(0,0,w,h);
        WinDrawText(hps,-1,(PCH)s,&r,0,0,DT_TEXTATTRS|DT_ERASERECT);
        WinReleasePS(hps);
}

class MouseTestApplication : public FApplication {
        FAppWindow faw;
        MyClient client;
public:
        Bool StartUp(int,char**);
        int ShutDown() {
                faw.Destroy();
                return 0;
        }
};

Bool MouseTestApplication::StartUp(int,char**) {
        if(!faw.Create(0,                               //no parent (=desktop)
                      0,                                //no ID
                      FFrameWindow::fcf_sysmenu|
                      FFrameWindow::fcf_titlebar|
                      FFrameWindow::fcf_minmaxbuttons|
                      FFrameWindow::fcf_sizeborder,
                      0,                                //don't load resources
                      0                                 //let Windows decide position
                  ))
                return False;
        if(!client.Create(&faw))
                return False;
        faw.SetCaption("BIF");
        faw.SetSubtitle("Mouse test program");
        faw.Show();
        faw.BringToFront();
        return True;
}


DEFBIFWINDOWMAIN(MouseTestApplication);

