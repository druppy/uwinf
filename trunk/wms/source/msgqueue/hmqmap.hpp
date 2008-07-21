#ifndef __HMQMAP_HPP
#define __HMQMAP_HPP

#include "wms.hpp"

class MsgQueue;
class HmqMap {
public:
        HmqMap();
        ~HmqMap();

        int Insert(WmsHMQ hmq, MsgQueue *mq);
        MsgQueue *Find(WmsHMQ hmq);
        void Remove(WmsHMQ hmq);
private:
        struct entry {
                WmsHMQ hmq;
                MsgQueue *mq;
                entry *next;
        };
        enum { HASH_ENTRIES=67 };
        entry *map[HASH_ENTRIES];
};

#endif

