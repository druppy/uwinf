#ifndef __LOWLBUT_H_INCLUDED
#define __LOWLBUT_H_INCLUDED

#ifndef __CLHNDL_H_INCLUDED
#  include <window\wms\clhndl.h>
#endif
#ifndef __KEYBOARD_H_INCLUDED
#  include <window\wms\keyboard.h>
#endif
#ifndef __MOUSE_H_INCLUDED
#  include <window\wms\mouse.h>
#endif
#ifndef __COMHNDL_H_INCLUDED
#  include <window\wms\comhndl.h>
#endif

//WM_CONTROL notification codes
#define BN_GOTFOCUS     0x0001
#define BN_LOSTFOCUS    0x0002
#define BN_CLICK        0x0003

class FLowLevelButtonHandler : public FHandler {
public:
        FLowLevelButtonHandler(FWnd *pwnd)
          : FHandler(pwnd)
          { SetDispatch(dispatchFunc_t(FLowLevelButtonHandler::Dispatch)); }
protected:
        Bool Dispatch(FEvent &ev);

        virtual unsigned bm_querycheck() { return 0; }
        virtual unsigned bm_querycheckindex() { return 0; }
        virtual Bool bm_setcheck(unsigned /*newstate*/) { return False; }
        virtual void bm_click() =0;
        virtual Bool bm_querydefault() { return False; }
        virtual Bool bm_setdefault(Bool /*d*/) { return False; }
};

class FLowLevelButton : public FControl,
                        public FControlTextHandler,
                        public FPaintHandler,
                        public FKeyboardHandler,
                        public FMouseHandler,
                        public FFocusHandler,
                        public FEnableHandler,
                        public FLowLevelButtonHandler
{
        Bool isDown;
protected:
        FLowLevelButton()
          : FControl(),
            FControlTextHandler(this),
            FPaintHandler(this),
            FKeyboardHandler(this),
            FMouseHandler(this),
            FFocusHandler(this),
            FEnableHandler(this),
            FLowLevelButtonHandler(this),
            isDown(False)
          {}

        Bool Create(FWnd *pParent, int resID)
          { return FControl::Create(pParent,resID); }
        Bool Create(FWnd *pParent, int ID,
                    const char *pszControlText,
                    uint32 flStyle, FRect *rect)
        { return FControl::Create(pParent,ID,pszControlText,flStyle,rect); }

        Bool ProcessResourceSetup(int props, char *name[], char *value[]);

protected:
        Bool Paint();
        Bool CharInput(FKeyboardEvent& ev);
        Bool Button1Down(FButtonDownEvent&);
        Bool Button1Up(FButtonUpEvent&);
        Bool MouseMove(FMouseMoveEvent&);
        Bool LostFocus(FKillFocusEvent&);
        Bool GotFocus(FSetFocusEvent&);
        Bool Enabled(FEnableEvent&) { Invalidate(); return False; }
        Bool Disabled(FEnableEvent&) { Invalidate(); return False; }
        void bm_click();

        virtual Bool Paint(WmsHDC hdc, FRect *rect, const char *pszText, Bool isEnabled, Bool isDown, Bool hasFocus) =0;

        Bool MatchMnemonic(char c);

        virtual void Click() =0;
        virtual void GetDownRect(FRect *) { }
};
#endif
