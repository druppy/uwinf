#ifndef __KEYBOARD_H_INCLUDED
#define __KEYBOARD_H_INCLUDED

#ifndef WMSINCL_KBDSTATE
#  define WMSINCL_KBDSTATE
#endif
#ifndef WMSINCL_LOGKEYS
#  define WMSINCL_LOGKEYS
#endif
#include <wms.hpp>

class FKeyboardEvent : public FEvent {
public:
        uint8 GetScanCode() const
                { return UINT81FROMMP(GetMP1()); }
        char GetChar() const
                { return (char)UINT82FROMMP(GetMP1()); }
        logical_key GetLogicalKey() const
                { return (logical_key)UINT83FROMMP(GetMP1()); }

        Bool IsShiftDown() const { return (Bool)((GetShiftState()&kss_shift)!=0); }
        Bool IsCtrlDown() const { return (Bool)((GetShiftState()&kss_ctrl)!=0); }
        Bool IsAltDown() const { return (Bool)((GetShiftState()&kss_alt)!=0); }

        uint8 GetShiftState() const { return UINT84FROMMP(GetMP1()); }

        uint16 GetFlags() const { return UINT161FROMMP(GetMP2()); }
        Bool IsCharAvailable() const { return (Bool)((GetFlags()&0x40)!=0); }
        Bool IsKeyAvailable() const { return (Bool)((GetFlags()&0x80)!=0); }
};

class FKeyboardHandler : public FHandler {
public:
        FKeyboardHandler(FWnd *wnd)
          : FHandler(wnd)
          { SetDispatch(dispatchFunc_t(FKeyboardHandler::Dispatch)); }
        Bool Dispatch(FEvent &);

        virtual Bool CharInput(FKeyboardEvent&) { return False; }
        virtual Bool KeyPressed(FKeyboardEvent&) { return False; }
};

//WMS only:
Bool TranslateAccelerator(const FAcceleratorTable &accel, const FKeyboardEvent &ev, FEvent &outev);

#endif
