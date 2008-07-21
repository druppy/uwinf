#ifndef __HABMAP_HPP
#define __HABMAP_HPP

#include "wms.hpp"

class AnchorBlock;
class HABMap {
public:
        HABMap();
        ~HABMap();

        int Insert(WmsHAB hab, unsigned long tid, AnchorBlock *ab);
        AnchorBlock *Find(WmsHAB hab);
        AnchorBlock *Find(unsigned long tid);
        void Remove(WmsHAB hab);
private:
        struct entry {
                WmsHAB hab;
                unsigned long tid;
                AnchorBlock *ab;
                entry *nextHandle;
                entry *nextThread;
        };
        enum { HASH_ENTRIES=67 };
        entry *handleMap[HASH_ENTRIES];
        entry *threadMap[HASH_ENTRIES];
};

#endif

