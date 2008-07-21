#ifndef __HWKEYBRD_HPP
#define __HWKEYBRD_HPP

#include "porttype.hpp"

class HWKeyboard {
public:
        HWKeyboard();
        ~HWKeyboard();

        int Initialize();
        void Terminate();

        uint8 QueryCurrentShiftState(void); //kss_...
};

extern HWKeyboard hwkeyboard_internal;

#endif
