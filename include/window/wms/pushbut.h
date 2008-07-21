#ifndef __PUSHBUT_H_INCLUDED
#define __PUSHBUT_H_INCLUDED

class BIFCLASS FPushButton : public FLowLevelButton
{
public:
        enum CommandType { wm_command, wm_syscommand, wm_help };
private:
        Bool defaultButton;
        CommandType cmdType;
public:
        FPushButton()
          : FLowLevelButton(),
            defaultButton(False),
            cmdType(wm_command)
          {}

        Bool Create(FWnd *pParent, int resID)
          { return FLowLevelButton::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID, FRect *rect, const char *pszText, Bool isDefault=False, CommandType commandType=wm_command);

        void Push();
protected:
        Bool Paint(WmsHDC hdc, FRect *rect, const char *pszText, Bool isEnabled, Bool isDown, Bool hasFocus);
        void Click();

        virtual Bool bm_querydefault();
        virtual Bool bm_setdefault(Bool d);
        virtual void GetDownRect(FRect *);
        unsigned QueryDlgCode() { return DLGC_PUSHBUTTON|(defaultButton?DLGC_DEFAULT:0); }
        Bool ProcessResourceSetup(int props, char *name[], char *value[]);
};


#endif
