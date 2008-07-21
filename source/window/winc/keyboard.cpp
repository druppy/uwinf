/***
Filename: keybaord.cpp
Hosts:
  WC 10.6
History:
  ISJ 96-05-31 Created
***/

#define BIFINCL_WINDOW
#define BIFINCL_KEYBOARDHANDLERS
#include <bif.h>

//since the virtual keycode is not available when processing WM_CHAR the virtual
//key code is saved whenever a WM_KEYDOWN is received
static unsigned last_vk=0;

logical_key FKeyboardEvent::GetLogicalKey() const {
        //translate Window's VK_xxx to our lk_xxx
        unsigned vk;
        if(GetID()==WM_CHAR)
                vk=last_vk;
        else
                vk=GetVirtualKey();
        if(vk>='A' && vk<='Z')
                return (logical_key)(vk - 'A' + lk_a);
        if(vk>=VK_NUMPAD0 && vk<=VK_NUMPAD9)
                return (logical_key)(vk - VK_NUMPAD0 + lk_num0);
        if(vk>=VK_F1 && vk<=VK_F12)
                return (logical_key)(vk - VK_F1 + lk_f1);
        
static struct vklk { unsigned vk; logical_key lk; } vklk[14] = {
        { VK_BACK, lk_backspace},
        { VK_TAB, lk_tab},
        { VK_RETURN, lk_enter},
        { VK_ESCAPE, lk_esc},
        { VK_PRIOR, lk_pageup},
        { VK_NEXT, lk_pagedown},
        { VK_END, lk_end},
        { VK_HOME, lk_home},
        { VK_LEFT, lk_left},
        { VK_UP, lk_up},
        { VK_RIGHT, lk_right},
        { VK_DOWN, lk_down},
        { VK_INSERT, lk_insert},
        { VK_DELETE, lk_delete}
};
        for(unsigned i=0; i<14; i++)
                if(vk==vklk[i].vk)
                        return vklk[i].lk;
        return lk_unknown;
}

Bool FKeyboardEvent::IsShiftDown() const {
        return (Bool)(GetKeyState(VK_SHIFT)&1);
}

Bool FKeyboardEvent::IsCtrlDown() const {
        return (Bool)(GetKeyState(VK_CONTROL)&1);
}


Bool FKeyboardHandler::Dispatch(FEvent &ev) {
        if(ev.GetID()==WM_KEYDOWN) {
                last_vk = ev.wParam;
                return KeyPressed((FKeyboardEvent&)ev);
        }
        if(ev.GetID()==WM_CHAR)
                return CharInput((FKeyboardEvent&)ev);
        return False;
}

