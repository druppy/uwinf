#ifndef __HWNDMAP_HPP
#define __HWNDMAP_HPP

#include "wms.hpp"

struct DeviceContext;
class HDCMap {
public:
        HDCMap();
        ~HDCMap();

        int Insert(WmsHDC hdc, DeviceContext *dc);
        DeviceContext *Find(WmsHDC hdc);
        void Remove(WmsHDC hdc);
private:
        struct entry {
                WmsHDC hdc;
                DeviceContext *dc;
                entry *next;
        };
        enum { HASH_ENTRIES=13 };
        entry *map[HASH_ENTRIES];
};

#endif

