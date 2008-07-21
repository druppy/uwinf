#include <wms.hpp>
#include "hwndmap.hpp"

static inline unsigned HashHWND(WmsHWND hwnd) {
        //since we _know_ the hwnd's are sequential this is easy
        return (unsigned)hwnd;
}

HwndMap::HwndMap() {
        for(unsigned i=0; i<HASH_ENTRIES; i++)
                map[i]=0;
}

HwndMap::~HwndMap() {
        //NO cleanup because all entries should be gone
}

int HwndMap::Insert(WmsHWND hwnd, Window *wnd) {
        entry *e=new entry;
        if(!e) return -1;
        unsigned i=HashHWND(hwnd)%HASH_ENTRIES;
        e->hwnd=hwnd;
        e->wnd=wnd;
        e->next=map[i];
        map[i]=e;
        return 0;
}

Window *HwndMap::Find(WmsHWND hwnd) {
        unsigned i=HashHWND(hwnd)%HASH_ENTRIES;
        for(entry *e=map[i]; e; e=e->next)
                if(e->hwnd==hwnd) return e->wnd;
        return 0;
}

void HwndMap::Remove(WmsHWND hwnd) {
        unsigned i=HashHWND(hwnd)%HASH_ENTRIES;
        for(entry *e=map[i],*prev=0; e; prev=e,e=e->next) {
                if(e->hwnd==hwnd) {
                        if(prev) prev->next=e->next;
                        else map[i]=e->next;
                        delete e;
                        return;
                }
        }
}

