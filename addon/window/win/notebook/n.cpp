#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_APPLICATION
#define BIFINCL_COMMONHANDLERS
#include "notebook.h"


class MyPageDialog : public FDialog {
public:
        MyPageDialog() : FDialog() {}
        Bool Command(FCommandEvent&) {
                return True; //currently ignore cancel
        }
};

class MyNoteBook : public FNoteBook {
        MyPageDialog page1,page2,page3,page4,page5,page6,page7,page8,page9,page10,page11;
public:
        FNoteBook::PageId p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11;
        MyNoteBook() : FNoteBook() {}

        FWnd *LoadPage(FNoteBook::PageId page, FWnd *parent);
};

FWnd *MyNoteBook::LoadPage(FNoteBook::PageId page, FWnd *parent) {
        if(page==p1) {
                page1.Create(parent, "PAGE1");
                return &page1;
        } else if(page==p2) {
                page2.Create(parent, "PAGE2");
                return &page2;
        } else if(page==p3) {
                page3.Create(parent, "PAGE3");
                return &page3;
        } else if(page==p4) {
                page4.Create(parent, "PAGE4");
                return &page4;
        } else if(page==p5) {
                page5.Create(parent, "PAGE5");
                return &page5;
        } else if(page==p6) {
                page6.Create(parent, "PAGE6");
                return &page6;
        } else if(page==p7) {
                page7.Create(parent, "PAGE7");
                return &page7;
        } else if(page==p8) {
                page8.Create(parent, "PAGE8");
                return &page8;
        } else if(page==p9) {
                page9.Create(parent, "PAGE9");
                return &page9;
        } else if(page==p10) {
                page10.Create(parent, "PAGE10");
                return &page10;
        } else if(page==p11) {
                page11.Create(parent, "PAGE11");
                return &page11;
        } else
                return 0;
}


class MyDialog : public FModalDialog, public FSizeHandler {
        MyNoteBook nb;
public:
        MyDialog() : FModalDialog(), FSizeHandler(this) {}
        int DoModal();
        Bool CreateControls();
protected:
        Bool Command(FCommandEvent& ev) {
                EndDialog(ev.GetItem());
                return True;
        }
        Bool SizeChanged(FSizeEvent &ev);
};

int MyDialog::DoModal() {
        return FModalDialog::DoModal(0, "DIALOG_1");
}

Bool MyDialog::SizeChanged(FSizeEvent &ev) {
        nb.SetSize(ev.GetWidth()-20, ev.GetHeight()-60);
        return False;
}

Bool MyDialog::CreateControls() {
        if(!FModalDialog::CreateControls())
                return False;
        if(!nb.Create(this, 100))
                return False;
        nb.p1 = nb.InsertPage(FNoteBook::nopage, "tab1");
        nb.SetPageText(nb.p1,"First page");
        nb.p2 = nb.InsertPage(FNoteBook::nopage, "tab2");
        nb.SetPageText(nb.p2,"Second page 1/2");
        nb.p3 = nb.InsertPage(FNoteBook::nopage, 0     );
        nb.SetPageText(nb.p3,"Third page 2/2");
        nb.p4 = nb.InsertPage(FNoteBook::nopage, "tab3");
        nb.SetPageText(nb.p4,"Fourth page");
        nb.p5 = nb.InsertPage(FNoteBook::nopage, "tab4");
        nb.SetPageText(nb.p5,"Fifth page");
        nb.p6 = nb.InsertPage(FNoteBook::nopage, "long tab 5");
        nb.SetPageText(nb.p6,"Sixth page");
        nb.p7 = nb.InsertPage(FNoteBook::nopage, "tab6");
        nb.SetPageText(nb.p7,"Seventh page");
        nb.p8 = nb.InsertPage(FNoteBook::nopage, "tab7");
        nb.SetPageText(nb.p8,"Eigth page");
        nb.p9 = nb.InsertPage(FNoteBook::nopage, "tab8");
        nb.SetPageText(nb.p9,"Nineth page");
        nb.p10 = nb.InsertPage(FNoteBook::nopage, "tab9");
        nb.SetPageText(nb.p10,"Tenth page");
        nb.p11 = nb.InsertPage(FNoteBook::nopage, "tab10");
        nb.SetPageText(nb.p11,"Eleventh page");
        nb.FlipTo(nb.p1);
        return True;
}


class DialogTestApplication : public FApplication {
        MyDialog dlg;
public:
        //modal test
        int Main(int,char**) {
                return dlg.DoModal();
        }
};

DEFBIFWINDOWMAIN(DialogTestApplication);

