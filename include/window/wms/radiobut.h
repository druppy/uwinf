#ifndef __RADIOBUT_H_INCLUDED
#define __RADIOBUT_H_INCLUDED

class FRadioButtonHandler : public FHandler {
public:
        FRadioButtonHandler(FWnd *pwnd);
        Bool Dispatch(FEvent&);
static WmsMSG wm_isradiobutton;
};

class BIFCLASS FRadioButton : public FLowLevelButton,
                              public FRadioButtonHandler
{
        int current_state;
public:
        FRadioButton()
          : FLowLevelButton(),
            FRadioButtonHandler(this),
            current_state(0)
            {}

        Bool Create(FWnd *pParent, int resID)
          { return FLowLevelButton::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, Bool GroupStart=False);

        Bool IsChecked();
        Bool Check();
        Bool UnCheck();

        int GetCheckIndex();
        Bool CheckIndex(int i);

protected:
        Bool GotFocus(FSetFocusEvent&);

        Bool Paint(WmsHDC hdc, FRect *rect, const char *pszText, Bool isEnabled, Bool isDown, Bool hasFocus);
        void Click();

        unsigned bm_querycheck();
        unsigned bm_querycheckindex();
        Bool bm_setcheck(unsigned newstate);
        
        unsigned QueryDlgCode() { return DLGC_RADIOBUTTON; }
};

#endif
