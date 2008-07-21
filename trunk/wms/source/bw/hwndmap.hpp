#ifndef __HWNDMAP_HPP
#define __HWNDMAP_HPP

#include "wms.hpp"

class Window;
class HwndMap {
public:
        HwndMap();
        ~HwndMap();

        int Insert(WmsHWND hwnd, Window *wnd);
        Window *Find(WmsHWND hwnd);
        void Remove(WmsHWND hwnd);
private:
        struct entry {
                WmsHWND hwnd;
                Window *wnd;
                entry *next;
        };
        enum { HASH_ENTRIES=67 };
        entry *map[HASH_ENTRIES];
};

#endif

