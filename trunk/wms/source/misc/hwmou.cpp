#define WMSINCL_HARDWAREMOUSE
#include <wms.hpp>
#include "../sysq/hwmouse.hpp"

void WmsSetPointerShape(const void *p) {
        hwmouse_internal.SetPointerShape((void*)p);
}

void WmsSetPointerPos(unsigned x, unsigned y) {
        hwmouse_internal.SetPosition(x,y);
}

void WmsQueryPointerPos(unsigned *x, unsigned *y) {
        hwmouse_internal.QueryPosition((int*)x,(int*)y);
}

