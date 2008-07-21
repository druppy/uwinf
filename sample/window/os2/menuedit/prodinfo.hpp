#ifndef __PRODINFO_HPP
#define __PRODINFO_HPP

#include "prodinfo.h"

class ProductInformationDialog : public FModalDialog {
public:
        ProductInformationDialog() : FModalDialog() {}

        int DoModal(FWnd *pOwner) {
                return FModalDialog::DoModal(pOwner,DIALOG_PRODINFO);
        }
};

#endif

