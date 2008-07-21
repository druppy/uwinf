#define INCL_GPI
#define BIFINCL_WINDOW
#define BIFINCL_APPLICATION
#define BIFINCL_APPWND
#define BIFINCL_CONTROLS
#define BIFINCL_LISTBOX
#define BIFINCL_COMBOBOX
#include <bif.h>

LONG colors[16]= {
        CLR_BLACK,
        CLR_DARKBLUE,
        CLR_DARKRED,
        CLR_DARKPINK,
        CLR_DARKGREEN,
        CLR_DARKCYAN,
        CLR_BROWN,
        CLR_PALEGRAY,
        CLR_DARKGRAY,
        CLR_BLUE,
        CLR_RED,
        CLR_PINK,
        CLR_GREEN,
        CLR_CYAN,
        CLR_YELLOW,
        CLR_WHITE
};

class MyComboBox : public FODSelectionComboBox, public FSizeHandler, public FMoveHandler {
public:
        MyComboBox(FOwnerDrawOwner *powner);
        Bool Create(FWnd *pParent, int ID, FRect *rect);
protected:
        Bool OwneeDraw(FDrawItemEvent &ev);
        int GetItemHeight();
};

MyComboBox::MyComboBox(FOwnerDrawOwner *powner)
  : FODSelectionComboBox(powner),
    FSizeHandler(this),
    FMoveHandler(this)
{
}

Bool MyComboBox::Create(FWnd *pParent, int ID, FRect *rect) {
        if(!FODSelectionComboBox::Create(pParent,ID,rect))
                return False;
        for(int i=0; i<16; i++)
                AddItem(colors[i]);
        return True;
}

Bool MyComboBox::OwneeDraw(FDrawItemEvent &ev) {
        POWNERITEM p=ev.GetOwnerItem();
        if(p->fsState)
                WinFillRect(p->hps, &p->rclItem, CLR_BLACK);
        else
                WinFillRect(p->hps, &p->rclItem, CLR_WHITE);
        RECTL r=p->rclItem;
        r.xLeft+=2;
        r.xRight-=2;
        r.yBottom+=2;
        r.yTop-=2;
        WinFillRect(p->hps, &r, p->hItem);
        p->fsState = p->fsStateOld = 0; //we did the selection drawing

        ev.SetResult(MRFROMSHORT(TRUE));
        return True;
}

int MyComboBox::GetItemHeight() {
        return 20;
}


class MyClient : public FClientWindow, 
                 public FOwnerDrawOwner
{
        MyComboBox combo;

        void setListBoxSize();
public:
        MyClient()
          : FClientWindow(),
            FOwnerDrawOwner(this),
            combo(this)
          {}
        Bool Create(FAppWindow *pwnd);
protected:
        Bool SizeChanged(FSizeEvent&) { setListBoxSize(); return False; }
};

void MyClient::setListBoxSize() {
        int w,h;
        GetSize(&w,&h);
        combo.Move(0,0);
        combo.SetSize(w,h);
}

Bool MyClient::Create(FAppWindow *pwnd) {
        if(!FClientWindow::Create(pwnd))
                return False;
        if(!combo.Create(this,100,0))
                return False;
        setListBoxSize();
        return True;
}

class MyFrame : public FAppWindow {
        MyClient client;
public:
        Bool Create(FWnd *pOwner);
};

Bool MyFrame::Create(FWnd *pOwner) {
        if(!FAppWindow::Create(pOwner,
                               0,
                               fcf_sysmenu|fcf_titlebar|fcf_sizeborder,
                               alf_none
                               ))
                return False;
        if(!client.Create(this))
                return False;
        return True;
}


class MyApp : public FApplication {
        MyFrame fw;
public:
        Bool StartUp(int,char**) {
                if(!fw.Create(0))
                        return False;
                fw.Show();
                fw.BringToFront();
                return True;
        }
        int ShutDown() {
                fw.Destroy();
                return 0;
        }
};

DEFBIFWINDOWMAIN(MyApp);

