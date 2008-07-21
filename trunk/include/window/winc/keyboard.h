#ifndef __KEYBOARD_H_INCLUDED
#define __KEYBOARD_H_INCLUDED

enum logical_key {
        lk_unknown,

        lk_a, lk_b, lk_c, lk_d, lk_e, lk_f, lk_g, lk_h,
        lk_i, lk_j, lk_k, lk_l, lk_m, lk_n, lk_o, lk_p,
        lk_q, lk_r, lk_s, lk_t, lk_u, lk_v, lk_w, lk_x,
        lk_y, lk_z,

        lk_1, lk_2, lk_3, lk_4, lk_5, lk_6, lk_7, lk_8, lk_9, lk_0,

        lk_f1, lk_f2, lk_f3, lk_f4, lk_f5, lk_f6,
        lk_f7, lk_f8, lk_f9, lk_f10, lk_f11, lk_f12,

        lk_esc,

        lk_backspace, lk_tab, lk_enter,

        lk_insert, lk_home, lk_pageup, lk_delete, lk_end, lk_pagedown,

        lk_up, lk_left, lk_down, lk_right,

        lk_num0, lk_num1, lk_num2, lk_num3, lk_num4, 
        lk_num5, lk_num6, lk_num7, lk_num8, lk_num9,

        lk_numslash, lk_numasterisk, lk_numminus, lk_numplus, lk_numenter
};

class FKeyboardEvent : public FEvent {
public:
        WORD GetVirtualKey() const { return (WORD)wParam; }
        unsigned char GetScanCode() const
                { return (unsigned char)(lParam>>16); }
        char GetChar() const
                { return (char)wParam; }
        logical_key GetLogicalKey() const;

        Bool IsShiftDown() const;
        Bool IsCtrlDown() const;
        Bool IsAltDown() const { return (Bool)((lParam&0x20000000UL)!=0); }

        LONG GetFlags() const { return lParam; }
        Bool IsCharAvailable() const { return (Bool)(GetID()==WM_CHAR); }
        Bool IsKeyAvailable() const { return (Bool)(GetID()==WM_KEYDOWN); }
};

class FKeyboardHandler : public FHandler {
public:
        FKeyboardHandler(FWnd *wnd)
          : FHandler(wnd)
          { SetDispatch(dispatchFunc_t(&FKeyboardHandler::Dispatch)); }
        Bool Dispatch(FEvent &);

        virtual Bool CharInput(FKeyboardEvent&) { return False; }
        virtual Bool KeyPressed(FKeyboardEvent&) { return False; }
};

#endif
