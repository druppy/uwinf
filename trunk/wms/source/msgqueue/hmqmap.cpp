#include "hmqmap.hpp"

static inline unsigned HashHMQ(WmsHMQ hmq) {
        //since we _know_ the hmq's are sequential this is easy
        return (unsigned)hmq;
}
HmqMap::HmqMap() {
        for(unsigned i=0; i<HASH_ENTRIES; i++)
                map[i]=0;
}

HmqMap::~HmqMap() {
        //NO cleanup because all entries should be gone
}

int HmqMap::Insert(WmsHMQ hmq, MsgQueue *mq) {
        entry *e=new entry;
        if(!e) return -1;
        unsigned i=HashHMQ(hmq)%HASH_ENTRIES;
        e->hmq=hmq;
        e->mq=mq;
        e->next=map[i];
        map[i]=e;
        return 0;
}

MsgQueue *HmqMap::Find(WmsHMQ hmq) {
        unsigned i=HashHMQ(hmq)%HASH_ENTRIES;
        for(entry *e=map[i]; e; e=e->next)
                if(e->hmq==hmq) return e->mq;
        return 0;
}

void HmqMap::Remove(WmsHMQ hmq) {
        unsigned i=HashHMQ(hmq)%HASH_ENTRIES;
        for(entry *e=map[i],*prev=0; e; prev=e,e=e->next) {
                if(e->hmq==hmq) {
                        if(prev) prev->next=e->next;
                        else map[i]=e->next;
                        delete e;
                        return;
                }
        }
}

