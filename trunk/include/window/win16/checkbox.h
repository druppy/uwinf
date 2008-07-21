#ifndef __CHECKBOX_H_INCLUDED
#define __CHECKBOX_H_INCLUDED

class BIFCLASS FCheckBox : public FControl, public FControlTextHandler {
public:
        FCheckBox() : FControl(), FControlTextHandler(this) {}

        Bool Create(FWnd *pParent, int ID)
          { return FControl::Create(pParent,ID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, int states=2);

        int GetCheck();
        Bool SetCheck(int state);

        Bool Check() { return SetCheck(1); }
        Bool UnCheck() { return SetCheck(0); }
};

#endif

