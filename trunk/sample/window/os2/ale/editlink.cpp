#include "editlink.hpp"
#include "editlink.h"
#include <stdlib.h>

void FEditLinkDialog::readControls() {
        if(c_heading.IsChecked())
                link.reftype=Link::heading;
        if(c_footnote.IsChecked())
                link.reftype=Link::footnote;
        if(c_launch.IsChecked())
                link.reftype=Link::launch;
        if(c_inform.IsChecked())
                link.reftype=Link::inform;

        char buf[40];
        c_x.GetText(buf,40);
        link.x=atoi(buf);
        c_y.GetText(buf,40);
        link.y=atoi(buf);
        c_cx.GetText(buf,40);
        link.cx=atoi(buf);
        c_cy.GetText(buf,40);
        link.cy=atoi(buf);

        c_res.GetText(link.res,40);
        c_refid.GetText(link.refid,40);
        c_database.GetText(link.database,40);
        c_object.GetText(link.object,40);
        c_data.GetText(link.data,40);
}


FEditLinkDialog::FEditLinkDialog(const Link &l, LinkChangeNotifier *n)
  : FModalDialog()
{
        link=l;
        notifier=n;
}

int FEditLinkDialog::DoModal(FWnd *pOwner) {
        return FModalDialog::DoModal(pOwner,IDD_EDITLINK);
}


Bool FEditLinkDialog::CreateControls() {
        int b=1;
        b=b&&c_heading.Create(this,DID_REFTYPE_HD);
        b=b&&c_footnote.Create(this,DID_REFTYPE_FN);
        b=b&&c_launch.Create(this,DID_REFTYPE_LAUNCH);
        b=b&&c_inform.Create(this,DID_REFTYPE_INFORM);
        b=b&&c_x.Create(this,DID_X);
        b=b&&c_y.Create(this,DID_Y);
        b=b&&c_cx.Create(this,DID_CX);
        b=b&&c_cy.Create(this,DID_CY);
        b=b&&c_res.Create(this,DID_RES);
        b=b&&c_refid.Create(this,DID_REFID);
        b=b&&c_database.Create(this,DID_DATABASE);
        b=b&&c_object.Create(this,DID_OBJECT);
        b=b&&c_data.Create(this,DID_DATA);
        if(!b) return False;

        //fill controls
        switch(link.reftype) {
                case Link::heading:
                        c_heading.Check();
                        break;
                case Link::footnote:
                        c_footnote.Check();
                        break;
                case Link::launch:
                        c_launch.Check();
                        break;
                case Link::inform:
                        c_inform.Check();
                        break;
        }

        char buf[40];
        itoa(link.x,buf,10);
        c_x.SetText(buf);
        c_x.LimitText(5);
        itoa(link.y,buf,10);
        c_y.SetText(buf);
        c_y.LimitText(5);
        itoa(link.cx,buf,10);
        c_cx.SetText(buf);
        c_cx.LimitText(5);
        itoa(link.cy,buf,10);
        c_cy.SetText(buf);
        c_cy.LimitText(5);

        c_res.SetText(link.res);
        c_refid.SetText(link.refid);
        c_database.SetText(link.database);
        c_object.SetText(link.object);
        c_data.SetText(link.data);

        return True;
}


Bool FEditLinkDialog::Command(FCommandEvent &ev) {
        switch(ev.GetItem()) {
                case DID_OK:
                        readControls();
                        notifier->ChangeLink(link);
                        EndDialog(DID_OK);
                        return True;

                case DID_CANCEL:
                        EndDialog(DID_CANCEL);
                        return True;

                case DID_APPLY_COORDINATES:
                        readControls();
                        notifier->ChangeLink(link);
                        return True;

                default:
                        return False;
        }
}


