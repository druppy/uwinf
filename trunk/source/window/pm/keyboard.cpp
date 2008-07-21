/***
Filename: keyboard.cpp
Description:
  Implementation of a FKeyboardHandler
Host:
  Watcom 10.6
History:
  ISJ 96-05-31 Created
***/
#define BIFINCL_WINDOW
#define BIFINCL_KEYBOARDHANDLERS
#define BIFINCL_RESOURCETEMPLATE
#define BIFINCL_COMMONHANDLERS
#include <bif.h>

logical_key FKeyboardEvent::GetLogicalKey() const {
        //try mapping of VK_XXX -> lk_xxx
static struct { USHORT vk; logical_key lk; } vklk[28] = {
        { VK_BACKSPACE, lk_backspace },
        { VK_TAB,       lk_tab },
        { VK_BACKTAB,   lk_tab },
        { VK_NEWLINE,   lk_enter },
        { VK_ESC,       lk_esc },
        { VK_PAGEUP,    lk_pageup },
        { VK_PAGEDOWN,  lk_pagedown },
        { VK_END,       lk_end },
        { VK_HOME,      lk_home },
        { VK_LEFT,      lk_left },
        { VK_UP,        lk_up },
        { VK_RIGHT,     lk_right },
        { VK_DOWN,      lk_down },
        { VK_INSERT,    lk_insert },
        { VK_DELETE,    lk_delete },
        { VK_ENTER,     lk_numenter },
        { VK_F1,        lk_f1 },
        { VK_F2,        lk_f2 },
        { VK_F3,        lk_f3 },
        { VK_F4,        lk_f4 },
        { VK_F5,        lk_f5 },
        { VK_F6,        lk_f6 },
        { VK_F7,        lk_f7 },
        { VK_F8,        lk_f8 },
        { VK_F9,        lk_f9 },
        { VK_F10,       lk_f10 },
        { VK_F11,       lk_f11 },
        { VK_F12,       lk_f12 }
};
        for(unsigned i=0; i<28; i++)
                if(SHORT1FROMMP(GetMP2()) == vklk[i].vk)
                        return  vklk[i].lk;
        //try special mapping of numeric keys
static struct { UCHAR c; UCHAR scancode; logical_key lk; } scanlk[14] = {
        { '0', 0x52, lk_num0 },
        { '1', 0x4f, lk_num1 },
        { '2', 0x50, lk_num2 },
        { '3', 0x51, lk_num3 },
        { '4', 0x4b, lk_num4 },
        { '5', 0x4c, lk_num5 },
        { '6', 0x4d, lk_num6 },
        { '7', 0x47, lk_num7 },
        { '8', 0x48, lk_num8 },
        { '9', 0x49, lk_num9 },
        { '/', 0x5c, lk_numslash },
        { '*', 0x37, lk_numasterisk },
        { '-', 0x4a, lk_numminus },
        { '+', 0x4e, lk_numplus }
};
        char c=(char)CHAR1FROMMP(mp2);
        for(i=0; i<14; i++)
                if(scanlk[i].c == c && scanlk[i].scancode == CHAR4FROMMP(mp1))
                        return scanlk[i].lk;
        if(c>='a' && c<='z')
                return (logical_key)(lk_a + c-'a');
        if(c>='A' && c<='Z')
                return (logical_key)(lk_a + c-'A');

        return lk_unknown;
}

Bool FKeyboardHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_CHAR) {
                FKeyboardEvent &ke=(FKeyboardEvent&)ev;
                if(ke.IsCharAvailable())
                        return CharInput(ke);
                else if((ke.GetFlags()&KC_KEYUP)==0)
                        return KeyPressed(ke);
        }
        return False;
}

