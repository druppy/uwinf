#ifndef __ANCHOR_HPP
#define __ANCHOR_HPP

#include "wms.hpp"

struct AnchorBlock {
        WmsHAB hab;
        unsigned long tid;
        WmsHMQ hmq;
        AnchorBlock *next;
};

AnchorBlock *WmsQueryAnchorBlockAnchorBlock_internal(WmsHAB hab);
Bool WmsSetAnchorBlockMsgQueue_internal(WmsHAB hab, WmsHMQ hmq);
#endif

