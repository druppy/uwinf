#ifndef __EDITLINK_HPP
#define __EDITLINK_HPP

#define BIFINCL_WINDOW
#define BIFINCL_DIALOG
#define BIFINCL_CONTROLS
#define BIFINCL_RADIOBUTTON
#define BIFINCL_SLE
#include <bif.h>

#include "link.hpp"

#include "lnote.hpp"


class FEditLinkDialog : public FModalDialog {
        FRadioButton c_heading,
                     c_footnote,
                     c_launch,
                     c_inform;
        FSLE         c_x,
                     c_y,
                     c_cx,
                     c_cy;
        FSLE         c_res,
                     c_refid,
                     c_database,
                     c_object,
                     c_data;

        Link link;
        LinkChangeNotifier *notifier;
        void readControls();
public:
        FEditLinkDialog(const Link &l, LinkChangeNotifier *n);

        int DoModal(FWnd *pOwner);
protected:
        Bool CreateControls();

        Bool Command(FCommandEvent&);
};

#endif

