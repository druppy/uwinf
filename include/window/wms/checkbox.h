#ifndef __CHECKBOX_H_INCLUDED
#define __CHECKBOX_H_INCLUDED

class BIFCLASS FCheckBox : public FLowLevelButton
{
        int states;
        int current_state;
public:
        FCheckBox()
          : FLowLevelButton(),
            states(2),
            current_state(0)
          {}

        Bool Create(FWnd *pParent, int resID)
          { return FControl::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, int states=2);

        int GetCheck();
        Bool SetCheck(int state);

        Bool Check() { return SetCheck(1); }
        Bool UnCheck() { return SetCheck(0); }

protected:
        virtual Bool Paint(WmsHDC hdc, FRect *rect, const char *pszText, Bool isEnabled, Bool isDown, Bool hasFocus);
        void Click();

        unsigned bm_querycheck();
        Bool bm_setcheck(unsigned newstate);
        unsigned QueryDlgCode() { return DLGC_CHECKBOX; }
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};

#endif

