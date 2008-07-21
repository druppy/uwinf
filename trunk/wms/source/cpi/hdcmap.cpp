#include <wms.hpp>
#include "hdcmap.hpp"

static inline unsigned HashHDC(WmsHDC hdc) {
        //since we _know_ the hdc's are sequential this is easy
        return (unsigned)hdc;
}

HDCMap::HDCMap() {
        for(unsigned i=0; i<HASH_ENTRIES; i++)
                map[i]=0;
}

HDCMap::~HDCMap() {
        //NO cleanup because all entries should be gone
}

int HDCMap::Insert(WmsHDC hdc, DeviceContext *dc) {
        entry *e=new entry;
        if(!e) return -1;
        unsigned i=HashHDC(hdc)%HASH_ENTRIES;
        e->hdc=hdc;
        e->dc=dc;
        e->next=map[i];
        map[i]=e;
        return 0;
}

DeviceContext *HDCMap::Find(WmsHDC hdc) {
        unsigned i=HashHDC(hdc)%HASH_ENTRIES;
        for(entry *e=map[i]; e; e=e->next)
                if(e->hdc==hdc) return e->dc;
        return 0;
}

void HDCMap::Remove(WmsHDC hdc) {
        unsigned i=HashHDC(hdc)%HASH_ENTRIES;
        for(entry *e=map[i],*prev=0; e; prev=e,e=e->next) {
                if(e->hdc==hdc) {
                        if(prev) prev->next=e->next;
                        else map[i]=e->next;
                        delete e;
                        return;
                }
        }
}

