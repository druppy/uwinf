#include "habmap.hpp"

static inline unsigned HashHAB(WmsHAB hab) {
        //since we _know_ the hab's are sequential this is easy
        return (unsigned)hab;
}

static inline unsigned HashTID(unsigned long tid) {
        return (unsigned)tid;
}

HABMap::HABMap() {
        for(unsigned i=0; i<HASH_ENTRIES; i++) {
                handleMap[i]=0;
                threadMap[i]=0;
        }
}

HABMap::~HABMap() {
        //NO cleanup because all entries should be gone
}

int HABMap::Insert(WmsHAB hab, unsigned long tid, AnchorBlock *ab) {
        entry *e=new entry;
        if(!e) return -1;
        e->hab=hab;
        e->ab=ab;
        e->tid=tid;

        unsigned i=HashHAB(hab)%HASH_ENTRIES;
        e->nextHandle=handleMap[i];
        handleMap[i]=e;
        
        i=HashTID(tid)%HASH_ENTRIES;
        e->nextThread=threadMap[i];
        threadMap[i]=e;

        return 0;
}

AnchorBlock *HABMap::Find(WmsHAB hab) {
        unsigned i=HashHAB(hab)%HASH_ENTRIES;
        for(entry *e=handleMap[i]; e; e=e->nextHandle)
                if(e->hab==hab) return e->ab;
        return 0;
}

AnchorBlock *HABMap::Find(unsigned long tid) {
        unsigned i=HashTID(tid)%HASH_ENTRIES;
        for(entry *e=threadMap[i]; e; e=e->nextThread)
                if(e->tid==tid) return e->ab;
        return 0;
}

void HABMap::Remove(WmsHAB hab) {
        unsigned i=HashHAB(hab)%HASH_ENTRIES;
        for(entry *e=handleMap[i],*prev=0; e; prev=e,e=e->nextHandle) {
                if(e->hab==hab) {
                        if(prev) prev->nextHandle=e->nextHandle;
                        else handleMap[i]=e->nextHandle;
                        goto found;
                }
        }
        return;
found:
        i=HashTID(e->tid)%HASH_ENTRIES;
        if(threadMap[i]==e)
                threadMap[i]=e->nextThread;
        else {
                for(entry *prev=threadMap[i]; prev->nextThread!=e; prev=prev->nextThread);
                prev->nextThread=e->nextThread;
        }

        delete e;
}

